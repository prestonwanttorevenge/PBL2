#include "../../include/models/Patient.h"
#include <iostream>
#include <iomanip>

Patient::Patient() : Person(){
    this->medicalRecordId = "";
    this->BHYT = "";
}
string Patient::getMedicalRecordId() const {return medicalRecordId; }
string Patient::getBHYT() const {return BHYT; }

void Patient::setMedicalRecordId(string recordId) {this->medicalRecordId = recordId; }
void Patient::setBHYT(string BH) {this->BHYT = BH; }

void Patient::inputData(){
    Person::inputData();
    cout << "Nhap Ma benh an: ";
    getline(cin, this->medicalRecordId);
    cin.ignore();
    cout << "Nhap chiet khau giam gia bao hiem (VD: 0.8): ";
      getline(cin, this->BHYT);
    cin.ignore();
}

void Patient::displayData() const {
    Person::displayData();
    cout << left << setw(15) << medicalRecordId 
         << left << setw(10) << BHYT;
}

void Patient::viewMedicalHistory() const {
    cout << "Xem lich su benh an cua ID: " << medicalRecordId << endl;
}