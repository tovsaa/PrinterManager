#ifndef SQLITE_HELPER_H
#define SQLITE_HELPER_H

#include <string>
#include <vector>
#include <sqlite3.h>

struct PrintJob {
    std::string user;
    std::string printer;
    int pages;
};

bool addRecord(sqlite3* db, const std::string& user, const std::string& printer, int pages);
bool readAllRecords(sqlite3* db, std::vector<PrintJob>& jobs);
bool deleteAllRecords(sqlite3* db);
bool openDatabase(const std::string& filename, sqlite3** db);
void closeDatabase(sqlite3* db);

#endif
