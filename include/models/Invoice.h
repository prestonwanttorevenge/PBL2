#include <iostream>
#include <string>
#include <InvoiceDetail.h>
#include <vector>
#ifndef INVOICE_H
#define INVOICE_H
using namespace std;

class Invoice {
private:
    string invoiceId;
    string patientId;
    double consultationFee; 
    double medicineFee;     
    double totalAmount;     
    vector<InvoiceDetail> details;

public:
    Invoice(string iId = "", string pId = "", double mFee = 0.0)
        : invoiceId(iId), patientId(pId), consultationFee(0.0), medicineFee(mFee), totalAmount(mFee) {}
    ~Invoice(){}
    void addDetail(const InvoiceDetail& detail);
    string getId(){return invoiceId;}
    void setMedicineFee(double fee);
    void printInvoice() const ;
};
#endif