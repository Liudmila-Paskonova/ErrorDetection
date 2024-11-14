#include <support/Database/Metadata.h>

bool
db::Package::isEmpty() const
{
    if (subID.empty() || probID.empty() || userID.empty() || lang.empty() || status.empty()) {
        return true;
    }
    return false;
}

db::Package::Package(const unsigned char *subID_, const unsigned char *probID_, const unsigned char *userID_,
                     const unsigned char *lang_, const unsigned char *status_)
    : subID(reinterpret_cast<const char *>(subID_)), probID(reinterpret_cast<const char *>(probID_)),
      userID(reinterpret_cast<const char *>(userID_)), lang(reinterpret_cast<const char *>(lang_)),
      status(reinterpret_cast<const char *>(status_))
{
}

auto
db::Database::execute(int command, const ParseError &p, int err) -> std::expected<int, std::string>
{
    sqlite3_stmt *stmt = nullptr;
    std::string res;
    if (command == err) {
        bool flag = false;
        std::string errMsg;
        switch (p) {
        case ParseError::KeyDoesNotExist:
            errMsg = "Key doesn't exist";
            flag = false;
            break;
            // clean allocated memory
            if (stmt) {
                sqlite3_finalize(stmt);
            }
            if (db) {
                sqlite3_close(db);
            }
        case ParseError::ErrOpenDB:
            errMsg = "Failed to open database";
            flag = true;
            break;
        case ParseError::ErrProcessStmt:
            errMsg = "Failed to prepare statement";
            flag = true;
            break;
        case ParseError::ErrExec:
            errMsg = "Failed to execute statement";
            flag = true;
            break;
        case ParseError::PrimaryKeyNotUnique:
            errMsg = "Primary key is not unique";
            flag = true;
            break;

        default:
            break;
        }

        return std::unexpected(errMsg);
    }
    return command;
}

db::Database::Database(const std::string &sql, const std::string &tableName) : tableName(tableName)
{
    if (!(rc = execute(sqlite3_open(sql.c_str(), &db), ParseError::ErrOpenDB))) {
        throw(rc.error());
    }
}

db::Package
db::Database::getPackage(const std::string &subID)
{
    Package res;
    sqlite3_stmt *stmt = nullptr;

    auto select = std::format("SELECT * from {} where submission_id = \'{}\';", tableName, subID);

    // check the correctness of the stmt
    if (!(rc = execute(sqlite3_prepare_v2(db, select.c_str(), -1, &stmt, nullptr), ParseError::ErrProcessStmt))) {
        throw(rc.error() + ":" + select);
    }

    // trying to execute stmt
    auto p = sqlite3_step(stmt);
    rc = execute(p, ParseError::ErrExec);
    if (!rc) {
        throw(rc.error() + ":" + select);
    }
    // check if subID exists
    rc = execute(p, ParseError::KeyDoesNotExist, SQLITE_DONE);
    if (!rc) {
        sqlite3_finalize(stmt);
        return res;
    }

    res = Package(sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2),
                  sqlite3_column_text(stmt, 3), sqlite3_column_text(stmt, 4));

    // check the uniqueness of the subID
    p = sqlite3_step(stmt);

    rc = execute(p, ParseError::ErrExec);
    if (!rc) {
        throw(rc.error());
    }
    rc = execute(p, ParseError::PrimaryKeyNotUnique, SQLITE_ROW);
    if (!rc) {
        throw(rc.error() + ":" + subID);
    }

    sqlite3_finalize(stmt);

    return res;
}

std::vector<std::string>
db::Database::getPairSolutions(const std::string &probID, const std::string &userID, const std::string &status)
{
    sqlite3_stmt *stmt = nullptr;
    std::vector<std::string> solutiosIDs;
    auto select = std::format("SELECT * from {} where problem_id = \'{}\' and user_id = \'{}\' and status != \'{}\';",
                              tableName, probID, userID, status);

    if (!(rc = execute(sqlite3_prepare_v2(db, select.c_str(), -1, &stmt, nullptr), ParseError::ErrProcessStmt))) {
        throw(rc.error() + ":" + select);
    }

    while (true) {
        auto p = sqlite3_step(stmt);
        rc = execute(p, ParseError::ErrExec);
        if (!rc) {
            throw(rc.error() + ":" + select);
        }
        // check if subID exists
        rc = execute(p, ParseError::KeyDoesNotExist, SQLITE_DONE);
        if (!rc) {
            break;
        }
        solutiosIDs.push_back(reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0)));
    }
    sqlite3_finalize(stmt);
    return solutiosIDs;
}

std::vector<db::Package>
db::Database::query(const char *sqlSt)
{
    sqlite3_stmt *stmt = nullptr;
    std::vector<Package> solutiosIDs;

    if (!(rc = execute(sqlite3_prepare_v2(db, sqlSt, -1, &stmt, nullptr), ParseError::ErrProcessStmt))) {
        throw(rc.error() + ":" + sqlSt);
    }

    while (true) {
        auto p = sqlite3_step(stmt);
        rc = execute(p, ParseError::ErrExec);
        if (!rc) {
            throw(rc.error() + ":" + sqlSt);
        }

        rc = execute(p, ParseError::KeyDoesNotExist, SQLITE_DONE);
        if (!rc) {
            break;
        }
        solutiosIDs.push_back(Package(sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1),
                                      sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3),
                                      sqlite3_column_text(stmt, 4)));
    }
    sqlite3_finalize(stmt);
    return solutiosIDs;
}

db::Database::~Database()
{
    if (db) {
        sqlite3_close(db);
    }
}
