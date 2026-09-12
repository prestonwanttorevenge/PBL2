#include <iostream>
#include <string>
#ifndef PERSON_H
#define PERSON_H
class Person{
    protected:
    std::string id;
    std::string fullName;
    std::string dateOfBirth;
    std::string gender;
    std::string phoneNumber;
    public:
    Person(std::string p_id = "-1",
       std::string p_fullName = "ERROR",
       std::string p_dateOfBirth = "ERROR",
       std::string p_gender = "ERROR",
       std::string p_phoneNumber = "ERROR") 
    : id(p_id), fullName(p_fullName), dateOfBirth(p_dateOfBirth), gender(p_gender), phoneNumber(p_phoneNumber){}
    virtual void inputData(){}
    virtual void displayData() const{
        std::cout << "Mã ID: " << id << std::endl;
        std::cout << "Họ và tên: " << fullName << std::endl;
        std::cout << "Ngày sinh: " << dateOfBirth << std::endl;
        std::cout << "Giới tính: " << gender << std::endl;
        std::cout << "Số điện thoại: " << phoneNumber << std::endl;
    }
    std::string getId() const { return id; }
    virtual ~Person() {}
};
#endif