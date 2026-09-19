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
        Inpatient();
        ~Inpatient() override;

        void inputData();
        void displayData() const override;

        void assignRoom();
        void processDischarge();
};

#endif