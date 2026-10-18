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
              string recordId, string BH , string appDate , string clinicroom)
        : Patient(id, name, dob, gender, phone, recordId, BH),
          appointmentDate(appDate) , clinicRoom(clinicroom){}
        ~Outpatient() override{}
        std::string getType() const override {
        return "NgoaiTru";
    }
        std::string getAppointmentDate()const{return appointmentDate;}
        std::string getClinicRoom()const{return clinicRoom;}
        void inputData();
        void displayData() const override;
        void bookAppointment();
        void setClinicRoom(string clinic){ this->clinicRoom = clinic;}
        void setAppointmentDate(string appointment){ this->appointmentDate = appointment;}
};

#endif