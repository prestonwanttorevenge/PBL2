    #include "Invoice.h"
    #include "InvoiceDetail.h"
    #include<iostream>
    #include<string>
    void Invoice::addDetail(const InvoiceDetail& detail){
        details.push_back(detail);
        consultationFee += detail.getSubTotal(); 
        totalAmount = consultationFee + medicineFee; 
    }
    void Invoice::setMedicineFee(double fee){
        medicineFee = fee;
        totalAmount = consultationFee + medicineFee;
    }
    void Invoice::printInvoice() const {
        cout << "--- HOA DON: " << invoiceId << " | BN: " << patientId << " ---" << endl;
        for (const auto& d : details) {
            cout << "DV: " << d.getServiceId() << " | Tien: " << d.getSubTotal() << endl;
        }
        cout << "Phi DV: " << consultationFee << " | Phi Thuoc: " << medicineFee << endl;
        cout << "TONG THANH TOAN: " << totalAmount << endl;
    }