#include "arg_parser.h"
#include "err.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

using namespace au;

static const size_t max_line_size = 78;

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

bool OptionImpl::has_name(const std::string &name) const
{
    auto name_normalized = name.substr(name.find_first_not_of('-'));
    for (auto &other : names)
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
    description = desc;
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
    description = desc;
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
    possible_values.push_back(std::pair<std::string, std::string>(
        value, description));
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

static void print_possible_values_table(
    const std::vector<std::pair<std::string, std::string>> &possible_values)
{
    auto max_value_size = 0;
    for (auto &it : possible_values)
        max_value_size = std::max<size_t>(it.first.size(), max_value_size);
    auto columns = (max_line_size - max_value_size - 2) / max_value_size;
    auto rows = (possible_values.size() + columns - 1) / columns;
    for (auto y : util::range(rows))
    {
        for (auto x : util::range(columns))
        {
            auto i = x * rows + y;
            if (i >= possible_values.size())
                continue;
            Log.info(
                util::format(
                    util::format("- %%-%ds ", max_value_size),
                    possible_values[i].first.c_str()));
        }
        Log.info("\n");
    }
}

static void print_possible_values_list(
    const std::vector<std::pair<std::string, std::string>> &possible_values)
{
    auto max_value_size = 0;
    for (auto &it : possible_values)
        max_value_size = std::max<size_t>(it.first.size(), max_value_size);
    for (auto &it : possible_values)
    {
        Log.info(
            util::format(
                util::format("- %%-%ds  %%s\n", max_value_size),
                it.first.c_str(),
                it.second.c_str()));
    }
}

static void print_switches(const std::vector<SwitchImpl*> &switches)
{
    for (auto &sw : switches)
    {
        auto possible_values = sw->possible_values;
        if (!possible_values.size() || sw->possible_values_hidden)
            continue;
        Log.info(util::format(
            "\nAvailable %s values:\n\n", sw->value_name.c_str()));
        bool use_descriptions = false;
        for (auto &p : possible_values)
            if (p.second != "")
                use_descriptions = true;
        if (use_descriptions)
            print_possible_values_list(possible_values);
        else
            print_possible_values_table(possible_values);
    }
}

static void print_options(
    const std::vector<std::unique_ptr<OptionImpl>> &options)
{
    size_t max_invocation_size = 0;
    for (auto &option : options)
        max_invocation_size = std::max<size_t>(
            max_invocation_size, option->get_invocation_help().size());
    max_invocation_size += 2; // keep two spaces
    const auto max_description_size = max_line_size - max_invocation_size;
    for (auto &option : options)
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
    for (auto &name : names)
    for (auto &option : options)
    {
        if (option->has_name(name))
        {
            throw std::logic_error(util::format(
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

    for (auto &flag : flags)
    {
        if (!flag->has_name(arg))
            continue;
        flag->is_set = true;
        return;
    }

    auto tmp = arg.find_first_of('=');
    if (tmp == std::string::npos)
        return;
    auto key = arg.substr(0, tmp);
    auto value = arg.substr(tmp + 1);

    for (auto &sw : switches)
    {
        if (!sw->has_name(key))
            continue;

        if (sw->possible_values.size())
        {
            bool found = false;
            for (auto &it : sw->possible_values)
                if (it.first == value)
                    found = true;
            if (!found)
            {
                throw err::UsageError(
                    "Bad value for option \"" + key + "\". "
                    "See --help for usage.");
            }
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
    for (auto &arg : args)
        p->parse_single_arg(arg);
}

Flag *ArgParser::register_flag(const std::initializer_list<std::string> &names)
{
    p->check_names(names);
    std::unique_ptr<FlagImpl> f(new FlagImpl(names));
    auto ret = f.get();
    p->flags.push_back(f.get());
    p->options.push_back(std::move(f));
    return ret;
}

Switch *ArgParser::register_switch(
    const std::initializer_list<std::string> &names)
{
    p->check_names(names);
    std::unique_ptr<SwitchImpl> sw(new SwitchImpl(names));
    auto ret = sw.get();
    p->switches.push_back(sw.get());
    p->options.push_back(std::move(sw));
    return ret;
}

bool ArgParser::has_switch(const std::string &name) const
{
    for (auto &sw : p->switches)
        if (sw->has_name(name))
            return sw->is_set;
    throw std::logic_error("Trying to use undefined switch \"" + name + "\"");
}

const std::string ArgParser::get_switch(const std::string &name) const
{
    for (auto &sw : p->switches)
        if (sw->has_name(name))
            return sw->value;
    throw std::logic_error("Trying to use undefined switch \"" + name + "\"");
}

bool ArgParser::has_flag(const std::string &name) const
{
    for (auto &f : p->flags)
        if (f->has_name(name))
            return f->is_set;
    throw std::logic_error("Trying to use undefined flag \"" + name + "\"");
}

const std::vector<std::string> ArgParser::get_stray() const
{
    return p->stray;
}

void ArgParser::print_help() const
{
    if (!p->options.size())
    {
        Log.info("No additional switches are available.\n");
        return;
    }

    print_options(p->options);
    print_switches(p->switches);
    Log.info("\n");
}
