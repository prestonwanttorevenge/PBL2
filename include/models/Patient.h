#ifndef PATIENT_H
#define PATIENT_H
#include "Person.h"

class Patient : public Person {
protected: 
    string medicalRecordId;   
    double insuranceDiscount;
public:
    Patient();
    Patient(string id, string name, string dob, string gender, string phone, 
            string recordId, double discount)
        : Person(id, name, dob, gender, phone), 
          medicalRecordId(recordId), insuranceDiscount(discount){}
    Patient(Patient &);
    virtual ~Patient() override{}
    virtual string getType() const = 0;
    string getMedicalRecordId() const;
    double getInsuranceDiscount() const;

    void setMedicalRecordId(string recordId);
    void setInsuranceDiscount(double discount);

    void inputData() override;
    void displayData() const override;
    
    void viewMedicalHistory() const;
};

#endif