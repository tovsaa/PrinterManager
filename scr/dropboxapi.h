#ifndef DROPBOXAUTH_H
#define DROPBOXAUTH_H

#include <string>
#include <chrono>
#include <vector>

// Класс для аутентификации в Dropbox через OAuth 2.0
class DropboxAuth {
public:
    // Выполнить интерактивную авторизацию, используя полученный код
    void AuthorizeInteractive(std::string code);

    // Получить действующий access token (при необходимости выполняет refresh)
    std::string GetAccessToken();

private:
    // Обновить access token с помощью refresh_token
    void Refresh();

    std::string access_token;
    std::string refresh_token;
    std::chrono::system_clock::time_point expires_at;
};

// Прочитать содержимое локального файла в вектор байт
std::vector<char> ReadFile(const std::string& path);

// Загрузить файл на Dropbox
bool Upload(DropboxAuth& auth,
            const std::string& localPath,
            const std::string& remotePath);

// Скачать файл с Dropbox
bool Download(DropboxAuth& auth,
              const std::string& remotePath,
              const std::string& localPath);

#endif // DROPBOXAUTH_H
