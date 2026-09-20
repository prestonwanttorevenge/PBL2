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
    MedicalRecord(std::string recordId = "-1", std::string appointmentId = "-1", 
              std::string symp = "", std::string diag = "", std::string n = "")
    : recordId(recordId), appointmentId(appointmentId), symptoms(symp), diagnosis(diag), notes(n) {}

    std::string getRecordId() const;
    std::string getAppointmentId() const;
    std::string getSymptoms() const;
    std::string getDiagnosis() const;
    std::string getNotes() const;
    const vector<PrescriptionDetail>& getPrescriptions() const { return prescriptions; }
    void inputData();
    void displayData() const;
    void addMedicine(std::string mId, std::string dose, int qty);
};

#endif