# integrations/bit7z/CMakeLists.txt
#
# bit7z 集成：把 SharedCppLib2 的 API 与 bit7z（7-Zip 封装）对接。
#
# bit7z 有两种获取方式，这里都兼容：
#   1) 官方配置       : find_package(bit7z)             -> target bit7z::bit7z
#   2) vcpkg 兼容模式 : find_package(unofficial-bit7z)  -> target unofficial::bit7z::bit7z64 / bit7z32
#
# 开关：SCL2EXT_BIT7Z = AUTO(默认,找不到就跳过) / ON(强制) / OFF(跳过)

find_package(SharedCppLib2 REQUIRED)

set(_bit7z_target "")
find_package(bit7z QUIET)
if(TARGET bit7z::bit7z)
    set(_bit7z_target bit7z::bit7z)
else()
    find_package(unofficial-bit7z CONFIG QUIET)
    if(TARGET unofficial::bit7z::bit7z64)
        set(_bit7z_target unofficial::bit7z::bit7z64)
    elseif(TARGET unofficial::bit7z::bit7z32)
        set(_bit7z_target unofficial::bit7z::bit7z32)
    endif()
endif()

if(NOT _bit7z_target)
    if(SCL2EXT_BIT7Z STREQUAL "ON")
        message(FATAL_ERROR
            "SCL2Ext: bit7z is required (SCL2EXT_BIT7Z=ON) but neither "
            "find_package(bit7z) nor find_package(unofficial-bit7z) succeeded")
    endif()
    message(STATUS "SCL2Ext: bit7z not found (official or unofficial-bit7z); skipping bit7z integration")
    return()
endif()

add_library(scl2ext_bit7z STATIC src/bit7z.cpp)
target_include_directories(scl2ext_bit7z PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(scl2ext_bit7z PUBLIC SharedCppLib2::basic ${_bit7z_target})
add_library(SCL2Ext::bit7z ALIAS scl2ext_bit7z)

# bit7z 运行时通过 LoadLibrary 动态加载 7-Zip 的 DLL（不走导入表），所以
# VCPKG_APPLOCAL_DEPS 检测不到、不会自动复制。vcpkg 的 bit7z 会带 7zip::7zip
# imported target；这里提示最终 exe 需要在 POST_BUILD 里把 7z.dll 复制到可执行旁。
if(TARGET 7zip::7zip)
    message(STATUS
        "SCL2Ext: bit7z integration built. Remember to copy the 7-Zip DLL "
        "(7zip::7zip) next to your executable for runtime loading.")
endif()
