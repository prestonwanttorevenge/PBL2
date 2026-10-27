#ifndef PATIENT_H
#define PATIENT_H
#include "Person.h"

class Patient : public Person {
protected: 
    string medicalRecordId;   
    string BHYT;
public:
    Patient();
    Patient(string id, string name, string dob, string gender, string phone, 
            string recordId, string BH)
        : Person(id, name, dob, gender, phone), 
          medicalRecordId(recordId), BHYT(BH){}
    Patient(Patient &);
    virtual ~Patient() override{}
    virtual string getType() const = 0;
    bool hasBHYT() const {
    return !BHYT.empty() && BHYT != "Không" && BHYT != "Khong" &&
           BHYT != "khong" && BHYT != "No" && BHYT != "no";
}
    string getMedicalRecordId() const;
    string getBHYT() const;

    void setMedicalRecordId(string recordId);
    void setBHYT(string BH);

    void inputData() override;
    void displayData() const override;
    
    void viewMedicalHistory() const;
};

#endif