#ifndef SUPPORT_DATABASE_METADATA_H
#define SUPPORT_DATABASE_METADATA_H

#include <iostream>
#include <string_view>
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <expected>
#include <vector>
#include <format>
#include <print>

namespace db
{
enum class ParseError { ErrOpenDB, ErrProcessStmt, ErrExec, PrimaryKeyNotUnique, KeyDoesNotExist };

struct Package {
    std::string subID;
    std::string probID;
    std::string userID;
    std::string lang;
    std::string status;
    Package() = default;
    Package(const unsigned char *subID_, const unsigned char *probID_, const unsigned char *userID_,
            const unsigned char *lang_, const unsigned char *status_);

    bool isEmpty() const;
};

class Database
{
    sqlite3 *db;
    std::string tableName;
    std::expected<int, std::string> rc;

    // template <typename... Ts>
    auto execute(int command, const ParseError &p, int err = SQLITE_ERROR) -> std::expected<int, std::string>;

  public:
    Database(const std::string &sql, const std::string &tableName = "dataset_info");

    /// Get information associated with submission_id
    Package getPackage(const std::string &subID);

    std::vector<std::string> getPairSolutions(const std::string &probID, const std::string &userID,
                                              const std::string &status);

    std::vector<Package> query(const char *sqlSt);

    ~Database();
};
}; // namespace db
#endif
