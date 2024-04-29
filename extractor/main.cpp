#include "extractor.h"

static std::map<std::string, size_t> cmdArgs = {
    {"-maxlen", 0},   {"--max_path_length", 0}, {"-threads", 1},  {"--num_threads", 1},        {"-dir", 2},
    {"--dir", 2},     {"-predict", 3},          {"--predict", 3}, {"-batch_size", 4},          {"--batch_size", 4},
    {"-maxWidth", 5}, {"--max_path_width", 5},  {"-vectors", 6},  {"--export_code_vectors", 6}};

int
main(int argc, char *argv[])
{

    size_t maxLen = UINT32_MAX;
    size_t maxWidth = UINT32_MAX;
    size_t numThreads = 1;
    std::string dir;
    size_t batchSize = 1;
    bool exportVectors = false;

    // process cmd line params
    for (int i = 1; i < argc; ++i) {
        auto it = cmdArgs.find(argv[i]);
        if (it != cmdArgs.end()) {
            size_t n = it->second;
            std::stringstream value;
            switch (n) {
            case 0:
                value << argv[i + 1];
                value >> maxLen;
                break;
            case 1:
                value << argv[i + 1];
                value >> numThreads;
                break;
            case 2:
                value << argv[i + 1];
                value >> dir;
                break;
            case 3:
                value << argv[i + 1];
                value >> dir;
                break;
            case 4:
                value << argv[i + 1];
                value >> batchSize;
                break;
            case 5:
                value << argv[i + 1];
                value >> maxWidth;
                break;
            case 6:
                exportVectors = true;
                break;

            default:
                break;
            }
        }
    }

    try {
        Extractor e(1, maxLen, maxWidth, exportVectors);

        e.run(dir, batchSize, numThreads);

    } catch (const char *err) {
        std::cerr << err << std::endl;
        return 1;
    }
    return 0;
}
