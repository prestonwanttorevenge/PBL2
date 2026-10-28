#include "services/AuthService.h"

AuthService::AuthService(Database& database) : db(database), currentUser(nullptr) {}
AuthService::~AuthService() {}

bool AuthService::login(const std::string& inputUsername, const std::string& inputPassword) {
    // Xóa phiên cũ trước khi thử đăng nhập để tránh giữ nhầm quyền.
    currentUser = nullptr;
    User* user = db.loginFromQT(inputUsername, inputPassword);

    if (user != nullptr && user->getIsActive()) {
        currentUser = user;
        return true;
    }
    return false;
}

void AuthService::logout() {
    currentUser = nullptr;
}

bool AuthService::changePassword(const std::string& oldPassword, const std::string& newPassword) {
    if (!currentUser) return false;
    if (currentUser->authenticate(oldPassword)) {
        currentUser->setPassword(newPassword);
        return true;
    }
    return false;
}

User* AuthService::getCurrentUser() const {
    return currentUser;
}

bool AuthService::isLoggedIn() const {
    return currentUser != nullptr;
}

std::string AuthService::getCurrentUserRole() const {
    if (currentUser) {
        return currentUser->getRole();
    }
    return "";
}