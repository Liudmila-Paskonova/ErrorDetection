#include <support/ArgParser/ArgParser.h>

bool
argparser::KeyParam::operator<(const KeyParam &k2) const
{
    return this->sharg < k2.sharg;
}

void
argparser::Arguments::parse(int argc, char *argv[])
{
    for (int argIndex = 1; argIndex < argc; ++argIndex) {
        std::string arg{argv[argIndex]};
        std::string param;
        if (arg.starts_with("-")) {
            // arg is a key
            auto it = std::find_if(parameters.begin(), parameters.end(),
                                   [&arg](const auto &p) { return p.first.sharg == arg; });

            auto vit =
                std::find_if(values.begin(), values.end(), [&arg](const auto &p) { return p.first.sharg == arg; });

            if (it == parameters.end()) {
                throw "Wrong key!\n";
            }
            ++argIndex;

            if (argIndex == argc || std::string(argv[argIndex]).starts_with("-")) {
                // arg is a CostrainedArgument()
                it->second->setValue(vit->second, "1");
                --argIndex;
            } else {
                // value
                param = argv[argIndex];
                it->second->setValue(vit->second, param);
            }
        } else {
            // arg is not a key
            throw std::format("Argument {} is not a key", arg);
        }
    }
}
