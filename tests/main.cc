#define CATCH_CONFIG_RUNNER
#include "compat/entry_point.h"
#include "test_support/catch.hh"

int main(int argc, char *const argv[])
{
    init_fs_utf8();
    return Catch::Session().run(argc, argv);
}
