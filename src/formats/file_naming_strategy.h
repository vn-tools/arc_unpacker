#ifndef FORMATS_FILE_NAMING_STRATEGY_H
#define FORMATS_FILE_NAMING_STRATEGY_H
#include <memory>
#include <string>

enum class FileNamingStrategy : uint8_t
{
    Root = 1,
    Sibling = 2,
    Child = 3,
};

class FileNameDecorator
{
public:
    static std::string decorate(
        const FileNamingStrategy &strategy,
        const std::string &parent_file_name,
        const std::string &current_file_name);
};

#endif
