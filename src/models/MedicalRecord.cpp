#include "../../include/models/MedicalRecord.h"

MedicalRecord::MedicalRecord(std::string rId, std:: string aId){
    this->recordId = rId;
    this->appointmentId = aId;
    this->symptoms = "";
    this->diagnosis = "";
    this->notes = "";
}

MedicalRecord::~MedicalRecord() {}

std::string MedicalRecord::getRecordId() const {return recordId; }
std::string MedicalRecord::getAppointmentId() const {return appointmentId; }
std::string MedicalRecord::getSymptoms() const {return symptoms; }
std::string MedicalRecord::getDiagnosis() const {return diagnosis; }
std::string MedicalRecord::getNotes() const {return notes; }
std::vector<PrescriptionDetail> MedicalRecord::getPrescriptions() const {return prescriptions; }

void MedicalRecord::inputData(){
    std::cout << "Nhap Ma benh an: " ;
    std::getline(std::cin, recordId);
    std::cout << "Nhap Trieu chung: ";
    std::getline(std::cin, symptoms);
    std::cout << "Nhap Chan doan: ";
    std::getline(std::cin, diagnosis);
    std::cout << "Nhap Ghi chu: ";
    std::getline(std::cin, notes);
}

void MedicalRecord::displayData() const {
    std::cout << "--- BENH AN ---" << std::endl;
    std::cout << "ID: " << recordId << " | Lich hen: " << appointmentId << std::endl;
    std::cout << "Trieu chung: " << symptoms << std::endl;
    std::cout << "Chan doan: " << diagnosis << std::endl;
    std::cout << "So loai thuoc da ke: " << prescriptions.size() << std::endl;
}

void MedicalRecord::addMedicine(std::string mId, std::string dose, int qty){
    PrescriptionDetail pd = {mId, dose, qty};
    prescriptions.push_back(pd);
}