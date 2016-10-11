// Copyright (C) 2016 by rr-
//
// This file is part of arc_unpacker.
//
// arc_unpacker is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// arc_unpacker is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with arc_unpacker. If not, see <http://www.gnu.org/licenses/>.

#include "arg_parser.h"
#include "algo/format.h"
#include "algo/range.h"
#include "err.h"

using namespace au;

static const size_t line_size = 78;

namespace
{
    struct OptionImpl : Option
    {
        OptionImpl(const std::initializer_list<std::string> &names);
        virtual ~OptionImpl();
        virtual Option *set_description(const std::string &desc) override;
        bool has_name(const std::string &name) const;
        virtual std::string get_invocation_help() const;

        std::vector<std::string> names;
        std::string description;
        bool is_set;
    };

    struct FlagImpl final : OptionImpl, Flag
    {
        FlagImpl(const std::initializer_list<std::string> &names);
        virtual Flag *set_description(const std::string &desc) override;
    };

    struct SwitchImpl final : OptionImpl, Switch
    {
        SwitchImpl(const std::initializer_list<std::string> &names);
        virtual std::string get_invocation_help() const override;
        virtual Switch *set_description(const std::string &desc) override;
        virtual Switch *set_value_name(const std::string &name) override;
        virtual Switch *add_possible_value(
            const std::string &value, const std::string &description) override;
        virtual Switch *hide_possible_values() override;

        std::string value_name;
        std::string value;
        std::vector<std::pair<std::string, std::string>> possible_values;
        bool possible_values_hidden;
    };
}

OptionImpl::OptionImpl(const std::initializer_list<std::string> &names)
    : names(names), description("No description"), is_set(false)
{
}

OptionImpl::~OptionImpl()
{
}

std::string OptionImpl::get_invocation_help() const
{
    std::string invocation;
    bool primary_long_option_shown = false;
    for (const auto &name : names)
    {
        const auto is_long_option = name.compare(0, 2, "--") == 0;
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

bool OptionImpl::has_name(const std::string &name) const
{
    const auto name_normalized = name.substr(name.find_first_not_of('-'));
    for (const auto &other : names)
        if (name_normalized == other.substr(other.find_first_not_of('-')))
            return true;
    return false;
}

Option *OptionImpl::set_description(const std::string &desc)
{
    description = desc;
    return this;
}

FlagImpl::FlagImpl(const std::initializer_list<std::string> &names)
    : OptionImpl(names)
{
}

Flag *FlagImpl::set_description(const std::string &desc)
{
    OptionImpl::set_description(desc);
    return this;
}

SwitchImpl::SwitchImpl(const std::initializer_list<std::string> &names)
    : OptionImpl(names), possible_values_hidden(false)
{
}

std::string SwitchImpl::get_invocation_help() const
{
    return OptionImpl::get_invocation_help() + "=" + value_name;
}

Switch *SwitchImpl::set_description(const std::string &desc)
{
    OptionImpl::set_description(desc);
    return this;
}

Switch *SwitchImpl::set_value_name(const std::string &name)
{
    value_name = name;
    return this;
}

Switch *SwitchImpl::add_possible_value(
    const std::string &value, const std::string &description)
{
    possible_values.push_back({value, description});
    return this;
}

Switch *SwitchImpl::hide_possible_values()
{
    possible_values_hidden = true;
    return this;
}

static std::vector<std::string> split_to_words(const std::string &sentence)
{
    std::vector<std::string> words;
    size_t pos = 0, new_pos = 0;
    while ((new_pos = sentence.find_first_of(" \r\n\t", pos)) != sentence.npos)
    {
        words.push_back(sentence.substr(pos, new_pos + 1 - pos));
        pos = new_pos + 1;
    }
    words.push_back(sentence.substr(pos));
    return words;
}

static std::vector<std::string> word_wrap(
    const std::string &text, const size_t max)
{
    std::vector<std::string> lines;
    std::string line;
    for (const auto &word : split_to_words(text))
    {
        if (!word.empty() && word.back() == '\n')
        {
            lines.push_back(line + word);
            line = "";
        }
        else if (line.size() + word.size() >= max)
        {
            lines.push_back(line + "\n");
            line = word;
        }
        else
            line += word;
    }
    if (line != "")
        lines.push_back(line);
    return lines;
}

static std::string format_dictionary_as_table(
    const std::string &prefix,
    const std::vector<std::pair<std::string, std::string>> &dict)
{
    std::string out;
    size_t key_size = 0;
    for (const auto &it : dict)
        key_size = std::max(prefix.size () + it.first.size(), key_size);
    key_size += 2; // keep two spaces around value names
    const auto columns = (line_size - key_size) / key_size;
    const auto rows = (dict.size() + columns - 1) / columns;
    for (const auto y : algo::range(rows))
    {
        for (const auto x : algo::range(columns))
        {
            const auto i = x * rows + y;
            if (i >= dict.size())
                continue;
            out += prefix + dict[i].first + std::string(
                key_size - prefix.size() - dict[i].first.size(), ' ');
        }
        out += "\n";
    }
    return out;
}

static std::string format_dictionary_as_list(
    const std::string &prefix,
    const std::vector<std::pair<std::string, std::string>> &dict)
{
    size_t key_size = 0;
    for (const auto &it : dict)
        key_size = std::max(it.first.size() + prefix.size(), key_size);
    key_size += 2; // keep two spaces around parameter descriptions
    const auto description_size = line_size - key_size;
    std::string out;
    for (const auto &it : dict)
    {
        out += prefix + it.first + std::string(
            key_size - prefix.size() - it.first.size(), ' ');
        for (const auto &line : word_wrap(it.second, description_size))
            out += line + std::string(key_size, ' ');
        out += "\n";
    }
    return out;
}

static std::string format_switch_help(const SwitchImpl &sw, bool force)
{
    const auto &values = sw.possible_values;
    if (!values.size() || (sw.possible_values_hidden && !force))
        return "";
    auto out
        = algo::format("\nAvailable %s values:\n\n", sw.value_name.c_str());
    const bool use_descriptions = std::all_of(
        values.begin(), values.end(),
        [](const auto &it) { return !it.second.empty(); });
    std::vector<std::pair<std::string, std::string>> dict;
    for (const auto &it : values)
        dict.push_back({it.first, it.second});
    out += use_descriptions
        ? format_dictionary_as_list("- ", dict)
        : format_dictionary_as_table("- ", dict);
    return out;
}

static void print_switches(
    const Logger &logger, const std::vector<SwitchImpl*> &switches)
{
    for (const auto &sw : switches)
        logger.info(format_switch_help(*sw, false));
}

static void print_options(
    const Logger &logger,
    const std::vector<std::unique_ptr<OptionImpl>> &options)
{
    std::vector<std::pair<std::string, std::string>> dict;
    for (const auto &opt : options)
        dict.push_back({opt->get_invocation_help(), opt->description});
    logger.info(format_dictionary_as_list("", dict));
}

struct ArgParser::Priv final
{
    void check_names(const std::vector<std::string> &names);
    void parse_single_arg(const std::string &arg);

    std::vector<std::unique_ptr<OptionImpl>> options;
    std::vector<FlagImpl*> flags;
    std::vector<SwitchImpl*> switches;
    std::vector<std::string> stray;
};

void ArgParser::Priv::check_names(const std::vector<std::string> &names)
{
    for (const auto &name : names)
    for (const auto &option : options)
    {
        if (option->has_name(name))
        {
            throw std::logic_error(algo::format(
                "An option with name \"%s\" was already registered.",
                name.c_str()));
        }
    }
}

void ArgParser::Priv::parse_single_arg(const std::string &arg)
{
    if (arg[0] != '-')
    {
        stray.push_back(arg);
        return;
    }

    for (const auto &flag : flags)
    {
        if (!flag->has_name(arg))
            continue;
        flag->is_set = true;
        return;
    }

    const auto tmp = arg.find_first_of('=');
    if (tmp == std::string::npos)
        return;
    const auto key = arg.substr(0, tmp);
    const auto value = arg.substr(tmp + 1);

    for (const auto &sw : switches)
    {
        if (!sw->has_name(key))
            continue;

        if (sw->possible_values.size()
            && std::all_of(
                sw->possible_values.begin(),
                sw->possible_values.end(),
                [&](const auto &it) { return it.first != value; }))
        {
            throw err::UsageError(
                "Bad value for option \"" + key + "\".\n"
                + format_switch_help(*sw, true));
        }

        sw->is_set = true;
        sw->value = value;
        return;
    }
}

ArgParser::ArgParser() : p(new Priv)
{
}

ArgParser::~ArgParser()
{
}

void ArgParser::parse(const std::vector<std::string> &args)
{
    for (const auto &arg : args)
        p->parse_single_arg(arg);
}

Flag *ArgParser::register_flag(const std::initializer_list<std::string> &names)
{
    p->check_names(names);
    auto flag = std::make_unique<FlagImpl>(names);
    auto ret = flag.get();
    p->flags.push_back(ret);
    p->options.push_back(std::move(flag));
    return ret;
}

Switch *ArgParser::register_switch(
    const std::initializer_list<std::string> &names)
{
    p->check_names(names);
    auto switch_ = std::make_unique<SwitchImpl>(names);
    auto ret = switch_.get();
    p->switches.push_back(ret);
    p->options.push_back(std::move(switch_));
    return ret;
}

bool ArgParser::has_switch(const std::string &name) const
{
    for (const auto &sw : p->switches)
        if (sw->has_name(name))
            return sw->is_set;
    throw std::logic_error("Trying to use undefined switch \"" + name + "\"");
}

const std::string ArgParser::get_switch(const std::string &name) const
{
    for (const auto &sw : p->switches)
        if (sw->has_name(name))
            return sw->value;
    throw std::logic_error("Trying to use undefined switch \"" + name + "\"");
}

bool ArgParser::has_flag(const std::string &name) const
{
    for (const auto &f : p->flags)
        if (f->has_name(name))
            return f->is_set;
    throw std::logic_error("Trying to use undefined flag \"" + name + "\"");
}

const std::vector<std::string> ArgParser::get_stray() const
{
    return p->stray;
}

void ArgParser::print_help(const Logger &logger) const
{
    if (!p->options.size())
    {
        logger.info("No additional switches are available.\n");
        return;
    }

    print_options(logger, p->options);
    print_switches(logger, p->switches);
    logger.info("\n");
}
