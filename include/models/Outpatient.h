#ifndef OUTPATIENT_H
#define OUTPATIENT_H
#include "Patient.h"
#include <string>

class Outpatient : public Patient {
    private:
        std::string appointmentDate;
        std::string clinicRoom;
    public:
        Outpatient();
        ~Outpatient() override;

        void inputData() override;
        void displayData() const override;

        void bookAppointment();

};

#endif