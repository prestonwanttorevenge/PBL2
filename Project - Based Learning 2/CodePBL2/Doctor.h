#include <iostream>
#include <string>
#include "Person.h"
class Doctor : public Person{
    private:
    std::string specialty;
    int yearOfExperience;
    std::string workSchedule;
    public:
    void inputData(){
        Person::inputData();

    }
    void displayDat(){
        Person::displayData();

    }
    void viewSchedule(){};
};