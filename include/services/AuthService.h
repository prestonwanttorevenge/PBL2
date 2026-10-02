#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <string>
#include "../models/User.h"
#include "Database.h"

class AuthService {
private:
    Database& db;
    User* currentUser;
public:
    AuthService(Database& database);
    ~AuthService();

    bool login(const std::string& inputUsername, const std::string& inputPassword);
    void logout();
    bool changePassword(const std::string& oldPassword, const std::string& newPassword);

    User* getCurrentUser() const;
    bool isLoggedIn() const;
    std::string getCurrentUserRole() const;
};

#endif