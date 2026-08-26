/*
    OpenSSL integration for SharedCppLib2 (SCL2Ext)

    tls_client — a TLS client transport over OpenSSL that implements
    the shared scl2::transport_interface (see SharedCppLib2/stream.hpp). This lets SharedCppLib2's protocol layers
    (e.g. httpclient) speak HTTPS with no changes: just pass a tls_client
    where a scl2::transport_interface& is expected.

    Server certificate verification is enabled by default (uses the system /
    OpenSSL default trust store); call set_verify_peer(false) to skip it
    (for local testing only — insecure against MITM).
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include <SharedCppLib2/stream.hpp>

namespace scl2ext {

class tls_client : public scl2::transport_interface {
public:
    tls_client();
    ~tls_client() override;

    tls_client(const tls_client&) = delete;
    tls_client& operator=(const tls_client&) = delete;

    // ---- transport_interface ----
    bool connect(const std::string& host, uint16_t port) override;
    void disconnect() override;
    bool is_connected() const override;

    // ---- basic_istream ----
    bool valid() override;
    bool readyRead() override;
    bool waitForReadyRead(std::chrono::milliseconds timeout
                          = std::chrono::seconds(5)) override;
    size_t available() override;
    scl2::bytearray read(size_t bytes) override;
    scl2::bytearray readAll() override;
    bool reset() override;

    // ---- basic_ostream ----
    size_t write(const scl2::bytearray& data) override;

    /// @brief Enable/disable server certificate verification (default: on).
    void set_verify_peer(bool enable);

private:
    struct Impl;   // pimpl: hides OpenSSL types from this header
    Impl* m_impl;
};

} // namespace scl2ext
