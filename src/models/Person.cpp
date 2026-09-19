#include "../../include/models/Person.h"
#include <iomanip>
Person::Person() {
    this->id = "";
    this->fullName = "";
    this->dateOfBirth = "";
    this->gender = "";
    this->phoneNumber = "";
}
Person::Person(string id, string fullName, string dob, string gender, string phone) {
    this->id = id;
    this->fullName = fullName;
    this->dateOfBirth = dob;
    this->gender = gender;
    this->phoneNumber = phone;
}
Person::~Person() {
}
string Person::getId() const { return id; }
string Person::getFullName() const { return fullName; }
string Person::getDateOfBirth() const { return dateOfBirth; }
string Person::getGender() const { return gender; }
string Person::getPhoneNumber() const { return phoneNumber; }
void Person::setId(string id) { this->id = id; }
void Person::setFullName(string name) { this->fullName = name; }
void Person::setDateOfBirth(string dob) { this->dateOfBirth = dob; }
void Person::setGender(string gender) { this->gender = gender; }
void Person::setPhoneNumber(string phone) { this->phoneNumber = phone; }
void Person::inputData() {
    cout << "Nhap ID: ";
    getline(cin, this->id);
    
    cout << "Nhap Ho va Ten: ";
    getline(cin, this->fullName);
    
    cout << "Nhap Ngay Sinh (DD/MM/YYYY): ";
    getline(cin, this->dateOfBirth);
    
    cout << "Nhap Gioi Tinh: ";
    getline(cin, this->gender);
    
    cout << "Nhap So Dien Thoai: ";
    getline(cin, this->phoneNumber);
}
void Person::displayData() const {
    cout << left << setw(10) << id 
         << left << setw(25) << fullName 
         << left << setw(15) << dateOfBirth 
         << left << setw(10) << gender 
         << left << setw(15) << phoneNumber;
}