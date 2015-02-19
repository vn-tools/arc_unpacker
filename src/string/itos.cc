#include "string/itos.h"

std::string itos(int i)
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
