#include <set>
#include "arg_parser.h"
#include "log.h"
#include "util/range.h"

using namespace au;

namespace
{
    struct Option
    {
        std::vector<std::string> names;
        std::string description;

        bool has_name(const std::string &name) const;
        virtual std::string get_invocation_help() const;
    };

    struct Flag : Option
    {
    };

    struct Switch : Option
    {
        std::string value_name;
        virtual std::string get_invocation_help() const override;
    };
}

static bool is_alphanumeric(const std::string &str)
{
    for (auto i : util::range(str.size()))
    {
        char c = str[i];
        if (c < '0' && c > '9' && c < 'a' && c > 'a' && c < 'A' && c > 'A')
            return false;
    }
    return true;
}

std::string Option::get_invocation_help() const
{
    std::string invocation;
    bool primary_long_option_shown = false;
    for (auto &name : names)
    {
        bool is_long_option = name.compare(0, 2, "--") == 0;
        if (is_long_option)
        {
            if (!invocation.size())
                invocation = "    ";
            if (primary_long_option_shown)
                break;
            primary_long_option_shown = true;
        }
        invocation += name + ", ";
    }
    invocation.erase(invocation.size() - 2);
    return invocation;
}

std::string Switch::get_invocation_help() const
{
    return Option::get_invocation_help() + "=" + value_name;
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

bool Option::has_name(const std::string &name) const
{
    auto normalized_name = strip_dashes(name);
    for (auto &option_name : names)
        if (strip_dashes(option_name) == normalized_name)
            return true;
    return false;
}

static std::vector<std::string> split_to_words(const std::string sentence)
{
    std::vector<std::string> words;
    size_t pos = 0, new_pos = 0;
    while (new_pos != sentence.size())
    {
        for (new_pos = pos; new_pos < sentence.size(); new_pos++)
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

static std::vector<std::string> word_wrap(const std::string &text, size_t max)
{
    auto words = split_to_words(text);

    std::vector<std::string> lines;
    std::string line;
    for (auto &word : words)
    {
        line += word;
        if (word.size() > 0 && word[word.size() - 1] == '\n')
        {
            lines.push_back(line);
            line = "";
        }
        else if (line.size() > max)
        {
            if (line.size() > 0)
                line.erase(line.size() - 1, 1);
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
    std::vector<std::unique_ptr<Option>> options;
    std::set<Flag*> flags;
    std::set<Switch*> switches;
    std::vector<std::string> stray;

    std::vector<std::pair<std::string, std::string>> switch_values;
    std::vector<std::string> set_flags;

    std::vector<Option*> help_items;
};

ArgParser::ArgParser() : p(new Priv)
{
}

ArgParser::~ArgParser()
{
}

void ArgParser::parse(const std::vector<std::string> &args)
{
    if (args.size() == 0)
        return;

    for (auto &arg : args)
    {
        std::string key = "";
        std::string value = "";
        if (::get_switch(arg, key, value))
        {
            p->switch_values.push_back(
                std::pair<std::string, std::string>(key, value));
        }
        else if (::get_flag(arg, value))
        {
            p->set_flags.push_back(value);
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

void ArgParser::register_flag(
    const std::initializer_list<std::string> &names,
    const std::string &description)
{
    std::unique_ptr<Flag> f(new Flag);
    f->names = names;
    f->description = description;
    p->flags.insert(f.get());
    p->help_items.push_back(f.get());
    p->options.push_back(std::move(f));
}

void ArgParser::register_switch(
    const std::initializer_list<std::string> &names,
    const std::string &value_name,
    const std::string &description)
{
    std::unique_ptr<Switch> sw(new Switch);
    sw->names = names;
    sw->description = description;
    sw->value_name = value_name;
    p->switches.insert(sw.get());
    p->help_items.push_back(sw.get());
    p->options.push_back(std::move(sw));
}

bool ArgParser::has_switch(const std::string &name) const
{
    for (auto &it : p->switch_values)
        if (it.first == strip_dashes(name))
            return true;
    return false;
}

const std::string ArgParser::get_switch(const std::string &name) const
{
    for (auto &it : p->switch_values)
        if (it.first == strip_dashes(name))
            return it.second;
    return "";
}

bool ArgParser::has_flag(const std::string &name) const
{
    for (auto &f : p->set_flags)
        if (f == strip_dashes(name))
            return true;
    return false;
}

const std::vector<std::string> ArgParser::get_stray() const
{
    return p->stray;
}

void ArgParser::print_help() const
{
    const auto max_line_size = 78;

    if (!p->help_items.size())
    {
        Log.info("No additional switches are available.\n");
        return;
    }

    size_t max_invocation_size = 0;
    for (auto &option : p->help_items)
    {
        max_invocation_size = std::max<size_t>(
            max_invocation_size, option->get_invocation_help().size());
    }
    max_invocation_size++;

    const auto max_description_size = max_line_size - max_invocation_size;

    for (auto &option : p->help_items)
    {
        auto invocation = option->get_invocation_help();
        auto description = option->description;

        auto tmp_size = invocation.size();
        Log.info(invocation);

        for (; tmp_size < max_invocation_size; tmp_size++)
            Log.info(" ");

        auto lines = word_wrap(description, max_description_size);
        for (auto line : lines)
        {
            Log.info(line);
            for (auto i : util::range(max_invocation_size))
                Log.info(" ");
        }
        Log.info("\n");
    }
}
