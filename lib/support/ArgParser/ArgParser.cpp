#include <support/ArgParser/ArgParser.h>

bool
argparser::Data::operator==(const std::string &str) const
{
    return longArg == str || shortArg == str;
}
