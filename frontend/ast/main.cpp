#include <iostream>
#include <fstream>
#include <sstream>
#include "extractor/treeSitter.h"
int
main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "Path to file with source code is required as well as destination!" << std::endl;
        return 1;
    }
    std::ifstream f(argv[1]);
    std::stringstream buffer;
    buffer << f.rdbuf();
    f.close();
    std::string strBuf = buffer.str();
    TreeSitter t(strBuf);
    t.getGraph(argv[2]);
}
