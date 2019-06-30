#pragma once

#include <sqlite3/database.h>

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_set>

class LoginManager
{
public:
    struct User
    {
        User(uint64_t id = 0, const std::string& handle = "")
            : id(id), handle(handle)
        {}
        const uint64_t id;
        const std::string handle;
    };

    static const uint64_t NO_USER = 0;
    LoginManager(sqlite3db::Database& db) : db(db)
    {
        db.exec("CREATE TABLE IF NOT EXISTS bbsuser (sha TEXT, handle TEXT)");
    }
    uint64_t verify_user(const std::string& handle,
                         const std::string& password);
    bool change_password(const std::string& handle, const std::string& newp,
                         const std::string& oldp = "");
    uint64_t login_user(const std::string& handle, const std::string& password);
    void logout_user(uint64_t id);
    std::vector<std::string> list_users();
    std::vector<std::string> list_logged_in();

    uint64_t get_id(const std::string& handle);
    std::string get(uint64_t id);
    uint64_t add_user(const std::string& handle, const std::string& password);

private:
    sqlite3db::Database& db;
    std::unordered_set<uint64_t> logged_in;
    std::mutex lock;
};

