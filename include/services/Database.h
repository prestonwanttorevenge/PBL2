#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include "Doctor.h"
#include "Patient.h"
#include "Medicine.h" 
#include "Invoice.h"

class Database{
private:
    std::vector<Doctor> doctors;
    std::vector<Patient*> patients;
    std::vector<Invoice> invoices;
    std::vector<Medicine> medicines;
    const std::string doctorFile = "data_doctors.csv";
    const std::string patientFile = "data_patients.csv";
    const std::string invoiceFile = "data_invoice.csv";
    const std::string medicineFile = "data_medicine.csv";
public:
    Database(); 
    ~Database();

    void loadAllData();
    void saveAllData();

    void addDoctor(const Doctor& doc);
    void showAllDoctors() const;
    
    void addPatient(const Patient& pat);
    void showAllPatients() const;

    void addInvoice(const Invoice& inv);
    void showAllInvoices() const;

    void addMedicine(const Medicine& med);
    void showAllMedicine() const;
};

#endif