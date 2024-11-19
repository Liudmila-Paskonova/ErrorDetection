#ifndef SUPPORT_SUPPORT_SUPPORT_H
#define SUPPORT_SUPPORT_SUPPORT_H

#include <vector>
#include <filesystem>
#include <random>

namespace support
{
std::vector<std::filesystem::path> getNRandomFiles(const std::filesystem::path &dir, size_t n);

}; // namespace support
#endif
