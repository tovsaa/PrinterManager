#include "dropboxapi.h"
#include <cpr/cpr.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <string>

namespace {
const std::string APP_KEY    = "9so3tgijjtzer5l";
const std::string APP_SECRET = "05yo7xrrdtl5fgg";

std::string ExtractJsonValue(const std::string& json, const std::string& key)
{
    std::string pattern = "\"" + key + "\":";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";

    pos += pattern.size();

    while (pos < json.size() && (json[pos] == ' ')) pos++;

    if (json[pos] == '"') {
        pos++;
        auto end = json.find('"', pos);
        return json.substr(pos, end - pos);
    } else {
        auto end = json.find_first_of(",}", pos);
        return json.substr(pos, end - pos);
    }
}
}

void DropboxAuth::AuthorizeInteractive(std::string code)
{
    auto r = cpr::Post(
        cpr::Url{"https://api.dropboxapi.com/oauth2/token"},
        cpr::Payload{
            {"code", code},
            {"grant_type", "authorization_code"},
            {"client_id", APP_KEY},
            {"client_secret", APP_SECRET}
        }
        );

    access_token  = ExtractJsonValue(r.text, "access_token");
    refresh_token = ExtractJsonValue(r.text, "refresh_token");

    int expires = std::stoi(ExtractJsonValue(r.text, "expires_in"));
    expires_at = std::chrono::system_clock::now() +
                 std::chrono::seconds(expires);
}

std::string DropboxAuth::GetAccessToken()
{
    if (std::chrono::system_clock::now() >= expires_at) {
        Refresh();
    }
    return access_token;
}

void DropboxAuth::Refresh()
{
    auto r = cpr::Post(
        cpr::Url{"https://api.dropboxapi.com/oauth2/token"},
        cpr::Payload{
            {"refresh_token", refresh_token},
            {"grant_type", "refresh_token"},
            {"client_id", APP_KEY},
            {"client_secret", APP_SECRET}
        }
        );

    access_token = ExtractJsonValue(r.text, "access_token");

    int expires = std::stoi(ExtractJsonValue(r.text, "expires_in"));
    expires_at = std::chrono::system_clock::now() +
                 std::chrono::seconds(expires);
}


std::vector<char> ReadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(file), {} };
}

bool Upload(DropboxAuth& auth,
            const std::string& localPath,
            const std::string& remotePath)
{
    auto data = ReadFile(localPath);
    if (data.empty()) return false;

    std::string body(data.begin(), data.end());

    auto r = cpr::Post(
        cpr::Url{"https://content.dropboxapi.com/2/files/upload"},
        cpr::Header{
            {"Authorization", "Bearer " + auth.GetAccessToken()},
            {"Content-Type", "application/octet-stream"},
            {"Dropbox-API-Arg",
             R"({"path":")" + remotePath +
                 R"(","mode":"overwrite"})"}
        },
        cpr::Body{body}
        );

    return r.status_code == 200;
}

bool Download(DropboxAuth& auth,
              const std::string& remotePath,
              const std::string& localPath)
{
    auto r = cpr::Post(
        cpr::Url{"https://content.dropboxapi.com/2/files/download"},
        cpr::Header{
            {"Authorization", "Bearer " + auth.GetAccessToken()},
            {"Dropbox-API-Arg", R"({"path":")" + remotePath + R"("})"},
            {"Content-Type", "text/plain"}   // обязательно для корректной работы
        }
        );

    if (r.status_code != 200) {
        return false;
    }

    std::ofstream out(localPath, std::ios::binary);
    out.write(r.text.data(), r.text.size());
    return true;
}
