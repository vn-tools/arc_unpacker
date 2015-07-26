#include <iostream>
#include "arg_parser.h"

using namespace au;

static bool is_alphanumeric(std::string string)
{
    for (size_t i = 0; i < string.length(); i++)
    {
        char c = string[i];
        if (c < '0' && c > '9' && c < 'a' && c > 'a' && c < 'A' && c > 'A')
            return false;
    }
    return true;
}

static std::string strip_dashes(const std::string argument)
{
    std::string ret = argument;
    while (ret[0] == '-')
        ret.erase(0, 1);
    return ret;
}

static bool get_switch(
    const std::string argument, std::string &key, std::string &value)
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

static bool get_flag(const std::string argument, std::string &value)
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

static std::vector<std::string> create_words(const std::string sentence)
{
    std::vector<std::string> words;
    size_t pos = 0, new_pos = 0;
    while (new_pos != sentence.length())
    {
        for (new_pos = pos; new_pos < sentence.length(); new_pos++)
        {
            char c = sentence[new_pos];
            if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
                break;
        }
        words.push_back(sentence.substr(pos, new_pos + 1 - pos));
        pos = new_pos + 1;
    }
    return words;
}

static std::vector<std::string> word_wrap(
    const std::string sentence, size_t max)
{
    std::vector<std::string> words = create_words(sentence);
    std::vector<std::string> lines;
    std::string line;
    for (auto &word : words)
    {
        line += word;
        if (word.length() > 0 && word[word.length() - 1] == '\n')
        {
            lines.push_back(line);
            line = "";
        }
        else if (line.length() > max)
        {
            if (line.length() > 0)
                line.erase(line.length() - 1, 1);
            lines.push_back(line + "\n");
            line = "";
        }
    }
    if (line != "")
        lines.push_back(line);
    return lines;
}

struct ArgParser::Priv
{
    std::vector<std::string> flags;
    std::vector<std::pair<std::string, std::string>> switches;
    std::vector<std::string> stray;
    std::vector<std::pair<std::string, std::string>> help_items;
};

ArgParser::ArgParser() : p(new Priv)
{
}

ArgParser::~ArgParser()
{
}

void ArgParser::parse(std::vector<std::string> args)
{
    if (args.size() == 0)
        return;

    for (auto &arg : args)
    {
        std::string key = "";
        std::string value = "";
        if (::get_switch(arg, key, value))
        {
            p->switches.push_back(
                std::pair<std::string, std::string>(key, value));
        }
        else if (::get_flag(arg, value))
        {
            p->flags.push_back(value);
        }
        else
        {
            p->stray.push_back(std::string(arg));
        }
    }
}

void ArgParser::clear_help()
{
    p->help_items.clear();
}

void ArgParser::add_help(std::string invocation, std::string description)
{
    p->help_items.push_back(
        std::pair<std::string, std::string>(invocation, description));
}

bool ArgParser::has_switch(std::string key) const
{
    for (auto &it : p->switches)
        if (it.first == strip_dashes(key))
            return true;
    return false;
}

const std::string ArgParser::get_switch(std::string key) const
{
    for (auto &it : p->switches)
        if (it.first == strip_dashes(key))
            return it.second;
    return "";
}

bool ArgParser::has_flag(std::string flag) const
{
    for (auto &it : p->flags)
        if (it == strip_dashes(flag))
            return true;
    return false;
}

const std::vector<std::string> ArgParser::get_stray() const
{
    return p->stray;
}

void ArgParser::print_help() const
{
    const size_t max_invocation_length = 25;
    const size_t max_line_length = 78;
    const size_t max_description_length
        = max_line_length - max_invocation_length;

    if (p->help_items.size() == 0)
    {
        std::cout << "No additional switches are available.\n";
        return;
    }

    for (auto &item : p->help_items)
    {
        std::string invocation = item.first;
        std::string description = item.second;

        size_t tmp_length = invocation.length();
        if (tmp_length >= 2 && invocation.compare(0, 2, "--") == 0)
        {
            std::cout << "    ";
            tmp_length += 4;
        }
        std::cout << invocation;

        for (; tmp_length < max_invocation_length; tmp_length++)
            std::cout << " ";

        std::vector<std::string> lines = word_wrap(
            description, max_description_length);
        for (auto line : lines)
        {
            std::cout << line;
            for (size_t i = 0; i < max_invocation_length; i++)
                std::cout << " ";
        }
        std::cout << "\n";
    }
}
