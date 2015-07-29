#include "util/itos.h"
#include "util/format.h"

std::string au::util::itos(int i)
{
    return au::util::format("%d", i);
}

std::string au::util::itos(int i, size_t length)
{
    return au::util::format("%0" + itos(length) + "d", i);
}
