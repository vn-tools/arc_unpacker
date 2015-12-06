#include "util/algo/str.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/hex.hpp>

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

std::string util::algo::unhex(const std::string &input)
{
    std::string output;
    boost::algorithm::unhex(input, std::back_inserter(output));
    return output;
}

std::string util::algo::replace_all(
    const std::string &input,
    const std::string &from,
    const std::string &to)
{
    std::string output(input);
    boost::replace_all(output, from, to);
    return output;
}


namespace au {
namespace util {
namespace algo {

    template<> int from_string(const std::string &input)
    {
        return boost::lexical_cast<int>(input);
    }

    template<> float from_string(const std::string &input)
    {
        return boost::lexical_cast<float>(input);
    }

} } }
