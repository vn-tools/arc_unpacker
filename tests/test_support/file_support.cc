#include "test_support/file_support.h"
#include "util/pack/zlib.h"
#include "util/range.h"
#include "test_support/catch.hh"

using namespace au;

std::shared_ptr<File> tests::stub_file(
    const std::string &name, const bstr &data)
{
    std::shared_ptr<File> f(new File);
    f->name = name;
    f->io.write(data);
    return f;
}

std::shared_ptr<File> tests::file_from_path(
    const boost::filesystem::path &path)
{
    return std::shared_ptr<File>(new File(path, io::FileMode::Read));
}

std::shared_ptr<File> tests::zlib_file_from_path(
    const boost::filesystem::path &path)
{
    File compressed_file(path, io::FileMode::Read);
    auto compressed_data = compressed_file.io.read_to_eof();
    auto decompressed_data = util::pack::zlib_inflate(compressed_data);
    std::shared_ptr<File> decompressed_file(new File);
    decompressed_file->name = compressed_file.name;
    decompressed_file->io.write(decompressed_data);
    return decompressed_file;
}

void tests::compare_files(
    const std::vector<std::shared_ptr<File>> &expected_files,
    const std::vector<std::shared_ptr<File>> &actual_files,
    bool compare_file_names)
{
    REQUIRE(actual_files.size() == expected_files.size());
    for (auto i : util::range(expected_files.size()))
    {
        auto &expected_file = expected_files[i];
        auto &actual_file = actual_files[i];
        tests::compare_files(*expected_file, *actual_file, compare_file_names);
    }
}

void tests::compare_files(
    const File &expected_file, const File &actual_file, bool compare_file_names)
{
    if (compare_file_names)
        REQUIRE(expected_file.name == actual_file.name);
    REQUIRE(expected_file.io.size() == actual_file.io.size());
    expected_file.io.seek(0);
    actual_file.io.seek(0);
    auto expected_content = expected_file.io.read_to_eof();
    auto actual_content = actual_file.io.read_to_eof();
    INFO("Expected content: " << (expected_content.size() < 1000
        ? expected_content.str()
        : "(too big to display)"));
    INFO("Actual content: " << (actual_content.size() < 1000
        ? actual_content.str()
        : "(too big to display)"));
    REQUIRE(expected_content == actual_content);
}
