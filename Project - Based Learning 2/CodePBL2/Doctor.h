#include <iostream>
#include <string>
#include "Person.h"
class Doctor : public Person{
    private:
    std::string specialty;
    int yearOfExperience;
    std::string workSchedule;
    public:
    Doctor(std::string d_speacialty ="ERROR", int d_yearOfExperience = 0, std::string d_workSchedule = "ERROR" )
    : specialty(d_speacialty), yearOfExperience(d_yearOfExperience),workSchedule(d_workSchedule){}
    Doctor(const Doctor &);
    virtual ~Doctor(){};
    void inputData(){
        Person::inputData();

    }
    void displayDat(){
        Person::displayData();

    }
    void viewSchedule(){};
};