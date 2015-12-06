#include "util/algo/str.h"
#include <boost/algorithm/string.hpp>

using namespace au;

std::string util::algo::lower(const std::string &input)
{
    return boost::algorithm::to_lower_copy(input);
}

std::string util::algo::reverse(const std::string &input)
{
    std::string output(input);
    std::reverse(output.begin(), output.end());
    return output;
}

std::string util::algo::trim_to_zero(const std::string &input)
{
    return std::string(input.c_str());
}

bstr util::algo::trim_to_zero(const bstr &input)
{
    return bstr(input.get<const char>());
}
