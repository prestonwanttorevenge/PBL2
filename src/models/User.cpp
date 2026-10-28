#include "../../include/models/User.h"
#include <iostream>

using namespace std;

void User::inputData() {
    cout << "Nhap ID nguoi dung: ";
    cin >> userId;
    cin.ignore();
    
    // Bắt nhập lại cho đến khi username đúng quy tắc dùng chung.
    do {
        cout << "Nhap Username (3-32 ky tu khong dau, chi dung chu, so, ., _ hoac -): ";
        getline(cin, username);
        if (!isValidUsername(username)) {
            cout << "Ten dang nhap khong hop le. Vui long nhap lai.\n";
        }
    } while (!isValidUsername(username));
    
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