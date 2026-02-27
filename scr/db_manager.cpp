#include <string>
#include "dropboxapi.h"
#include "sqlite.h"
#include "userip.h"

bool addPrintJob(std::string code, const std::string& printer, int pages, std::string localDb) {
    DropboxAuth auth;
    auth.AuthorizeInteractive(code);

    std::string localIP = GetLocalIP();

    const std::string remoteDb = "/printjobs.db";

    bool downloaded = Download(auth, remoteDb, localDb);

    sqlite3* db = nullptr;
    if (!openDatabase(localDb, &db))
        return false;

    if (!addRecord(db, localIP, printer, pages)) {
        closeDatabase(db);
        return false;
    }

    closeDatabase(db);

    if (!Upload(auth, localDb, remoteDb))
        return false;

    return true;
}


bool readPrintJobs(std::string code, std::vector<PrintJob>& jobs, std::string localDb) {
    DropboxAuth auth;
    auth.AuthorizeInteractive(code);

    const std::string remoteDb = "/printjobs.db";

    bool downloaded = Download(auth, remoteDb, localDb);
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
