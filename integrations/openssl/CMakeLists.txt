# integrations/openssl/CMakeLists.txt
#
# OpenSSL 集成：一个基于 OpenSSL 的 TLS 客户端传输（tls_client），实现
# SharedCppLib2 的 scl2::transport_interface，使 httpclient 等协议层可以跑 HTTPS。
#
# 开关：SCL2EXT_OPENSSL = AUTO(默认,找不到就跳过) / ON(强制) / OFF(跳过)

find_package(SharedCppLib2 REQUIRED)
find_package(OpenSSL QUIET)

if(NOT OpenSSL_FOUND AND NOT SCL2EXT_OPENSSL STREQUAL "ON")
    message(STATUS "SCL2Ext: OpenSSL not found; skipping openssl integration")
    return()
endif()
if(NOT OpenSSL_FOUND)
    message(FATAL_ERROR "SCL2Ext: OpenSSL is required (SCL2EXT_OPENSSL=ON) but not found")
endif()

add_library(scl2ext_openssl STATIC src/tls.cpp)
target_include_directories(scl2ext_openssl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
if(WIN32)
    # Windows 需要 ws2_32 提供 socket 相关符号
    target_link_libraries(scl2ext_openssl PUBLIC ws2_32)
endif()
target_link_libraries(scl2ext_openssl PUBLIC SharedCppLib2::basic OpenSSL::SSL OpenSSL::Crypto)
add_library(SCL2Ext::openssl ALIAS scl2ext_openssl)
