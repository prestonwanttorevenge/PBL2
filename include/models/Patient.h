#ifndef PATIENT_H
#define PATIENT_H
#include "Person.h"
#include <cctype>

class Patient : public Person {
protected: 
    string medicalRecordId;   
    string BHYT;
public:
    Patient();
    Patient(string id, string name, string dob, string gender, string phone, 
            string recordId, string BH)
        : Person(id, name, dob, gender, phone), 
          medicalRecordId(recordId), BHYT(BH){}
    Patient(Patient &);
    virtual ~Patient() override{}
    virtual string getType() const = 0;
    // Chỉ coi là có BHYT khi mã thẻ hợp lệ (không nhận chuỗi rác hoặc từ phủ định).
    bool hasBHYT() const {
        size_t start = 0;
        while (start < BHYT.size() && std::isspace(static_cast<unsigned char>(BHYT[start]))) {
            ++start;
        }
        size_t end = BHYT.size();
        while (end > start && std::isspace(static_cast<unsigned char>(BHYT[end - 1]))) {
            --end;
        }
        const string trimmed = BHYT.substr(start, end - start);
        if (trimmed.size() < 6) return false;
        string lower;
        lower.reserve(trimmed.size());
        for (unsigned char ch : trimmed) {
            if (!std::isalnum(ch)) return false;
            lower.push_back(static_cast<char>(std::tolower(ch)));
        }
        return lower != "khong" && lower != "none";
    }
    string getMedicalRecordId() const;
    string getBHYT() const;

    void setMedicalRecordId(string recordId);
    void setBHYT(string BH);

    void inputData() override;
    void displayData() const override;
    
    void viewMedicalHistory() const;
};

#endif