#include "SCL2Ext/tls.hpp"

#include <openssl/ssl.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <chrono>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace scl2ext {

namespace {

#ifdef _WIN32
using socket_t = SOCKET;
constexpr socket_t kInvalidSocket = INVALID_SOCKET;
#else
using socket_t = int;
constexpr socket_t kInvalidSocket = -1;
#endif

void closeSocket(socket_t s)
{
#ifdef _WIN32
    closesocket(s);
#else
    ::close(s);
#endif
}

bool setNonBlocking(socket_t s, bool nb)
{
#ifdef _WIN32
    u_long mode = nb ? 1 : 0;
    return ioctlsocket(s, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) return false;
    flags = nb ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK);
    return fcntl(s, F_SETFL, flags) == 0;
#endif
}

// Non-blocking connect with a 10s timeout.
socket_t connectTcp(const std::string& host, uint16_t port)
{
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return kInvalidSocket;
#endif

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = nullptr;
    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res) != 0)
        return kInvalidSocket;

    socket_t sock = kInvalidSocket;
    for (addrinfo* ai = res; ai; ai = ai->ai_next) {
        sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
        if (sock == kInvalidSocket) continue;
        setNonBlocking(sock, true);
        if (::connect(sock, ai->ai_addr, static_cast<int>(ai->ai_addrlen)) == 0)
            break;   // connected immediately
        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(sock, &wfds);
        timeval tv{ 10, 0 };
        int sr = select(static_cast<int>(sock) + 1, nullptr, &wfds, nullptr, &tv);
        if (sr > 0 && FD_ISSET(sock, &wfds)) {
            int err = 0;
#ifdef _WIN32
            int len = sizeof(err);
#else
            socklen_t len = sizeof(err);
#endif
            getsockopt(sock, SOL_SOCKET, SO_ERROR,
                       reinterpret_cast<char*>(&err), &len);
            if (err == 0) break;
        }
        closeSocket(sock);
        sock = kInvalidSocket;
    }
    freeaddrinfo(res);
    if (sock == kInvalidSocket) return kInvalidSocket;
    setNonBlocking(sock, false);
    return sock;
}

} // namespace

struct tls_client::Impl {
    SSL_CTX* ctx = nullptr;
    SSL* ssl = nullptr;
    int sock = -1;
    bool verify_peer = true;
    bool connected = false;
};

tls_client::tls_client() : m_impl(new Impl)
{
    OPENSSL_init_ssl(0, nullptr);
    m_impl->ctx = SSL_CTX_new(TLS_client_method());
    if (!m_impl->ctx)
        throw std::runtime_error("SCL2Ext::tls_client: SSL_CTX_new failed");
    SSL_CTX_set_min_proto_version(m_impl->ctx, TLS1_2_VERSION);
    SSL_CTX_set_default_verify_paths(m_impl->ctx);
}

tls_client::~tls_client()
{
    disconnect();
    if (m_impl->ctx) SSL_CTX_free(m_impl->ctx);
    delete m_impl;
}

void tls_client::set_verify_peer(bool enable) { m_impl->verify_peer = enable; }

bool tls_client::connect(const std::string& host, uint16_t port)
{
    if (m_impl->connected) return true;
    disconnect();

    const socket_t sock = connectTcp(host, port);
    if (sock == kInvalidSocket) return false;

    m_impl->sock = static_cast<int>(sock);
    m_impl->ssl = SSL_new(m_impl->ctx);
    if (!m_impl->ssl) {
        closeSocket(sock);
        m_impl->sock = -1;
        return false;
    }
    SSL_set_fd(m_impl->ssl, static_cast<int>(sock));
    SSL_set_tlsext_host_name(m_impl->ssl, host.c_str());
    SSL_set_verify(m_impl->ssl,
                   m_impl->verify_peer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE,
                   nullptr);

    if (SSL_connect(m_impl->ssl) != 1) {
        disconnect();
        return false;
    }
    m_impl->connected = true;
    return true;
}

void tls_client::disconnect()
{
    if (m_impl->ssl) {
        SSL_shutdown(m_impl->ssl);
        SSL_free(m_impl->ssl);
        m_impl->ssl = nullptr;
    }
    if (m_impl->sock >= 0) {
        closeSocket(static_cast<socket_t>(m_impl->sock));
        m_impl->sock = -1;
    }
    m_impl->connected = false;
}

bool tls_client::is_connected() const { return m_impl->connected && m_impl->ssl; }

bool tls_client::valid() { return is_connected(); }

bool tls_client::readyRead()
{
    if (!valid()) return false;
    if (SSL_pending(m_impl->ssl) > 0) return true;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(static_cast<socket_t>(m_impl->sock), &rfds);
    timeval tv{ 0, 0 };
    const int rc = select(static_cast<int>(m_impl->sock) + 1, &rfds, nullptr, nullptr, &tv);
    return rc > 0 && FD_ISSET(static_cast<socket_t>(m_impl->sock), &rfds);
}

bool tls_client::waitForReadyRead(std::chrono::milliseconds timeout)
{
    if (!valid()) return false;
    if (SSL_pending(m_impl->ssl) > 0) return true;
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(static_cast<socket_t>(m_impl->sock), &rfds);
    const auto ms = timeout.count();
    timeval tv{ static_cast<long>(ms / 1000), static_cast<long>((ms % 1000) * 1000) };
    const int rc = select(static_cast<int>(m_impl->sock) + 1, &rfds, nullptr, nullptr, &tv);
    return rc > 0 && FD_ISSET(static_cast<socket_t>(m_impl->sock), &rfds);
}

bool tls_client::reset()
{
    disconnect();   // idempotent
    return true;
}

size_t tls_client::available()
{
    if (!valid()) return 0;
    const int pending = SSL_pending(m_impl->ssl);
    return pending > 0 ? static_cast<size_t>(pending) : 0;
}

scl2::bytearray tls_client::read(size_t bytes)
{
    if (!valid() || bytes == 0) return {};
    scl2::bytearray out(bytes, std::byte{0});
    const int rc = SSL_read(m_impl->ssl, out.data(), static_cast<int>(bytes));
    if (rc <= 0) return {};
    out.resize(static_cast<size_t>(rc));
    return out;
}

scl2::bytearray tls_client::readAll()
{
    scl2::bytearray out;
    char buf[4096];
    while (valid()) {
        const int rc = SSL_read(m_impl->ssl, buf, sizeof(buf));
        if (rc <= 0) break;
        out.append(reinterpret_cast<const std::byte*>(buf), static_cast<size_t>(rc));
    }
    return out;
}

size_t tls_client::write(const scl2::bytearray& data)
{
    if (!valid() || data.empty()) return 0;
    const int rc = SSL_write(m_impl->ssl, data.data(), static_cast<int>(data.size()));
    return rc > 0 ? static_cast<size_t>(rc) : 0;
}

} // namespace scl2ext
