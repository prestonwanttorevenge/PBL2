#ifndef INVOICE_H
#define INVOICE_H
#include <iostream>
#include <string>

class Invoice {
private:
    std::string invoiceId;
    std::string patientId;
    double consultationFee;
    double medicineFee;
    double totalAmount;

public:
    Invoice(std::string invId = "ERROR", std::string patId = "ERROR", double consultFee = 0.0, double medFee = 0.0);
    ~Invoice();

    std::string getInvoiceId() const;
    std::string getPatientId() const;
    double getConsultationFee() const;
    double getMedicineFee() const;
    double getTotalAmount() const;

    void calculateTotal();
    void printInvoice() const;
};
#endif