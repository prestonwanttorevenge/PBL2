#ifndef INPATIENT_H
#define INPATIENT_H
#include "Patient.h"

class Inpatient : public Patient {
    private:
        string admissionDate;
        string dischargeDate;
        string roomNumber;
        string bedNumber;
    public:
    Inpatient(string id, string name, string dob, string gender, string phone, 
              string recordId, double discount, string admDate, string disDate, 
              string room, string bed)
        : Patient(id, name, dob, gender, phone, recordId, discount),
          admissionDate(admDate), dischargeDate(disDate), 
          roomNumber(room), bedNumber(bed) {}
        ~Inpatient() override;
        string getType() const override {
        return "NoiTru";
    }
        void inputData();
        void displayData() const override;

        void assignRoom();
        void processDischarge();
};

#endif