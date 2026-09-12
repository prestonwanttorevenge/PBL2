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
        std::cout << "--- HÓA ĐƠN ---" << std::endl;
        std::cout << "Mã hóa đơn: " << invoiceId << std::endl;
        std::cout << "Mã bệnh nhân: " << patientId << std::endl;
        std::cout << "Phí khám: " << consultationFee << std::endl;
        std::cout << "Phí thuốc: " << medicineFee << std::endl;
        std::cout << "Tổng tiền: " << totalAmount << std::endl;
        std::cout << "---------------" << std::endl;
    }
};