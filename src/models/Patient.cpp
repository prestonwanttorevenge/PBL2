#include "../../include/models/Patient.h"
#include <iostream>
#include <iomanip>

Patient::Patient() : Person(){
    this->medicalRecordId = "";
    this->insuranceDiscount = 0.0;
}
string Patient::getMedicalRecordId() const {return medicalRecordId; }
double Patient::getInsuranceDiscount() const {return insuranceDiscount; }

void Patient::setMedicalRecordId(string recordId) {this->medicalRecordId = recordId; }
void Patient::setInsuranceDiscount(double discount) {this->insuranceDiscount = discount; }

void Patient::inputData(){
    Person::inputData();
    cout << "Nhap Ma benh an: ";
    getline(cin, this->medicalRecordId);
    cout << "Nhap chiet khau giam gia bao hiem (VD: 0.8): ";
    cin >> this->insuranceDiscount;
    cin.ignore();
}

void Patient::displayData() const {
    Person::displayData();
    cout << left << setw(15) << medicalRecordId 
         << left << setw(10) << insuranceDiscount;
}

void Patient::viewMedicalHistory() const {
    cout << "Xem lich su benh an cua ID: " << medicalRecordId << endl;
}