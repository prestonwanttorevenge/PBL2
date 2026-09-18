#include "../../include/models/Doctor.h"
#include <iostream>
#include <iomanip>
#include <limits>

Doctor::Doctor() : Person() {
    this->specialty = "";
    this->yearOfExperience = 0;
    this->workSchedule = "";
}

Doctor::Doctor(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears, string schedule)
 : Person(id, fullName, dob, gender, phone){
    this->specialty = specialty;
    this->yearOfExperience = expYears;
    this->workSchedule = schedule;
 }
Doctor::~Doctor(){
}
//Getters
string Doctor::getSpecialty() const {return specialty; }
int Doctor::getYearsOfExperience() const {return yearOfExperience; }
string Doctor::getWorkSchedule() const {return workSchedule; }
//Setters
void Doctor::setSpecialty(string specialty) {this->specialty = specialty; }
void Doctor::setYearsOfExperience(int expYears) {this->yearOfExperience = expYears; }
void Doctor::setWorkSchedule(string schedule) {this->workSchedule = schedule; }
//Processing 
void Doctor::inputData(){
    Person::inputData();
    cout << "Nhap Chuyen khoa: ";
    getline(cin, this->specialty);
    cout << "Nhap So nam kinh nghiem: ";
    cin >> this->yearOfExperience;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Nhap Lich truc (VD: T2 - T4 - T6 Sang): ";
    getline(cin, this->workSchedule);
}

void Doctor::displayData() const {
    Person::displayData();
    cout << left << setw(20) << specialty
         << left << setw(10) << yearOfExperience
         << left << setw(30) << workSchedule << endl;
}

void Doctor::viewSchedule() const {
    cout << "--- LICH TRUC CUA BAC SI" << fullName << " ---" << endl;
    cout << "ID: " << id << " | Chuyen khoa: " << specialty << endl;
    cout << "Lich truc hien tai: " << workSchedule << endl;
    cout << "------------------------------------------------" << endl;
}

