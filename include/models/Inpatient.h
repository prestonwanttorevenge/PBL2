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
        Inpatient();
        ~Inpatient() override;

        void inputData() override;
        void displayData() const override;

        void assignRoom();
        void processDischarge();
};

#endif