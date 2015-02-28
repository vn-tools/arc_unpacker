#include "formats/gfx/anm_archive.h"
#include "test_support/archive_support.h"
#include "test_support/converter_support.h"
#include "test_support/eassert.h"

void test_anm_archive(
    const std::string path_to_anm,
    const std::vector<std::string> paths_to_png)
{
    std::unique_ptr<Archive> archive(new AnmArchive);
    auto output_files = unpack_to_memory(path_to_anm, *archive);
    auto actual_files = output_files->get_saved();

    eassert(actual_files.size() == paths_to_png.size());
    for (size_t i = 0; i < paths_to_png.size(); i ++)
    {
        std::unique_ptr<File> expected_file(new File(paths_to_png[i], "rb"));
        assert_decoded_image(*actual_files[i], *expected_file);
    }
}

int main(void)
{
    // format 1
    test_anm_archive(
        "tests/test_files/gfx/anm/eff01.anm",
        { "tests/test_files/gfx/anm/eff01-out.png" });

    // format 3
    test_anm_archive(
        "tests/test_files/gfx/anm/face_01_00.anm",
        { "tests/test_files/gfx/anm/face_01_00-out.png" });

    // format 5
    test_anm_archive(
        "tests/test_files/gfx/anm/player00.anm",
        { "tests/test_files/gfx/anm/player00-out.png" });

    // format 7
    test_anm_archive(
        "tests/test_files/gfx/anm/clouds.anm",
        { "tests/test_files/gfx/anm/clouds-out.png" });

    // multiple files
    test_anm_archive(
        "tests/test_files/gfx/anm/eff05.anm",
        {
            "tests/test_files/gfx/anm/eff05-out.png",
            "tests/test_files/gfx/anm/eff05-out2.png"
        });

    return 0;
}
