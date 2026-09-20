#ifndef INVOICE_H
#define INVOICE_H
#include <string>
#include <vector>
#include <iostream>
#include "InvoiceDetail.h"

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
    void addDetail(const InvoiceDetail& detail) {
        details.push_back(detail);
        consultationFee += detail.getSubTotal();
        totalAmount = consultationFee + medicineFee;
    }
    string getId()const{return invoiceId;}
    string getPatientId()const{return patientId;}
    double getMedicineFee()const {return medicineFee;}
    vector<InvoiceDetail> getDetails()const {return details;}
    void setMedicineFee(double fee);
    void printInvoice() const ;
    string getId(){return invoiceId;}
    ~Invoice(){}
};
#endif