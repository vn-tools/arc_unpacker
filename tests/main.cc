#define CATCH_CONFIG_RUNNER
#include "entry_point.h"
#include "io/program_path.h"
#include "test_support/catch.h"
#include "virtual_file_system.h"

using namespace au;

struct Listener final : Catch::TestEventListenerBase
{
    using TestEventListenerBase::TestEventListenerBase;

    void testCaseStarting(Catch::TestCaseInfo const &test_info) override
    {
        VirtualFileSystem::clear();
    }
};

INTERNAL_CATCH_REGISTER_LISTENER(Listener)

int main(int argc, char *argv[])
{
    io::set_program_path_from_arg(argv[0]);
    init_fs_utf8();
    return Catch::Session().run(argc, argv);
}
