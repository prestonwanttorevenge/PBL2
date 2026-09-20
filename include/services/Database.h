#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include <string>
#include "Doctor.h"
#include "Patient.h"
#include "Inpatient.h" 
#include "Outpatient.h"
#include "Medicine.h" 
#include "Invoice.h"
#include "User.h"
#include "Appointment.h"
#include "MedicalRecord.h"
#include "Service.h"

class Database {
private:
    std::vector<Doctor> doctors;
    std::vector<Patient*> patients; 
    std::vector<Invoice> invoices;
    std::vector<Medicine> medicines;
 
    std::vector<User> users;
    std::vector<Appointment> appointments;
    std::vector<MedicalRecord> medicalRecords;
    std::vector<Service> services;

    const std::string doctorFile = "data_doctors.csv";
    const std::string patientFile = "data_patients.csv";
    const std::string invoiceFile = "data_invoice.csv";
    const std::string invoiceDetailsFile = "data_invoice_details.csv";
    const std::string medicineFile = "data_medicine.csv";
    const std::string userFile = "data_users.csv";
    const std::string appointmentFile = "data_appointments.csv";
    const std::string medicalRecordFile = "data_medical_records.csv";
    const std::string serviceFile = "data_services.csv";
public:
    Database();
    Database(Database&);
    ~Database();

    void loadAllData();
    void saveAllData();

    void addDoctor(const Doctor& doc);
    void showAllDoctors() const;

    void addPatient(Patient* pat); 
    void showAllPatients() const;

    void addInvoice(const Invoice& inv);
    void showAllInvoices() const;

    void addMedicine(const Medicine& med);
    void showAllMedicine() const;

    void addUser(const User& user);
    void showAllUsers() const;

    void addAppointment(const Appointment& appt);
    void showAllAppointments() const;

    void addMedicalRecord(const MedicalRecord& record);
    void showAllMedicalRecords() const;

    void addService(const Service& srv);
    void showAllServices() const;
};

#endif