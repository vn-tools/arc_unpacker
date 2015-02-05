#ifndef ARG_PARSER_H
#define ARG_PARSER_H
#include <string>
#include <vector>
#include <utility>

class ArgParser final
{
public:
    void clear_help();
    void add_help(const std::string invocation, const std::string description);
    void print_help() const;

    void parse(int argc, const char **argv);

    bool has_flag(const std::string argument) const;
    bool has_switch(const std::string key) const;

    const std::string get_switch(const std::string key) const;
    std::vector<std::string> get_stray();

private:
    std::vector<std::string> flags;
    std::vector<std::pair<std::string, std::string>> switches;
    std::vector<std::string> stray;
    std::vector<std::pair<std::string, std::string>> help_items;
};

#endif
