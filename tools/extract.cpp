/*#########################################################################################################//
Tool for dataset preprocessing

Supposing the following structure of a raw datase,
  |- dataset_directory
  |----|- prob1_name
  |    |---- sub1_name
  |    |---- sub2_name
  |      ...
  |---- prob2_name
  |    |---- sub1_name
  |    |---- sub2_name
  |      ...
this tool will create a new dataset:
  |- train_split_directory
  |----|- train
  |    |---- sub1_name
  |    |---- sub2_name
  |----|- val
  |    |---- sub1_name
  |    |---- sub2_name

  --max_path_length         |-maxlen    |=
  --max_path_width          |-maxwidth  |=
  --num_threads             |-threads   |=
  --batch_size              |-batch     |=
  --export_code_vectors     |-vectors   |=
  --dataset_language        |-lang      |=
  --path_contexts_encoding  |-contexts  |=
  --tokens_encoding         |-tokens    |=
  --dataset_directory       |-dir       |=
  --metadata_database       |-tmetadata |=

//#########################################################################################################*/

#include <extractor/Extractor.h>
#include <support/ArgParser/ArgParser.h>

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

int
main(int argc, char *argv[])
{
    try {
        auto parser = argparser::make_parser<Parameters>(cmdLineArgs);
        auto params = parser.parse(argc, argv);
        extractor::Extractor e(params);
        e.run(params.dir, params.metadata);

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    }

    return 0;
}
