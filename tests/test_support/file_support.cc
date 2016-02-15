#include "test_support/file_support.h"
#include "algo/format.h"
#include "algo/pack/zlib.h"
#include "algo/range.h"
#include "test_support/catch.h"

using namespace au;

std::unique_ptr<io::File> tests::stub_file(
    const std::string &path, const bstr &data)
{
    return std::make_unique<io::File>(path, data);
}

std::unique_ptr<io::File> tests::file_from_path(
    const io::path &path, const std::string &custom_path)
{
    auto ret = std::make_unique<io::File>(path, io::FileMode::Read);
    if (!custom_path.empty())
        ret->path = custom_path;
    return ret;
}

std::unique_ptr<io::File> tests::zlib_file_from_path(
    const io::path &path, const std::string &custom_path)
{
    io::File compressed_file(path, io::FileMode::Read);
    const auto compressed_data = compressed_file.stream.read_to_eof();
    const auto decompressed_data = algo::pack::zlib_inflate(compressed_data);
    return std::make_unique<io::File>(
        custom_path.empty() ? compressed_file.path : custom_path,
        decompressed_data);
}

void tests::compare_paths(
    const io::path &actual_file_path,
    const io::path &expected_file_path)
{
    INFO(algo::format(
        "Expected file path: %s, actual: %s\n",
        expected_file_path.c_str(),
        actual_file_path.c_str()));
    REQUIRE(actual_file_path == expected_file_path);
}

void tests::compare_files(
    const std::vector<std::shared_ptr<io::File>> &actual_files,
    const std::vector<std::shared_ptr<io::File>> &expected_files,
    const bool compare_file_paths)
{
    REQUIRE(actual_files.size() == expected_files.size());
    for (const auto i : algo::range(expected_files.size()))
    {
        const auto &expected_file = expected_files[i];
        const auto &actual_file = actual_files[i];
        INFO(algo::format("Files at index %d differ", i));
        tests::compare_files(*actual_file, *expected_file, compare_file_paths);
    }
}

void tests::compare_files(
    const io::File &actual_file,
    const io::File &expected_file,
    const bool compare_file_paths)
{
    if (compare_file_paths)
        tests::compare_paths(actual_file.path, expected_file.path);
    expected_file.stream.seek(0);
    actual_file.stream.seek(0);
    const auto expected_content = expected_file.stream.read_to_eof();
    const auto actual_content = actual_file.stream.read_to_eof();
    INFO("Expected content: " << (expected_content.size() < 1000
        ? expected_content.str()
        : "(too big to display)"));
    INFO("Actual content: " << (actual_content.size() < 1000
        ? actual_content.str()
        : "(too big to display)"));
    REQUIRE(actual_file.stream.size() == expected_file.stream.size());
    REQUIRE(actual_content == expected_content);
}

void tests::dump_file(io::File &input_file, const io::path &path)
{
    const auto old_pos = input_file.stream.pos();
    input_file.stream.seek(0);
    const auto data = input_file.stream.read_to_eof();
    io::File(path, io::FileMode::Write).stream.write(data);
    input_file.stream.seek(old_pos);
}
