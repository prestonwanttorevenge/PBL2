#ifndef INPATIENT_H
#define INPATIENT_H
#include "Patient.h"
#include <string>

class Inpatient : public Patient {
    private:
        std::string admissionDate;
        std::string dischargeDate;
        std::string roomNumber;
        std::string bedNumber;
    public:
    Inpatient(string id, string name, string dob, string gender, string phone, 
              string recordId, double discount, string admDate, string disDate, 
              string room, string bed)
        : Patient(id, name, dob, gender, phone, recordId, discount),
          admissionDate(admDate), dischargeDate(disDate), 
          roomNumber(room), bedNumber(bed) {}
        ~Inpatient() override;

        void inputData();
        void displayData() const override;

        void assignRoom();
        void processDischarge();
};

#endif