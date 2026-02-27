#include "dropboxapi.h"
#include <cpr/cpr.h>
#include <fstream>
#include <vector>

static const std::string access_token = "sl.u.AGXbFkFKdGHn-ltLAAI7L6cgf48ebaW_0rMLdnhYkRf-RW9mZKM0lOHa7z-NaGINUJKYftws1LPCR8umty4z9sUhzoKja3hve6SnH6CbsXU0cZqejC5t6xDL66ihDzhfwCxWmDATIrNibZXvn42k040LkWjv1gs9MpgD5hzq2Swdd_Cvl2uVppK86ewmKs6KCwWke_luZujvp7gGJQHLV-qNjOW9ibUDiwSGIZU_n0e6ONmUAsbf_fhWV6aD3VXd1R_GVlvjwUUcTFeJkEueV2JFsFBLKCuw_Bjts648MOnC3D5lF20SGwyVlasy-ToBSW0B_XE1N0R75La_Jblah0elukcZW1EY6RcK_-dGji3dXwMC0bDQXHyaFFd1ccSi_46KBHxb-wWZB9utSgo8WXu5w0SzT1IuAqVKAaVXySgSPU1KRde77SaOwKXMntD_Iaw_SJ14Aft7ki8Mwn8qM-vhWhZ9RFY1usflS4xjWOKcUm7Bs9g2z-0tqqHeBbdwAO_ib9qHAN0h2UrnsgFQ1eunIgGug2f1pJLQB0waVEMz8bXcQw9hAAN6cNL7OOzAkr9obPNIDYhPA540hmudRMybCruT5hEvB7V5Q_GkYFEtSoab0_WjyghNRFrNKWwacBhYxMyrSfjGWhlc31hjumzi1tWlJG3TPUSt5GCqijKyE1XgxwFRnvCaoOFzed3UA0wHoUKDupbqQhW5x_6moyjb_qVQx_frMTJTPh9EIVySlJTuxF_3ua6xVfUiwtSWCZjGRPamEDZCjHTiJH6JbBQTDzJUFMntiQu8Lc-Oa7rD8whlfY4JfuT4T940jxoX0brVlw_ByVcgRQsR2eSytbJnYhZ3K65JHwarzBjxA9sJV613eElNaMrH3vCvwcWLZ2IOvtOY99H0EMgQV8ueGDqEBV83ViDosflHaKLn1Z2akjEAi9lTbsgd5jfeoIvn4DCVsscJo9WvnJH2wMCG_kL8sCslGoF4tZVZ7-E9_mantWnXgtp2bilH7dney2IuPV2lXX6jwHttPu5KmB9jJQmKlgoZYW26uYXas1SaWLcmZ5MiR8gnFmnmezqoxIc7LANApXtI5IiNKg8E9R7bVnG9dADTM8l9ZqMX6ZjDjUE7zP1nJbxBuJJBPMiF83fOXCW_Xgmb6ZBbw0_cVOFdlleKmbzawipFPe-PN6miXK87YiftHmgyvhqD6JhYxZBQ0JdBcsSqn3Jkp1yACWqZUeBhyOzeV4Z48RXaSrf65SRQG5mCTpIlOwgcfBNlpRvgA2CHUcqv5DYiNv3y-hbSlvgY_APLSWrFYVkUfCBS3I34AajH_GzTSxucQHixjq68Pzbyen2a6a5dmpeYk64SGb8mAQeGjnOrtp3_4sESPhp2MEHdvDV0IW9F6n3hf5iyFdEzzJHHT7yGacwOnewuNJMY3mVYeEwrfpMsb5thTMAs_yQjNMli-9nxoTMsvyGRk-YK_CtlKg3fMyahkm8zRV9ADaxCY3RWKVDgANlsshQ2iA";

static std::vector<char> readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    return std::vector<char>((std::istreambuf_iterator<char>(file)),
                              std::istreambuf_iterator<char>());
}

bool upload(const std::string& localPath, const std::string& remotePath) {
    auto data = readFile(localPath);
    if (data.empty()) return false;   // файл не прочитан

    std::string body(data.begin(), data.end());

    auto response = cpr::Post(
        cpr::Url{"https://content.dropboxapi.com/2/files/upload"},
        cpr::Header{
            {"Authorization", "Bearer " + access_token},
            {"Content-Type", "application/octet-stream"},
            {"Dropbox-API-Arg",
             R"({"path":")" + remotePath +
             R"(","mode":"overwrite","mute":false})"}
        },
        cpr::Body{body}
    );

    return response.status_code == 200;
}

bool download(const std::string& remotePath, const std::string& localPath) {
    auto response = cpr::Post(
        cpr::Url{"https://content.dropboxapi.com/2/files/download"},
        cpr::Header{
            {"Authorization", "Bearer " + access_token},
            {"Dropbox-API-Arg", R"({"path":")" + remotePath + R"("})"},
            {"Content-Type", "application/octet-stream"}
        }
    );

    if (response.status_code != 200) 
        return false;

    std::ofstream out(localPath, std::ios::binary);
    out.write(response.text.data(), response.text.size());
    out.close();

    return true;
}
