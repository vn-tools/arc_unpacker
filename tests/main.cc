#define CATCH_CONFIG_RUNNER
#include "entry_point.h"
#include "test_support/catch.hh"
#include "io/program_path.h"

int main(int argc, char *const argv[])
{
    au::io::set_program_path_from_arg(argv[0]);
    init_fs_utf8();
    return Catch::Session().run(argc, argv);
}
