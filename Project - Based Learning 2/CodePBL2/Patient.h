#include <iostream>
#include <string>
#include "Person.h"
class Patient : public Person{
    private:
    std::string medicalRecordId;
    double insuranceDiscount;
    public:
    void inputData() override{
        Person::inputData();
    }
    void displayData() const override{
        Person::displayData();
    }
    virtual void viewMedicalHistory(){
    }
};