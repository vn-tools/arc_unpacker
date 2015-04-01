#include "formats/touhou/anm_archive.h"
#include "test_support/archive_support.h"
#include "test_support/converter_support.h"
#include "test_support/eassert.h"
using namespace Formats::Touhou;

void test_anm_archive(
    const std::string path_to_anm,
    const std::vector<std::string> paths_to_png)
{
    std::unique_ptr<Archive> archive(new AnmArchive);
    auto actual_files = unpack_to_memory(path_to_anm, *archive);

    eassert(actual_files.size() == paths_to_png.size());
    for (size_t i = 0; i < paths_to_png.size(); i ++)
    {
        std::unique_ptr<File> expected_file(
            new File(paths_to_png[i], FileIOMode::Read));
        assert_decoded_image(*actual_files[i], *expected_file);
    }
}

int main(void)
{
    // format 1
    test_anm_archive(
        "tests/formats/touhou/files/eff01.anm",
        { "tests/formats/touhou/files/eff01-out.png" });

    // format 3
    test_anm_archive(
        "tests/formats/touhou/files/face_01_00.anm",
        { "tests/formats/touhou/files/face_01_00-out.png" });

    // format 5
    test_anm_archive(
        "tests/formats/touhou/files/player00.anm",
        { "tests/formats/touhou/files/player00-out.png" });

    // format 7
    test_anm_archive(
        "tests/formats/touhou/files/clouds.anm",
        { "tests/formats/touhou/files/clouds-out.png" });

    // multiple files
    test_anm_archive(
        "tests/formats/touhou/files/eff05.anm",
        {
            "tests/formats/touhou/files/eff05-out.png",
            "tests/formats/touhou/files/eff05-out2.png"
        });

    return 0;
}
