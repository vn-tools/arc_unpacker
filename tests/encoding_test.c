#include <stdlib.h>
#include "encoding.h"
#include "assert.h"

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
    assert_that(convert(sjis, 41, &output, &output_size, "SJIS", "UTF-8"));
    assert_equalsn(utf8, output, 61);
    free(output);
}

void test_utf8_to_sjis()
{
    char *output = NULL;
    size_t output_size = 0;
    assert_that(convert(utf8, 61, &output, &output_size, "UTF-8", "SJIS"));
    assert_equalsn(sjis, output, 41);
    free(output);
}

int main(void)
{
    test_sjis_to_utf8();
    test_utf8_to_sjis();
    return 0;
}
