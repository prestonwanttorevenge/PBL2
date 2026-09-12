#include <iostream>
#include <string>
#include "Person.h"
class Patient : public Person{
    private:
    std::string medicalRecordId;
    double insuranceDiscount;
    public:
    Patient(){
    }
    void inputData() override{
        Person::inputData();
    }
    void displayData() const override{
        Person::displayData();
    }
    virtual void viewMedicalHistory(){
    }
};
class Inpatient : public Patient{
    private:
    std::string admissionDate;
    std::string dischargeDate;
    std::string roomNumber;
    std::string bedNumber;
    public:
    virtual ~Inpatient(){}
    void assignRoom(){}
    void processDischarge(){}
};
class Outpatient : public Patient{
    private:
    std::string appointmentDate;
    std::string clinicRoom;
    public:
    void bookAppointment(){

    }
};