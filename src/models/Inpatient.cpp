#include "../../include/models/Inpatient.h"
#include <iostream>
#include <iomanip>
using namespace std; 
Inpatient::Inpatient() : Patient("") {
    this->admissionDate = "";
    this->dischargeDate = "";
    this->roomNumber = "";
    this->bedNumber = "";
}

Inpatient::~Inpatient() {}

void Inpatient::inputData() {
    cout << "--- NHAP THONG TIN BENH NHAN NOI TRU ---" << endl;
    Patient::inputData(); 
    
    cout << "Nhap Ngay nhap vien: ";
    getline(cin, this->admissionDate);
    
    cout << "Nhap So phong: ";
    getline(cin, this->roomNumber);
    
    cout << "Nhap So giuong: ";
    getline(cin, this->bedNumber);
}

void Inpatient::displayData() const {
    Patient::displayData();
    cout << left << setw(15) << admissionDate 
         << left << setw(10) << roomNumber 
         << left << setw(10) << bedNumber << " (Noi Tru)" << endl;
}

void Inpatient::assignRoom() {
    cout << "Da xep phong " << roomNumber << ", giuong " << bedNumber << endl;
}

void Inpatient::processDischarge() {
    cout << "Nhap ngay xuat vien: ";
    getline(cin, this->dischargeDate);
    cout << "Benh nhan da xuat vien vao " << dischargeDate << endl;
}