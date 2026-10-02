#include "../../include/services/AuthService.h"
#include <iostream>
using namespace std;

AuthService::AuthService(Database& database) : db(database), currentUser(nullptr) {}
AuthService::~AuthService() {}

bool AuthService::login(const string& inputUsername, const string& inputPassword){
    for (User& u : db.getUsers()){
        if (u.getUsername() == inputUsername){
            if (!u.getIsActive()){
                cout << "[Thong bao] Tai khoan nay da bi khoa boi Admin!\n";
                return false;
            }
            if (u.authenticate(inputPassword)){
                currentUser = &u; 
                cout << "=> Dang nhap thanh cong! Chao mung, " << u.getFullName() << "\n";
                return true;
            } else {
                cout << "[Loi] Mat khau duoc nhap vao chua dung!\n";
                return false;
            }
        }
    }
    cout << "[Loi] Khong the tim thay tai khoan " << inputUsername << "!\n";
    return false;
}

void AuthService::logout(){
    if (currentUser != nullptr){
        cout << "=> Tam biet " << currentUser->getFullName() << ", ban da dang xuat thanh cong. \n";
        currentUser = nullptr;
    }
}

bool AuthService::changePassword(const string& oldPassword, const string& newPassword){
    if (!isLoggedIn()){
        cout << "[Loi] Ban phai dang nhap moi duoc doi mat khau!\n";
        return false;
    }
    if (currentUser->authenticate(oldPassword)){
        currentUser->setPassword(newPassword);
        db.saveAllData();
        cout << "=> Doi mat khau thanh cong!\n";
        return true;
    }
    
    cout << "[Loi] Mat khau hien tai khong chinh xac!\n";
    return false;
}

User* AuthService::getCurrentUser() const {return currentUser; }
bool AuthService::isLoggedIn() const {return currentUser != nullptr; }
string AuthService::getCurrentUserRole() const {
    if (isLoggedIn()){
        return currentUser->getRole();
    }
    return "";
}
