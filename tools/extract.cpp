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
  |      ...

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

struct Parameters : public argparser::Arguments {
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

    Parameters()
    {
        using namespace argparser;
        addParam<"-maxlen", "--max_path_length">(maxLen, NaturalRangeArgument<>(1, {1, 20}));
        addParam<"-maxwidth", "--max_path_width">(maxWidth, NaturalRangeArgument<>(1, {1, 20}));
        addParam<"-threads", "--num_threads">(numThreads, NaturalRangeArgument<>(1, {1, 16}));
        addParam<"-batch", "--batch_size">(batch, NaturalRangeArgument<>(1, {1, 20}));
        addParam<"-vectors", "--export_code_vectors">(exportVectors, CostrainedArgument());
        addParam<"-lang", "--dataset_language">(lang, CostrainedArgument<std::string>("cpp", {"c", "cpp"}));
        addParam<"-contexts", "--path_contexts_encoding">(contexts,
                                                          CostrainedArgument<std::string>("tpt", {"tpt", "rt"}));
        addParam<"-tokens", "--tokens_encoding">(tokens, CostrainedArgument<size_t>(0, {0, 1}));
        addParam<"-dir", "--dataset_directory">(dir, DirectoryArgument<std::string>("/home"));
        addParam<"-metadata", "--metadata_database">(metadata, DirectoryArgument<std::string>("/home"));
    }
};

int
main(int argc, char *argv[])
{
    try {
        Parameters params;
        params.parse(argc, argv);
        extractor::Extractor e(params);
        e.run(params.dir, params.metadata);

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    }

    return 0;
}
