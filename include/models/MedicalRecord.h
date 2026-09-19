#include<iostream>
#include<vector>
#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

struct PrescriptionDetail{
    std::string medicineId;
    std::string dosage;     
    int quantity;  
};
class MedicalRecord{
    private:
    std::string recordId;
    std::string appointmentId;
    std::string symptoms;
    std::string diagnosis;
    std::string notes;
    std::vector<PrescriptionDetail> prescriptions;
    public:
    MedicalRecord(std::string recordId="-1",std::string appointmentId="-1")
    : recordId(recordId),appointmentId(appointmentId),symptoms(""),diagnosis(""),notes(""){}
    MedicalRecord(MedicalRecord&);
    ~MedicalRecord(){}
    void inputData();
    void displayData();
    void addMedicine();
};

#endif