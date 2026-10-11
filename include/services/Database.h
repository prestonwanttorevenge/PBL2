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
    const std::string doctorScheduleFile = "data_doctor_schedules.csv";
    void sanitizeInput(string& input);
public:

    Database();
    Database(Database&);
    ~Database();
    
    Doctor* findDoctorById(string id);
    Patient* findPatientById(string id);
    Medicine* findMedicineById(string id);
    Appointment* findAppointmentById(string id);
    MedicalRecord* findMedicalRecordById(string id); 
    std::vector<Patient*> searchPatients(std::string keyword);

    std::vector<User>& getUsers() { return users; }
    std::vector<Doctor>& getDoctors() { return doctors; }
    std::vector<Patient*>& getPatients() { return patients; }
    std::vector<Medicine>& getMedicines() { return medicines; }
    std::vector<Appointment>& getAppointments() { return appointments; }
    std::vector<Invoice>& getInvoices() { return invoices; }
    std::vector<MedicalRecord>& getMedicalRecords() { return medicalRecords; }
    std::vector<Service>& getServices() { return services; }

    void deleteDoctor(string id);
    void deleteMedicine(string id);
    void deletePatient(string id);
    void updateDoctor(string id);
    void updateMedicine(string id);
    void updatePatient(string id);

    void loadAllData();
    void saveAllData();

    void addDoctor();
    void showAllDoctors() const;

    void addPatient(); 
    void showAllPatients() const;

    void addInvoice(const Invoice& inv);
    void showAllInvoices() const;

    void addMedicine();
    void showAllMedicine() const;

    void addUser(const User& user);
    void showAllUsers() const;

    void addAppointment(const Appointment& appt);
    void showAllAppointments() const;

    void addMedicalRecord(const MedicalRecord& record);
    void showAllMedicalRecords() const;

    void addService(const Service& srv);
    void showAllServices() const;

    bool addInpatientFromQT(string id, string name, string dob, string gender, string phone,string recordId, double discount, string admDate, string disDate, string room, string bed);
    bool addOutpatientFromQT(string id, string name, string dob, string gender, string phone, string recordId, double discount, string apptDate, string clinic);
    bool addDoctorFromQT(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears);
    bool addShiftToDoctorFromQT(string doctorId, string day, string time);
    int addMedicineFromQT(string medicineId,string medicineName,double unitPrice,string dosage,int addQty);
    vector<Doctor*> getAvailableDoctors(string day, string shift);
    int bookAppointmentFromQT(string patientId, string doctorId, string receptionistId, string day, string time);
    int createInvoiceFromQT(std::string patientId, double medFee);
    int createMedicalRecordFromQT(string apptId, string symp, string diag, string notes);
    int prescribeMedicineFromQT(string recordId, string medId, string dosage, int qty);
    int createInvoiceFromQT(std::string patientId, double& outTotalFee);
    std::vector<MedicalRecord*> getPatientHistory(std::string patientId);
    
    //login
    User* loginFromQT(std::string username, std::string password);
};

#endif