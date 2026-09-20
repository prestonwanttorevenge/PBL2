#ifndef USER_H
#define USER_H
#include <string>
#include <iostream>

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
    User(string id = "-1", string uName = "", string pass = "", 
         string fName = "", string role = "Receptionist", bool active = true)
        : userId(id), username(uName), passwordHash(pass), 
          fullName(fName), roleName(role), isActive(active){}
    ~User(){};
    string getId() const{return userId; }
    string getUsername() const{return username; }
    string getPassword() const{return passwordHash; }
    string getFullName() const{return fullName; }
    string getRole() const{return roleName; }
    bool getIsActive() const{return isActive; }
    void inputData();
    void displayData() const{
        cout << "ID: " << userId << " | User: " << username 
             << " | Quyen: " << roleName << " | Trang thai: " 
             << (isActive ? "Hoat dong" : "Bi khoa") << endl;
    }
    bool authenticate();
};
#endif