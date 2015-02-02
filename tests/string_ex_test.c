#include <stdlib.h>
#include <string.h>
#include "assert_ex.h"
#include "string_ex.h"

// sjis の"あいうえおかきくけこさしすせそたちつてと"
const char sjis[] = {
    "\x82\xA0\x82\xA2\x82\xA4\x82\xA6\x82\xA8"
    "\x82\xA9\x82\xAB\x82\xAD\x82\xAF\x82\xB1"
    "\x82\xB3\x82\xB5\x82\xB7\x82\xB9\x82\xBB"
    "\x82\xBD\x82\xBF\x82\xC2\x82\xC4\x82\xC6"
    "\x00" };

// utf8 の"あいうえおかきくけこさしすせそたちつてと"
const char utf8[] = {
    "\xE3\x81\x82\xE3\x81\x84\xE3\x81\x86\xE3"
    "\x81\x88\xE3\x81\x8A\xE3\x81\x8B\xE3\x81"
    "\x8D\xE3\x81\x8F\xE3\x81\x91\xE3\x81\x93"
    "\xE3\x81\x95\xE3\x81\x97\xE3\x81\x99\xE3"
    "\x81\x9B\xE3\x81\x9D\xE3\x81\x9F\xE3\x81"
    "\xA1\xE3\x81\xA4\xE3\x81\xA6\xE3\x81\xA8"
    "\x00" };

void test_sjis_to_utf8()
{
    char *output = NULL;
    size_t output_size = 0;
    assert_that(convert_encoding(
        sjis, 41,
        &output, &output_size,
        "SJIS", "UTF-8"));
    assert_equalsn(utf8, output, 61);
    free(output);
}

void test_utf8_to_sjis()
{
    char *output = NULL;
    size_t output_size = 0;
    assert_that(convert_encoding(
        utf8, 61,
        &output, &output_size,
        "UTF-8", "SJIS"));
    assert_equalsn(sjis, output, 41);
    free(output);
}

void test_trim_right_nothing_to_trim()
{
    char *target = strdup("abc");
    trim_right(target, " ");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_one()
{
    char *target = strdup("abc ");
    trim_right(target, " ");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_two()
{
    char *target = strdup("abc  ");
    trim_right(target, " ");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_different_characters()
{
    char *target = strdup("abc \t ");
    trim_right(target, " \t");
    assert_equals("abc", target);
    free(target);
}

void test_trim_right_trim_to_empty_string()
{
    char *target = strdup(" \t ");
    trim_right(target, " \t");
    assert_equals("", target);
    free(target);
}

void test_zlib_inflate()
{
    const char *input =
        "\x78\xda\xcb\xc9\x4c\x4b\x55\xc8"
        "\x2c\x56\x48\xce\x4f\x49\xe5\x02"
        "\x00\x20\xc1\x04\x62\x0a";
    char *output;
    size_t output_size;

    assert_that(zlib_inflate(input, 21, &output, &output_size));
    assert_equali(13, output_size);
    assert_equals("life is code\n", output);
    free(output);

    output = NULL;

    assert_that(zlib_inflate(input, 21, &output, NULL));
    assert_equali(13, output_size);
    assert_equals("life is code\n", output);
    free(output);
}

int main(void)
{
    test_zlib_inflate();
    test_sjis_to_utf8();
    test_utf8_to_sjis();
    test_trim_right_nothing_to_trim();
    test_trim_right_trim_one();
    test_trim_right_trim_two();
    test_trim_right_trim_different_characters();
    test_trim_right_trim_to_empty_string();
    return 0;
}
