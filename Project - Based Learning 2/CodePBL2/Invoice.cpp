#include <iostream>
#include <string>
#include <windows.h>
class Invoice {
private:
    std::string invoiceId;
    std::string patientId;
    double consultationFee;
    double medicineFee;
    double totalAmount;

public:
    Invoice(std::string invId, std::string patId, double consultFee, double medFee) {
        invoiceId = invId;
        patientId = patId;
        consultationFee = consultFee;
        medicineFee = medFee;
        totalAmount = 0.0; 
    }

    void calculateTotal() {
        totalAmount = consultationFee + medicineFee;
    }

    void printInvoice() {
        std::cout << "--- HOA DON ---" << std::endl;
        std::cout << "Ma hoa don: " << invoiceId << std::endl;
        std::cout << "Ma benh nhan: " << patientId << std::endl;
        std::cout << "Phi kham: " << consultationFee << std::endl;
        std::cout << "Phi thuoc: " << medicineFee << std::endl;
        std::cout << "Tong tien: " << totalAmount << std::endl;
        std::cout << "---------------" << std::endl;
    }
};