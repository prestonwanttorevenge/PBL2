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
    int stock;
public:
    Medicine(std::string m_id = "-1", std::string m_name = "ERROR", double m_price = 0.0, std::string m_dose = "ERROR",int stock = 0);
    ~Medicine();

    std::string getMedicineId() const;
    std::string getMedicineName() const;
    double getUnitPrice() const;
    std::string getDosage() const;
    int getStock() const {return stock;}
    void setUnitPrice(double newPrice);
    void setDosage(std::string newDosage);
    void setStock(int stock) {this->stock = stock;}
    
    void displayData() const;
    void inputData();   
};
#endif