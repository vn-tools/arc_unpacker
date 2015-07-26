#include "util/itos.h"

std::string au::util::itos(int i)
{
    if (i == 0)
        return "0";
    if (i < 0)
        return "-" + itos(- i);

    std::string x;
    while (i)
    {
        char c = (i % 10) + '0';
        x = c + x;
        i /= 10;
    }
    return x;
}

std::string au::util::itos(int i, size_t length)
{
    std::string ret = itos(i);
    while (ret.length() < length)
        ret = "0" + ret;
    return ret;
}
