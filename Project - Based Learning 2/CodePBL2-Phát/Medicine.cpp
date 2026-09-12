#include <iostream>
#include <string>
class Medicine {
private:
    std::string medicineId;
    std::string medicineName;
    double unitPrice;
    std::string dosage;

public:
    Medicine(std::string id, std::string name, double price, std::string dose) {
        medicineId = id;
        medicineName = name;
        unitPrice = price;
        dosage = dose;
    }
};