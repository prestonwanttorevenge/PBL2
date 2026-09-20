#include "../../include/models/Outpatient.h"
#include <iostream>
#include <iomanip>

Outpatient::~Outpatient() {}

void Outpatient::inputData() {
    cout << "--- NHAP THONG TIN BENH NHAN NGOAI TRU ---" << endl;
    Patient::inputData();
    cout << "Nhap Ngay hen kham: ";
    getline(cin, this->appointmentDate);
    cout << "Nhap Phong kham chi dinh ";
    getline(cin, this->clinicRoom);
}

void Outpatient::displayData() const {
    Patient::displayData();
    cout << left << setw(15) << appointmentDate
         << left << setw(15) << clinicRoom << " (Ngoai Tru)" << endl;
}

void Outpatient::bookAppointment(){
    cout << "Da dat lich kham vao " << appointmentDate << "tai" << " clinicRoom" << endl;
}