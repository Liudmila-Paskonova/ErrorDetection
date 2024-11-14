#include <support/ArgParser/ArgParser.h>

namespace
{
argparser::ArgTuple cmdLineArgs = {
    argparser::Argument<size_t>("--max_path_length", "-maxlen", UINT32_MAX),
    argparser::Argument<size_t>("--max_path_width", "-maxwidth", UINT32_MAX),
    argparser::Argument<size_t>("--num_threads", "-threads", 1),
    argparser::Argument<size_t>("--batch_size", "-batch", 1),
    argparser::Argument<bool>("--export_code_vectors", "-vectors", false),
    argparser::Argument<std::string>("--dataset_language", "-lang", "cpp", {"c", "cpp"}),
    argparser::Argument<std::string>("--path_contexts_encoding", "-contexts", "tpt", {"tpt", "rt"}),
    argparser::Argument<size_t>("--tokens_encoding", "-tokens", 0, {0, 1}),
    argparser::Argument<std::string>("--dataset_directory", "-dir", "smth"),
    argparser::Argument<std::string>("--metadata_database", "-metadata", "smth")};

struct Parameters : public argparser::ParametersBase {
    size_t maxLen;
    size_t maxWidth;
    size_t numThreads;
    size_t batch;
    bool exportVectors;
    std::string lang;
    std::string contexts;
    size_t tokens;
    std::string dir;
    std::string metadata;

    Parameters(std::vector<argparser::Data> &m) : ParametersBase(m)
    {
        getParam("-maxlen", maxLen);
        getParam("-maxwidth", maxWidth);
        getParam("-threads", numThreads);
        getParam("-batch", batch);
        getParam("-vectors", exportVectors);
        getParam("-lang", lang);
        getParam("-contexts", contexts);
        getParam("-tokens", tokens);
        getParam("-dir", dir);
        getParam("-metadata", metadata);
    }
};
}; // namespace
