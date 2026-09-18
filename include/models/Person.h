#ifndef PERSON_H
#define PERSON_H
#include <iostream>
#include <string>
using namespace std;
class Person {
protected: 
    string id;
    string fullName;
    string dateOfBirth;
    string gender;
    string phoneNumber;

public:
    Person();
    Person(string id, string fullName, string dob, string gender, string phone);
    virtual ~Person();
    string getId() const;
    string getFullName() const;
    string getDateOfBirth() const;
    string getGender() const;
    string getPhoneNumber() const;
    void setId(string id);
    void setFullName(string name);
    void setDateOfBirth(string dob);
    void setGender(string gender);
    void setPhoneNumber(string phone);
    virtual void inputData();
    virtual void displayData() const; 
};

#endif