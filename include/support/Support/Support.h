#ifndef SUPPORT_SUPPORT_SUPPORT_H
#define SUPPORT_SUPPORT_SUPPORT_H

#include <vector>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;

namespace support
{
std::vector<fs::path> getNRandomFiles(const fs::path &dir, size_t n);

}; // namespace support
#endif
