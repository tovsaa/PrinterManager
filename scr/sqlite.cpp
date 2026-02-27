#include "sqlite.h"
#include <windows.h>


bool addRecord(sqlite3* db, const std::string& user, const std::string& printer, int pages) {
    char* errMsg = nullptr;
    std::string sql = "INSERT INTO PrintJobs (User, Printer, Pages) VALUES ('"
                      + user + "', '" + printer + "', " + std::to_string(pages) + ");";
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool readAllRecords(sqlite3* db, std::vector<PrintJob>& jobs) {
    char* errMsg = nullptr;
    const char* sql = "SELECT User, Printer, Pages FROM PrintJobs;";

    auto callback = [](void* data, int argc, char** argv, char** /*colName*/) -> int {
        std::vector<PrintJob>* jobsPtr = static_cast<std::vector<PrintJob>*>(data);
        if (argc == 3) {
            PrintJob job;
            job.user = argv[0] ? argv[0] : "";
            job.printer = argv[1] ? argv[1] : "";
            job.pages = argv[2] ? std::stoi(argv[2]) : 0;
            jobsPtr->push_back(job);
        }
        return 0;
    };

    int rc = sqlite3_exec(db, sql, callback, &jobs, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool deleteAllRecords(sqlite3* db) {
    char* errMsg = nullptr;
    const char* sql = "DELETE FROM PrintJobs;";
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool openDatabase(const std::string& filename, sqlite3** db) {
    int rc = sqlite3_open(filename.c_str(), db);
    return rc == SQLITE_OK;
}

void closeDatabase(sqlite3* db) {
    if (db) {
        sqlite3_close(db);
    }
}
