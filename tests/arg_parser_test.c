#include <assert.h>
#include <string.h>
#include "arg_parser.h"

void test_switch_missing()
{
    ArgParser *ap = arg_parser_create();
    assert(arg_parser_get_switch(ap, "-s") == NULL);
    assert(arg_parser_get_switch(ap, "--long") == NULL);
    assert(!arg_parser_has_switch(ap, "-s"));
    assert(!arg_parser_has_switch(ap, "--long"));
    arg_parser_destroy(ap);
}

void test_switch_is_not_a_flag()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"-f"};
    arg_parser_parse(ap, 1, argv);
    assert(arg_parser_get_switch(ap, "-f") == NULL);
    assert(!arg_parser_has_switch(ap, "-f"));
    arg_parser_destroy(ap);
}

void test_switch_short()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"-s=short"};
    arg_parser_parse(ap, 1, argv);
    assert(strcmp("short", arg_parser_get_switch(ap, "-s")) == 0);
    assert(strcmp("short", arg_parser_get_switch(ap, "s")) == 0);
    assert(arg_parser_has_switch(ap, "-s"));
    assert(arg_parser_has_switch(ap, "s"));
    arg_parser_destroy(ap);
}

void test_switch_long()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"--long=long"};
    arg_parser_parse(ap, 1, argv);
    assert(strcmp("long", arg_parser_get_switch(ap, "--long")) == 0);
    assert(strcmp("long", arg_parser_get_switch(ap, "-long")) == 0);
    assert(strcmp("long", arg_parser_get_switch(ap, "long")) == 0);
    assert(arg_parser_has_switch(ap, "--long"));
    assert(arg_parser_has_switch(ap, "-long"));
    assert(arg_parser_has_switch(ap, "long"));
    arg_parser_destroy(ap);
}

void test_switch_overriding_short()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[2] = {"-s=short1", "-s=short2"};
    arg_parser_parse(ap, 2, argv);
    assert(strcmp("short2", arg_parser_get_switch(ap, "-s")) == 0);
    arg_parser_destroy(ap);
}

void test_switch_overriding_long()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[2] = {"--long=long1", "--long=long2"};
    arg_parser_parse(ap, 2, argv);
    assert(strcmp("long2", arg_parser_get_switch(ap, "--long")) == 0);
    arg_parser_destroy(ap);
}

void test_switch_with_space()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"--switch=long switch"};
    arg_parser_parse(ap, 1, argv);
    assert(strcmp("long switch", arg_parser_get_switch(ap, "--switch")) == 0);
    arg_parser_destroy(ap);
}

void test_switch_empty_value()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"--switch="};
    arg_parser_parse(ap, 1, argv);
    assert(strcmp("", arg_parser_get_switch(ap, "--switch")) == 0);
    arg_parser_destroy(ap);
}

void test_flag_missing()
{
    ArgParser *ap = arg_parser_create();
    assert(!arg_parser_has_flag(ap, "nope"));
    arg_parser_destroy(ap);
}

void test_flag()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"--flag"};
    arg_parser_parse(ap, 1, argv);
    assert(arg_parser_has_flag(ap, "flag"));
    arg_parser_destroy(ap);
}

void test_flag_mixed_with_stray()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[2] = {"--flag", "stray"};
    arg_parser_parse(ap, 2, argv);
    assert(arg_parser_has_flag(ap, "flag"));
    Array *stray = arg_parser_get_stray(ap);
    assert(1 == array_size(stray));
    assert(strcmp("stray", (const char*)array_get(stray, 0)) == 0);
    arg_parser_destroy(ap);
}

void test_stray_missing()
{
    ArgParser *ap = arg_parser_create();
    Array *stray = arg_parser_get_stray(ap);
    assert(0 == array_size(stray));
    arg_parser_destroy(ap);
}

void test_stray()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[2] = {"stray1", "stray2"};
    arg_parser_parse(ap, 2, argv);
    Array *stray = arg_parser_get_stray(ap);
    assert(2 == array_size(stray));
    assert(strcmp("stray1", (const char*)array_get(stray, 0)) == 0);
    assert(strcmp("stray2", (const char*)array_get(stray, 1)) == 0);
    arg_parser_destroy(ap);
}

void test_stray_with_space()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[1] = {"long stray"};
    arg_parser_parse(ap, 1, argv);
    Array *stray = arg_parser_get_stray(ap);
    assert(1 == array_size(stray));
    assert(strcmp("long stray", (const char*)array_get(stray, 0)) == 0);
    arg_parser_destroy(ap);
}

void test_mixed_types()
{
    ArgParser *ap = arg_parser_create();
    const char *argv[5] =
    {
        "stray1",
        "--switch=s",
        "--flag1",
        "stray2",
        "--flag2"
    };
    arg_parser_parse(ap, 5, argv);

    assert(strcmp("s", arg_parser_get_switch(ap, "--switch")) == 0);
    assert(arg_parser_has_flag(ap, "flag1"));
    assert(arg_parser_has_flag(ap, "flag2"));

    Array *stray = arg_parser_get_stray(ap);
    assert(2 == array_size(stray));
    assert(strcmp("stray1", (const char*)array_get(stray, 0)) == 0);
    assert(strcmp("stray2", (const char*)array_get(stray, 1)) == 0);

    arg_parser_destroy(ap);
}

#if 0
#endif

int main(void)
{
    test_switch_missing();
    test_switch_is_not_a_flag();
    test_switch_short();
    test_switch_long();
    // TODO: make this pass
    #if 0
    test_switch_overriding_short();
    test_switch_overriding_long();
    #endif
    test_switch_with_space();
    test_switch_empty_value();
    test_flag_missing();
    test_flag();
    test_flag_mixed_with_stray();
    test_stray_missing();
    test_stray();
    test_stray_with_space();
    test_mixed_types();
    return 0;
}
