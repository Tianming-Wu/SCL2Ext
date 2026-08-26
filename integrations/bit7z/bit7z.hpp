/*
    bit7z integration for SharedCppLib2 (SCL2Ext)

    Thin wrapper around bit7z (7-Zip bindings) exposing scl2-friendly APIs:
    extract a .7z archive to a directory, or extract a single item to memory.

    Requires the 7-Zip DLL (7z.dll / 7zip.dll) to be loadable at runtime.
*/

#pragma once

#include <filesystem>
#include <string>

#include <SharedCppLib2/bytearray.hpp>

namespace scl2ext {

/// @brief Extract a whole .7z archive to a directory (created if needed).
/// @param archive_path  path to the .7z archive (UTF-8)
/// @throw std::runtime_error on failure
void extract7z(const std::string& archive_path, const std::filesystem::path& out_dir);

/// @brief Extract a single item from a .7z archive into memory.
/// @param item_path  the item's path inside the archive (UTF-8)
/// @throw std::runtime_error if not found / on failure
scl2::bytearray extract7zItem(const std::string& archive_path, const std::string& item_path);

} // namespace scl2ext
