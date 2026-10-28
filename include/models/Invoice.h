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
    // Gắn hóa đơn với lượt khám để tránh thanh toán trùng lượt.
    string appointmentId;
    double consultationFee; 
    double medicineFee;     
    double totalAmount;     
    vector<InvoiceDetail> details;

public:
    Invoice(string iId = "", string pId = "", double mFee = 0.0, string aId = "")
        : invoiceId(iId), patientId(pId), appointmentId(aId),
          consultationFee(0.0), medicineFee(mFee), totalAmount(mFee) {}
        
    void addDetail(const InvoiceDetail& detail) {
        details.push_back(detail);
        consultationFee += detail.getSubTotal();
        totalAmount = consultationFee + medicineFee;
    }
    
    string getId() const {return invoiceId;}
    string getPatientId() const {return patientId;}
    string getAppointmentId() const { return appointmentId; }
    double getMedicineFee() const {return medicineFee;}
    vector<InvoiceDetail> getDetails() const {return details;}
    double getConsultationFee() const { return consultationFee; }
    double getTotalAmount() const { return totalAmount; }
    void setAppointmentId(const string& id) { appointmentId = id; }
    void setMedicineFee(double fee);
    void printInvoice() const;
    
    ~Invoice(){}
};
#endif