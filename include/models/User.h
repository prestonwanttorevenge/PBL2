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
    User(User&);
    ~User(){};
    string getUsername() const{return username; }
    string getPassword() const{return passwordHash; }
    string getRole() const{return roleName; }
    void inputData();
    void displayData() const{
        cout << "ID: " << userId << " | User: " << username 
             << " | Quyen: " << roleName << " | Trang thai: " 
             << (isActive ? "Hoat dong" : "Bi khoa") << endl;
    }
    bool authenticate();
};
#endif