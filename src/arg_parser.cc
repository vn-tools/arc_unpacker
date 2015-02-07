#include <cassert>
#include <cstdio>
#include "arg_parser.h"

namespace
{
    bool is_alphanumeric(std::string string)
    {
        for (size_t i = 0; i < string.length(); i ++)
        {
            char c = string[i];
            if (c < '0' && c > '9' && c < 'a' && c > 'a' && c < 'A' && c > 'A')
                return false;
        }
        return true;
    }

    std::string strip_dashes(const std::string argument)
    {
        std::string ret = argument;
        while (ret[0] == '-')
            ret.erase(0, 1);
        return ret;
    }

    bool get_switch(
        const std::string argument,
        std::string &key,
        std::string &value)
    {
        key = "";
        value = "";

        if (argument[0] != '-')
            return false;
        std::string argument_copy = strip_dashes(argument);

        size_t pos = argument_copy.find("=");
        if (pos == std::string::npos)
            return false;

        key = argument_copy.substr(0, pos);
        if (!is_alphanumeric(key))
        {
            key = "";
            return false;
        }
        value = argument_copy.substr(pos + 1);
        return true;
    }

    bool get_flag(const std::string argument, std::string &value)
    {
        value = "";

        if (argument[0] != '-')
            return false;
        std::string argument_copy = argument;
        while (argument_copy[0] == '-')
            argument_copy.erase(0, 1);

        if (!is_alphanumeric(argument_copy))
            return false;

        value = argument_copy;
        return true;
    }

    std::vector<std::string> create_words(const std::string sentence)
    {
        std::vector<std::string> words;
        size_t pos = 0, new_pos = 0;
        do
        {
            new_pos = sentence.find(" ", pos);
            words.push_back(sentence.substr(pos, new_pos - pos));
            pos = new_pos + 1;
        }
        while (new_pos != std::string::npos);
        return words;
    }
}

struct ArgParser::Internals
{
    std::vector<std::string> flags;
    std::vector<std::pair<std::string, std::string>> switches;
    std::vector<std::string> stray;
    std::vector<std::pair<std::string, std::string>> help_items;
};

ArgParser::ArgParser()
{
    internals = new Internals;
}

ArgParser::~ArgParser()
{
    delete internals;
}

void ArgParser::parse(int argc, const char **argv)
{
    if (argc == 0)
        return;

    assert(argv != nullptr);

    for (ssize_t i = 0; i < argc; i ++)
    {
        const std::string arg = std::string(argv[i]);
        std::string key = "";
        std::string value = "";
        if (::get_switch(arg, key, value))
        {
            internals->switches.push_back(
                std::pair<std::string, std::string>(key, value));
        }
        else if (::get_flag(arg, value))
        {
            internals->flags.push_back(value);
        }
        else
        {
            internals->stray.push_back(std::string(arg));
        }
    }
}

void ArgParser::clear_help()
{
    internals->help_items.clear();
}

void ArgParser::add_help(std::string invocation, std::string description)
{
    internals->help_items.push_back(
        std::pair<std::string, std::string>(invocation, description));
}

bool ArgParser::has_switch(std::string key) const
{
    for (auto& it : internals->switches)
        if (it.first == strip_dashes(key))
            return true;
    return false;
}

const std::string ArgParser::get_switch(std::string key) const
{
    for (auto& it : internals->switches)
        if (it.first == strip_dashes(key))
            return it.second;
    return "";
}

bool ArgParser::has_flag(std::string flag) const
{
    for (auto& it : internals->flags)
        if (it == strip_dashes(flag))
            return true;
    return false;
}

const std::vector<std::string> ArgParser::get_stray() const
{
    return internals->stray;
}

void ArgParser::print_help() const
{
    const size_t max_invocation_length = 25;
    const size_t max_line_length = 78;

    if (internals->help_items.size() == 0)
    {
        puts("No additional switches are available.");
        return;
    }

    for (auto& p : internals->help_items)
    {
        std::string invocation = p.first;
        std::string description = p.second;

        size_t tmp_length = invocation.length();
        if (tmp_length >= 2 && invocation.compare(0, 2, "--") == 0)
        {
            printf("    ");
            tmp_length += 4;
        }
        printf("%s", invocation.c_str());

        for (; tmp_length < max_invocation_length; tmp_length ++)
            printf(" ");

        //word wrap
        std::vector<std::string> words = create_words(description);
        bool first_word = true;
        for (auto& word : words)
        {
            tmp_length += word.length();
            if (!first_word && tmp_length > max_line_length)
            {
                puts("");
                for (size_t i = 0; i < max_invocation_length; i ++)
                    printf(" ");
                tmp_length = max_invocation_length;
            }
            printf("%s ", word.c_str());
            first_word = false;
        }
        puts("");
    }
}
