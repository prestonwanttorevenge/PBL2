#ifndef MEDICINE_H
#define MEDICINE_H

#include <iostream>
#include <string>

class Medicine {
private:
    std::string medicineId;
    std::string medicineName;
    double unitPrice;
    std::string dosage;

public:
    Medicine(std::string m_id = "-1", std::string m_name = "ERROR", double m_price = 0.0, std::string m_dose = "ERROR")
        : medicineId(m_id), medicineName(m_name), unitPrice(m_price), dosage(m_dose) {}
    Medicine(const Medicine &);
    ~Medicine(){}
    ~Medicine(){}
    std::string getMedicineId() const { return medicineId; }
    std::string getMedicineName() const { return medicineName; }

    double getUnitPrice() const { return unitPrice; }
    std::string getDosage() const { return dosage; }

    void setUnitPrice(double newPrice) { unitPrice = newPrice; }
    void setDosage(std::string newDosage) { dosage = newDosage; }

    void displayData() const {
        std::cout << "Ma thuoc: " << medicineId << std::endl;
        std::cout << "Ten thuoc: " << medicineName << std::endl;
        std::cout << "Lieu luong: " << dosage << std::endl;
        std::cout << "Don gia: " << unitPrice << " VND" << std::endl;
    }

    void inputData() {
        std::cin.sync();
        
        std::cout << "Nhap Ma thuoc: ";
        std::getline(std::cin, medicineId);
        
        std::cout << "Nhap Ten thuoc: ";
        std::getline(std::cin, medicineName);
        
        std::cout << "Nhap Lieu luong: ";
        std::getline(std::cin, dosage);
        
        std::cout << "Nhap Don gia: ";
        std::string tempPrice;
        std::getline(std::cin, tempPrice);
        try{
            unitPrice = std::stod(tempPrice);
        }catch (...) {
            unitPrice = 0.0;
        }
    }
};

#endif