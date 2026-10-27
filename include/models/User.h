#ifndef USER_H
#define USER_H
#include <string>
#include <iostream>
#include "bcrypt.h"

using namespace std;

class User{
private:
    string userId;
    string username;
    string passwordHash;
    string fullName;
    string roleName;   
    bool isActive; 

public:
    static bool isValidUsername(const string& value) {
        if (value.size() < 3 || value.size() > 32) return false;
        for (unsigned char character : value) {
            if (!((character >= 'A' && character <= 'Z') ||
                  (character >= 'a' && character <= 'z') ||
                  (character >= '0' && character <= '9') ||
                  character == '_' || character == '.' || character == '-')) {
                return false;
            }
        }
        return true;
    }

    User(string id = "", string user = "", string pass = "", string fname = "", string r = "", bool active = false)
        : userId(id), username(user), passwordHash(pass), fullName(fname), roleName(r), isActive(active) {}
    ~User(){};
    string getId() const{return userId; }
    string getUsername() const{return username; }
    string getPassword() const{return passwordHash; }
    string getFullName() const{return fullName; }
    string getRole() const{return roleName; }
    bool getIsActive() const{return isActive; }
    void setIsActive(bool active) { isActive = active; }
    bool authenticate(const string& inputPassword) const {
        return bcrypt::validatePassword(inputPassword, this->passwordHash);
    }
    void setPassword(const string& newPlainPassword) {
        this->passwordHash = bcrypt::generateHash(newPlainPassword);
    }
    void inputData();
    void displayData() const{
        cout << "ID: " << userId << " | User: " << username 
             << " | Quyen: " << roleName << " | Trang thai: " 
             << (isActive ? "Hoat dong" : "Bi khoa") << endl;
    }
    bool authenticate();
};
#endif