#include "../../include/models/Medicine.h"
#include <iostream>
using namespace std;

Medicine::Medicine(string m_id, string m_name, double m_price, string m_dose){
    this->medicineId = m_id;
    this->medicineName = m_name;
    this->unitPrice = m_price;
    this->dosage = m_dose;
}

Medicine::~Medicine(){}

string Medicine::getMedicineId() const {return medicineId; }
string Medicine::getMedicineName() const {return medicineName; }
double Medicine::getUnitPrice() const {return unitPrice; }
string Medicine::getDosage() const {return dosage; }

void Medicine::setUnitPrice(double newPrice){
    this->unitPrice = newPrice;
}

void Medicine::setDosage(string newDosage){
    this->dosage = newDosage;
}

void Medicine::displayData() const{
    cout << "Thuoc ID: " << medicineId
         << " | Ten " << medicineName
         << " | Gia " << unitPrice
         << " | Lieu dung " << dosage << endl;
}

void Medicine::inputData(){
    cout << "Nhap Ma thuoc: ";
    cin >> medicineId;
    cin.ignore();
    cout << "Nhap Ten thuoc: ";
    getline(cin, medicineName);
    cout << "Nhap Don gia: ";
    cin >> unitPrice;
    cin.ignore();
    cout << "Nhap Lieu dung: ";
    getline(cin, dosage);
}