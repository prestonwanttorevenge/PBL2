#include "../../include/models/Invoice.h"
#include <iostream>

Invoice::Invoice(std::string invId, std::string patId, double consultFee, double medFee){
    this->invoiceId = invId;
    this->patientId = patId;
    this->consultationFee = consultFee;
    this->medicineFee = medFee;
    this->totalAmount = 0.0;
}

Invoice::~Invoice() {}

std::string Invoice::getInvoiceId() const {return invoiceId; }
std::string Invoice::getPatientId() const {return patientId; }
double Invoice::getConsultationFee() const {return consultationFee; }
double Invoice::getMedicineFee() const {return medicineFee; }

void Invoice::calculateTotal(){
    totalAmount = consultationFee + medicineFee;
}

void Invoice::printInvoice() const{
    std::cout << "--- HOA DON ---" << std::endl;
    std::cout << "Ma hoa don: " << invoiceId << std::endl;
    std::cout << "Ma benh nhan: " << patientId << std::endl;
    std::cout << "Phi kham: " << consultationFee << std::endl;
    std::cout << "Phi thuoc: " << medicineFee << std::endl;
    std::cout << "Tong tien: " << totalAmount << std::endl;
    std::cout << "---------------" << std::endl;
}