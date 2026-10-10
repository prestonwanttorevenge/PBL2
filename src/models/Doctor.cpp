#include "../../include/models/Doctor.h"
#include <iostream>
#include <iomanip>
#include <limits>

Doctor::Doctor() : Person() {
    this->specialty = "";
    this->yearOfExperience = 0;
}

Doctor::Doctor(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears)
 : Person(id, fullName, dob, gender, phone){
    this->specialty = specialty;
    this->yearOfExperience = expYears;
}
Doctor::~Doctor(){
}
//Getters
string Doctor::getSpecialty() const {return specialty; }
int Doctor::getYearsOfExperience() const {return yearOfExperience; }
const vector<WorkShift>& Doctor::getSchedule() const {
        return workSchedule;
}
//Setters
void Doctor::setSpecialty(string specialty) {this->specialty = specialty; }
void Doctor::setYearsOfExperience(int expYears) {this->yearOfExperience = expYears; }
void Doctor::setWorkSchedule(string day, string time) {workSchedule.push_back({day, time}); }
//Processing 
void Doctor::inputData(){
    Person::inputData();
    cout << "Nhap Chuyen khoa: ";
    getline(cin, this->specialty);
    cout << "Nhap So nam kinh nghiem: ";
    cin >> this->yearOfExperience;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Nhap Lich truc (VD: T2 - Sang): ";
    string day ,time ;
    getline(cin, day);
    getline(cin, time);
    setWorkSchedule(day, time);
}

void Doctor::displayData() const {
        cout << "ID: " << getId() << " | Ten: " << getFullName() 
             << " | Chuyen khoa: " << specialty 
             << " | Kinh nghiem: " << yearOfExperience << " nam"
             << " | Lich lam: ";
        if (workSchedule.empty()) {
            cout << "Chua xep lich";
        } else {
            for (size_t i = 0; i < workSchedule.size(); i++) {
                cout << workSchedule[i].dayOfWeek << " (" << workSchedule[i].timeSlot << ")";
                if (i < workSchedule.size() - 1) {
                    cout << ", ";
                }
            }
        }
        cout << endl;
    }

void Doctor::viewSchedule() const {
    cout << "--- LICH TRUC CUA BAC SI" << getFullName() << " ---" << endl;
    cout << "ID: " << getId() << " | Chuyen khoa: " << specialty << endl;
    cout << "Lich truc hien tai: ";
    if (workSchedule.empty()) {
            cout << "Chua xep lich";
        } else {
            for (size_t i = 0; i < workSchedule.size(); i++) {
                cout << workSchedule[i].dayOfWeek << " (" << workSchedule[i].timeSlot << ")";
                if (i < workSchedule.size() - 1) {
                    cout << ", ";
                }
            }
        }
        cout << endl;
}

