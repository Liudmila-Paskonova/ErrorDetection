#include <support/Support/Support.h>

std::vector<fs::path>
support::getNRandomFiles(const fs::path &dir, size_t n)
{
    std::vector<fs::path> files;
    for (const auto &sub : fs::directory_iterator(dir)) {
        if (sub.is_regular_file()) {
            files.push_back(sub.path());
        }
    }
    if (n >= files.size()) {
        return files;
    }

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(files.begin(), files.end(), g);
    return std::vector<fs::path>(files.begin(), files.begin() + n);
}
