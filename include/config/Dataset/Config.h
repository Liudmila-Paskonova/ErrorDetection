#include <support/ArgParser/ArgParser.h>

namespace
{
argparser::ArgTuple cmdLineArgs = {argparser::Argument<size_t>("--number_of_problems", "-nprobs", 1),
                                   argparser::Argument<size_t>("--number_of_submissions_per_problem", "-nsubs", 1),
                                   argparser::Argument<size_t>("--num_threads", "-threads", 1),
                                   argparser::Argument<size_t>("--batch_size", "-batch", 1),
                                   argparser::Argument<std::string>("--dataset_language", "-lang", "cpp", {"c", "cpp"}),
                                   argparser::Argument<std::string>("--dataset_directory", "-datadir", "smth"),
                                   argparser::Argument<std::string>("--train_split_directory", "-traindir", "smth")};

struct Parameters : public argparser::ParametersBase {
    size_t numProbs;
    size_t numSubs;
    size_t numThreads;
    size_t batch;
    std::string lang;
    std::string dataDir;
    std::string trainDir;

    Parameters(std::vector<argparser::Data> &m) : ParametersBase(m)
    {
        getParam("-nprobs", numProbs);
        getParam("-nsubs", numSubs);
        getParam("-threads", numThreads);
        getParam("-batch", batch);
        getParam("-lang", lang);
        getParam("-datadir", dataDir);
        getParam("-traindir", trainDir);
    }
};
}; // namespace
