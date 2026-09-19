#include <iostream>
#include <string>
#include <InvoiceDetail.h>
#ifndef INVOICE_H
#define INVOICE_H
class Invoice {
private:
    std::string invoiceId;
    std::string patientId;
    double consultationFee;
    double medicineFee;
    double totalAmount;
    std::vector<InvoiceDetail> details;
public:
    Invoice(std::string invId = "ERROR", std::string patId = "ERROR", double consultFee = 0.0, double medFee = 0.0) {
        invoiceId = invId;
        patientId = patId;
        consultationFee = consultFee;
        medicineFee = medFee;
        totalAmount = 0.0; 
    }
    Invoice(const Invoice &);
    void calculateTotal(){
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
    ~Invoice(){}
};
#endif