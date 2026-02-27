#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <string>
#include "sqlite.h"

bool addPrintJob(std::string code, const std::string& printer, int pages, std::string localDb);
bool readPrintJobs(std::string code, std::vector<PrintJob>& jobs, std::string localDb);

#endif
