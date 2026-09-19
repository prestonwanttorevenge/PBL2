#ifndef DOCTOR_H
#define DOCTOR_H
#include "Person.h"
//Doctor class inherits from the Person class.
class Doctor : public Person{
    private:
        string specialty;
        int yearOfExperience;
        string workSchedule;
    
    public:
    //Default constructor and parameterized constructor
    Doctor();
    Doctor(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears, string schedule);
    //Destructor
    ~Doctor() override;
    //Getting method
    string getSpecialty() const;
    int getYearsOfExperience() const;
    string getWorkSchedule() const;
    //Seting method 
    void setSpecialty(string specialty);
    void setYearsOfExperience(int expYears);
    void setWorkSchedule(string schedule);
    //Override the virtual functions of parent class
    void inputData() override;
    void displayData() const override;
    //Specific function of Doctor
    void viewSchedule() const;
};

#endif 