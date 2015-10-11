#include "arg_parser.h"
#include "err.h"
#include "log.h"
#include "util/format.h"
#include "util/range.h"

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

static std::string format_possible_values_table(
    const std::vector<std::pair<std::string, std::string>> &values)
{
    std::string out;
    auto value_size = 0;
    for (const auto &it : values)
        value_size = std::max<size_t>(it.first.size(), value_size);
    value_size += 2; // keep two spaces around value names
    const auto columns = (line_size - value_size) / value_size;
    const auto rows = (values.size() + columns - 1) / columns;
    for (const auto y : util::range(rows))
    {
        for (const auto x : util::range(columns))
        {
            const auto i = x * rows + y;
            if (i < values.size())
            {
                out += util::format(
                    "- %-*s ", value_size, values[i].first.c_str());
            }
        }
        out += "\n";
    }
    return out;
}

static std::string format_possible_values_list(
    const std::vector<std::pair<std::string, std::string>> &possible_values)
{
    size_t value_size = 0;
    for (const auto &it : possible_values)
        value_size = std::max(it.first.size(), value_size);
    std::string out;
    for (const auto &it : possible_values)
    {
        out += util::format(
            "- %-*s  %s\n", value_size, it.first.c_str(), it.second.c_str());
    }
    return out;
}

static std::string format_switch_help(const SwitchImpl &sw)
{
    const auto &values = sw.possible_values;
    if (!values.size() || sw.possible_values_hidden)
        return "";
    auto out
        = util::format("\nAvailable %s values:\n\n", sw.value_name.c_str());
    const bool use_descriptions = std::all_of(
        values.begin(), values.end(),
        [](const auto &it) { return !it.second.empty(); });
    out += use_descriptions
        ? format_possible_values_list(values)
        : format_possible_values_table(values);
    return out;
}

static void print_switches(const std::vector<SwitchImpl*> &switches)
{
    for (const auto &sw : switches)
        Log.info(format_switch_help(*sw));
}

static void print_options(
    const std::vector<std::unique_ptr<OptionImpl>> &options)
{
    size_t invocation_size = 0;
    for (const auto &opt : options)
        invocation_size = std::max<size_t>(
            invocation_size, opt->get_invocation_help().size());
    invocation_size += 2; // keep two spaces around parameter descriptions
    const auto description_size = line_size - invocation_size;
    for (const auto &opt : options)
    {
        Log.info("%-*s", invocation_size, opt->get_invocation_help().c_str());
        for (const auto &line : word_wrap(opt->description, description_size))
            Log.info(line + std::string(invocation_size, ' '));
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
    for (const auto &name : names)
    for (const auto &option : options)
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
                + format_switch_help(*sw));
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
