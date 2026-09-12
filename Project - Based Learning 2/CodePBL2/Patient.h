#include <iostream>
#include <string>
#include "Person.h"
class Patient : public Person{
    private:
    std::string medicalRecordId;
    double insuranceDiscount;
    public:
    Patient( std::string pa_medicalRecordId = "ERROR", double pa_insuranceDiscount = 0.0) 
    : medicalRecordId(pa_medicalRecordId) , insuranceDiscount(pa_insuranceDiscount) {}
    Patient(const Patient &);
    virtual ~Patient(){}
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
    Inpatient(std::string ipa_admissionDate = "ERROR", std::string ipa_dischargeDate = "ERROR", std::string ipa_roomNumber = "ERROR", std::string ipa_bedNumber= "ERROR")
    : admissionDate(ipa_admissionDate) , dischargeDate(ipa_dischargeDate) , roomNumber(ipa_roomNumber) , bedNumber(ipa_bedNumber){}
    virtual ~Inpatient(){}
    void assignRoom(){}
    void processDischarge(){}
};
class Outpatient : public Patient{
    private:
    std::string appointmentDate;
    std::string clinicRoom;
    public:
    Outpatient( std::string opa_appointmentDate = "ERROR", std::string opa_clinicRoom = "ERROR")
    : appointmentDate(opa_appointmentDate) , clinicRoom(opa_clinicRoom){}
    Outpatient(const Outpatient &);
    virtual ~Outpatient(){}
    void bookAppointment(){

    }
};