#ifndef PATIENT_H
#define PATIENT_H
#include "Person.h"

class Patient : public Person {
protected: 
    string medicalRecordId;   
    double insuranceDiscount;

public:
    Patient();
    Patient(string id, string fullName, string dob, string gender, string phone, 
            string recordId, double discount);
    
    virtual ~Patient() override;

    string getMedicalRecordId() const;
    double getInsuranceDiscount() const;

    void setMedicalRecordId(string recordId);
    void setInsuranceDiscount(double discount);

    void inputData() override;
    void displayData() const override;
    
    void viewMedicalHistory() const;
};

#endif