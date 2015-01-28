#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arg_parser.h"
#include "cli_helpers.h"
#include "options.h"
#include "output_files.h"
#include "virtual_file.h"

static char *get_default_output_path(char *input_path)
{
    char *output_path = strdup(input_path);
    output_path = realloc(output_path, strlen(output_path) + 1);
    strcat(output_path, "~");
    return output_path;
}

static void add_format_option(ArgParser *arg_parser, Options *options)
{
    arg_parser_add_help(
        arg_parser,
        "-f, --fmt FORMAT",
        "Selects the archive format.");

    if (arg_parser_has_switch(arg_parser, "-f"))
        options_set(options, "fmt", arg_parser_get_switch(arg_parser, "-f"));
    if (arg_parser_has_switch(arg_parser, "--fmt"))
        options_set(options, "fmt", arg_parser_get_switch(arg_parser, "--fmt"));

    // TODO: use archive factory
}

static void add_path_options(ArgParser *arg_parser, Options *options)
{
    Array *stray = arg_parser_get_stray(arg_parser);
    if (array_size(stray) < 2)
    {
        fprintf(stderr, "Required more arguments.\n");
        exit(1);
    }

    options_set(options, "input_path", array_get(stray, 1));
    options_set(options, "output_path", array_size(stray) == 2
        ? get_default_output_path(array_get(stray, 1))
        : array_get(stray, 2));
}

static void print_help(char *path_to_self, ArgParser *arg_parser, Options *options)
{
    printf("Usage: %s [options [arc_options] input_path [output_path]\n",
        path_to_self);

    puts("");
    puts("Unless output path is provided, the script is going to use path");
    puts("followed with a tilde (~).");
    puts("");
    puts("[options] can be:");
    puts("");

    arg_parser_print_help(arg_parser);
    arg_parser_clear_help(arg_parser);
    puts("");

    if (options_get(options, "archive") == NULL)
    {
        puts("[arc_options] depend on each archive and are required at runtime.");
        puts("See --help --fmt FORMAT to get detailed help for given archive.");
    }
    else
    {
        //TODO: call help decorator on target archive
        printf("[arc_options] specific to %s:\n", options_get(options, "fmt"));
        puts("");
        arg_parser_print_help(arg_parser);
    }
}

VirtualFile *test_file_creator(void *context __attribute__((unused)))
{
    VirtualFile *test_file = vf_create();
    vf_set_name(test_file, "data.txt");
    vf_set_data(test_file, "abc", 3);
    return test_file;
}

static void run(Options *options)
{
    //TODO: decide on archiver and unpack
    printf(
        "run\ninput = %s\noutput = %s\n",
        options_get(options, "input_path"),
        options_get(options, "output_path"));

    OutputFiles *output_files = output_files_create(options);
    output_files_save(output_files, &test_file_creator, NULL);
    output_files_destroy(output_files);
}

int main(int argc, char **argv)
{
    Options *options = options_create();
    ArgParser *arg_parser = arg_parser_create();
    arg_parser_parse(arg_parser, argc, argv);

    add_format_option(arg_parser, options);
    cli_add_quiet_option(arg_parser, options);
    cli_add_verbose_option(arg_parser, options);
    cli_add_help_option(arg_parser);

    if (cli_should_show_help(arg_parser))
    {
        print_help(argv[0], arg_parser, options);
    }
    else
    {
        add_path_options(arg_parser, options);
        run(options);
    }

    arg_parser_destroy(arg_parser);
    options_destroy(options);
    return 0;
}
