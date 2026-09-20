#ifndef OUTPATIENT_H
#define OUTPATIENT_H
#include "Patient.h"
#include <string>

class Outpatient : public Patient {
    private:
        std::string appointmentDate;
        std::string clinicRoom;
    public:
    Outpatient(string id, string name, string dob, string gender, string phone, 
              string recordId, double discount , string appDate , string clinicroom)
        : Patient(id, name, dob, gender, phone, recordId, discount),
          appointmentDate(appDate) , clinicRoom(clinicroom){}
        ~Outpatient() override;

        void inputData();
        void displayData() const override;

        void bookAppointment();

};

#endif