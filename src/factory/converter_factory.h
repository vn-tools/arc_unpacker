#ifndef FACTORY_CONVERTER_FACTORY_H
#define FACTORY_CONVERTER_FACTORY_H
#include <vector>
#include "formats/converter.h"

class ConverterFactory final
{
public:
    ConverterFactory();
    ~ConverterFactory();
    const std::vector<std::string> get_formats() const;
    Converter *create_converter(std::string format) const;
private:
    struct Internals;
    Internals *internals;
};

#endif
