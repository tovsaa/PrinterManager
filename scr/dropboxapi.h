#ifndef DROPBOXAPI_H
#define DROPBOXAPI_H

#include <string>

bool upload(const std::string& localPath, const std::string& remotePath);
bool download(const std::string& remotePath, const std::string& localPath);

#endif
