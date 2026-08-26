# integrations/_template/CMakeLists.txt
#
# 新集成模板：把本目录复制为 integrations/<name>/，然后：
#   1. 把下面所有 <NAME>/<name>/<ExternalLib> 占位符替换为真实值
#   2. 顶层 CMakeLists.txt 加一行：
#        set(SCL2EXT_<NAME> "AUTO" CACHE STRING "Build <name> integration (AUTO/ON/OFF)")
#      并把这个 <name> 加进顶层的 foreach(...) 列表
#   3. 写 include/SCL2Ext/<name>.hpp 与 src/<name>.cpp
#
# 本目录名以下划线开头，顶层 foreach 不会把它当作集成构建。

find_package(SharedCppLib2 REQUIRED)

# —— 外部库检测：改写成这个集成实际需要的 find_package ——
find_package(<ExternalLib> QUIET)

if(NOT <ExternalLib>_FOUND AND NOT SCL2EXT_<NAME> STREQUAL "ON")
    message(STATUS "SCL2Ext: <ExternalLib> not found; skipping <name> integration")
    return()
endif()
if(NOT <ExternalLib>_FOUND)
    message(FATAL_ERROR "SCL2Ext: <ExternalLib> is required (SCL2EXT_<NAME>=ON) but not found")
endif()

add_library(scl2ext_<name> STATIC src/<name>.cpp)
target_include_directories(scl2ext_<name> PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(scl2ext_<name> PUBLIC SharedCppLib2::basic <ExternalLib>::<ExternalLib>)
add_library(SCL2Ext::<name> ALIAS scl2ext_<name>)
