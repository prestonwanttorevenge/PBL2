#ifndef MEDICALRECORD_H
#define MEDICALRECORD_H

#include <iostream>
#include <vector>
#include <string>

struct PrescriptionDetail {
    std::string medicineId;
    std::string dosage;     
    int quantity;  
    double unitPrice;
    bool dispensed;
};

class MedicalRecord {
private:
    std::string recordId;
    std::string appointmentId;
    std::string symptoms;
    std::string diagnosis;
    std::string notes;
    std::vector<PrescriptionDetail> prescriptions;
    std::vector<std::string> assignedServices;

public:
    MedicalRecord(std::string recordId = "-1", std::string appointmentId = "-1", 
              std::string symp = "", std::string diag = "", std::string n = "")
    : recordId(recordId), appointmentId(appointmentId), symptoms(symp), diagnosis(diag), notes(n) {}
    ~MedicalRecord();

    std::string getRecordId() const;
    std::string getAppointmentId() const;
    std::string getSymptoms() const;
    std::string getDiagnosis() const;
    std::string getNotes() const;
    std::vector<std::string> getAssignedServices() const { return assignedServices; }
    const std::vector<PrescriptionDetail>& getPrescriptions() const { return prescriptions; }
    void setSymptoms(const std::string& symp) { symptoms = symp; }
    void setDiagnosis(const std::string& diag) { diagnosis = diag; }
    void setNotes(const std::string& n) { notes = n; }
    void addService(std::string serviceId) { assignedServices.push_back(serviceId); }
    void inputData();
    void displayData() const;
    void addMedicine(std::string mId, std::string dose, int qty);
    void addMedicine(std::string mId, std::string dose, int qty, double price, bool isDispensed = false);
    bool hasMedicine(const std::string& medicineId) const;
    bool isMedicineDispensed(const std::string& medicineId) const;
    bool markMedicineDispensed(const std::string& medicineId);
};

#endif