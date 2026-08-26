#include "SCL2Ext/bit7z.hpp"

#include <stdexcept>

#include <bit7z/bitarchivereader.hpp>

namespace scl2ext {
namespace {

// fs::path -> UTF-8 std::string (bit7z treats path strings as UTF-8)
std::string pathToUtf8(const std::filesystem::path& p)
{
    const auto u8 = p.u8string();
    return std::string(reinterpret_cast<const char*>(u8.data()), u8.size());
}

// bit7z::buffer_t -> scl2::bytearray
scl2::bytearray toBytearray(const bit7z::buffer_t& src)
{
    scl2::bytearray dst(src.size(), std::byte{0});
    for (size_t i = 0; i < src.size(); ++i)
        dst[i] = std::byte{src[i]};
    return dst;
}

} // namespace

void extract7z(const std::string& archive_path, const std::filesystem::path& out_dir)
{
    bit7z::Bit7zLibrary lib("7zip.dll");
    bit7z::BitArchiveReader reader(lib, archive_path, bit7z::BitFormat::SevenZip);
    std::filesystem::create_directories(out_dir);
    reader.extractTo(pathToUtf8(out_dir));
}

scl2::bytearray extract7zItem(const std::string& archive_path, const std::string& item_path)
{
    bit7z::Bit7zLibrary lib("7zip.dll");
    bit7z::BitArchiveReader reader(lib, archive_path, bit7z::BitFormat::SevenZip);
    const auto it = reader.find(item_path);
    if (it == reader.end())
        throw std::runtime_error("SCL2Ext::extract7zItem: item not found: " + item_path);
    bit7z::buffer_t buf;
    reader.extractTo(buf, (*it).index());
    return toBytearray(buf);
}

} // namespace scl2ext
