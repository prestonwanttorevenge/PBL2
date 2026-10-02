#include "../../include/models/User.h"
#include <iostream>

using namespace std;

void User::inputData() {
    cout << "Nhap ID nguoi dung: ";
    cin >> userId;
    cin.ignore();
    
    cout << "Nhap Username: ";
    getline(cin, username);
    
    cout << "Nhap Mat khau: ";
    string plainPass;
    getline(cin, plainPass);
    setPassword(plainPass); 
    
    cout << "Nhap Ho va Ten: ";
    getline(cin, fullName);
    
    cout << "Nhap Vai tro (Admin/Doctor/Receptionist): ";
    getline(cin, roleName);
    
    isActive = true;
}

bool User::authenticate() {
    string inputPass;
    cout << "Nhap mat khau xac thực cho tai khoan [" << username << "]: ";
    cin >> inputPass;
    return authenticate(inputPass);
}