#ifndef OUTPATIENT_H
#define OUTPATIENT_H
#include "Patient.h"

class Outpatient : public Patient {
    private:
        string appointmentDate;
        string clinicRoom;
    public:
        Outpatient();
        ~Outpatient() override;

        void inputData();
        void displayData() const override;

        void bookAppointment();

};

#endif