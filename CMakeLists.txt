cmake_minimum_required(VERSION 3.16)
project(SCL2Ext VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 源码若含中文注释，MSVC 默认按系统代码页(936)读取会报 C4819
if(MSVC)
    add_compile_options(/utf-8)
endif()

# ── 集成开关 ──────────────────────────────────────────────────────────────
# 每个集成三态：AUTO(默认,找到外部库才构建) / ON(强制,找不到报错) / OFF(跳过)
set(SCL2EXT_BIT7Z   "AUTO" CACHE STRING "Build bit7z integration (AUTO/ON/OFF)")
set(SCL2EXT_OPENSSL "AUTO" CACHE STRING "Build OpenSSL integration (AUTO/ON/OFF)")

foreach(ext bit7z openssl)
    if(NOT SCL2EXT_${ext} STREQUAL "OFF" AND
       EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/integrations/${ext}/CMakeLists.txt")
        add_subdirectory(integrations/${ext})
    endif()
endforeach()
