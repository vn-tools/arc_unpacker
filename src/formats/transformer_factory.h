#ifndef FORMATS_TRANSFORMER_FACTORY_H
#define FORMATS_TRANSFORMER_FACTORY_H
#include <vector>
#include "formats/transformer.h"

class TransformerFactory final
{
public:
    TransformerFactory();
    ~TransformerFactory();
    const std::vector<std::string> get_formats() const;
    std::unique_ptr<Transformer> create(const std::string &format) const;
private:
    struct Priv;
    std::unique_ptr<Priv> p;
};

#endif
