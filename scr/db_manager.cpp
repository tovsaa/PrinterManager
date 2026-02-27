#include "db_manager.h"
#include "dropboxapi.h"
#include "userip.h"


bool addPrintJob(const std::string& printer, int pages, std::string localDb) {
    std::string localIP = GetLocalIP();

    const std::string remoteDb = "/printjobs.db";

    bool downloaded = download(remoteDb, localDb);

    sqlite3* db = nullptr;
    if (!openDatabase(localDb, &db))
        return false;

    if (!addRecord(db, localIP, printer, pages)) {
        closeDatabase(db);
        return false;
    }

    closeDatabase(db);

    if (!upload(localDb, remoteDb))
        return false;

    return true;
}


bool readPrintJobs(std::vector<PrintJob>& jobs, std::string localDb) {
    const std::string remoteDb = "/printjobs.db";

    bool downloaded = download(remoteDb, localDb);
    if (!downloaded)
        return false;

    sqlite3* db = nullptr;
    if (!openDatabase(localDb, &db))
        return false;

    if (!readAllRecords(db, jobs)) {
        closeDatabase(db);
        return false;
    }

    closeDatabase(db);
    return true;
}
