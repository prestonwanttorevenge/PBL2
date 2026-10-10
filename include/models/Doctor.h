#ifndef DOCTOR_H
#define DOCTOR_H
#include "Person.h"
#include <vector>
#include <string>
//Doctor class inherits from the Person class.

typedef struct WorkShift{
    string dayOfWeek;
    string timeSlot; 
}Workshift;
class Doctor : public Person{
    private:
        string specialty;
        int yearOfExperience;
        vector<WorkShift> workSchedule;
    
    public:
    //Default constructor and parameterized constructor
    Doctor();
    Doctor(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears);
    //Destructor
    ~Doctor() override;
    //Getting method
    string getSpecialty() const;
    int getYearsOfExperience() const;
    const vector<WorkShift>& getSchedule() const;
    //Seting method 
    void setSpecialty(string specialty);
    void setYearsOfExperience(int expYears);
    void setWorkSchedule(string , string );
    //Override the virtual functions of parent class
    void inputData() override;
    void displayData() const override;
    //Specific function of Doctor
    void viewSchedule() const;
    void clearSchedule(){workSchedule.clear();}
};

#endif 