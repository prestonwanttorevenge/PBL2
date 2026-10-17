#ifndef DATABASE_H
#define DATABASE_H

#include <vector>
#include "BST.h"
#include <string>
#include "models/Doctor.h"
#include "models/Patient.h"
#include "models/Inpatient.h"
#include "models/Outpatient.h"
#include "models/Medicine.h"
#include "models/Invoice.h"
#include "models/User.h"
#include "models/Appointment.h"
#include "models/MedicalRecord.h"
#include "models/Service.h"

using namespace std;

class Database {
private:
    BST<Doctor> doctors;
    BST<Patient*> patients;
    vector<Invoice> invoices;
    BST<Medicine> medicines;

    BST<User> users;
    vector<Appointment> appointments;
    vector<MedicalRecord> medicalRecords;
    BST<Service> services;

    const std::string doctorFile = "data/doctors.csv";
    const std::string patientFile = "data/patients.csv";
    const std::string invoiceFile = "data/invoices.csv";
    const std::string invoiceDetailsFile = "data/data_invoice_details.csv";
    const std::string medicineFile = "data/medicines.csv";
    const std::string userFile = "data/users.csv";
    const std::string appointmentFile = "data/data_appointments.csv";
    const std::string medicalRecordFile = "data/medical_records.csv";
    const std::string serviceFile = "data/data_services.csv";
    const std::string doctorScheduleFile = "data/data_doctor_schedules.csv";
    const std::string prescriptionDetailFile = "data/data_prescription_details.csv";
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
    vector<Patient*> searchPatients(string keyword);

    BST<User>& getUsers() { return users; }
    BST<Doctor>& getDoctors() { return doctors; }
    BST<Patient*>& getPatients() { return patients; }
    BST<Medicine>& getMedicines() { return medicines; }
    BST<Service>& getServices() { return services; }

    vector<Appointment>& getAppointments() { return appointments; }
    vector<Invoice>& getInvoices() { return invoices; }
    vector<MedicalRecord>& getMedicalRecords() { return medicalRecords; }

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

    // ==========================================
    // Hệ thống & Đăng nhập (System / Login)
    // ==========================================
    User* loginFromQT(string username, string password);

    // ==========================================
    // Quản Trị Viên (Admin)
    // ==========================================
    int addUserFromQT(string id, string username, string plainPassword, string fullName, string role);
    int addDoctorWithAccount(string id, string name, string dob, string gender, string phone, string specialty, int expYears, string username, string plainPassword);
    bool addDoctorFromQT(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears);
    bool addShiftToDoctorFromQT(string doctorId, string day, string time);
    int addServiceFromQT(string id, string name, double price);
    
    bool deleteUserFromQT(const std::string& id);
    bool deleteDoctorFromQT(const std::string& id);
    bool deleteMedicineFromQT(const std::string& id);
    bool deleteServiceFromQT(const std::string& id);

    // ==========================================
    // Lễ Tân (Receptionist)
    // ==========================================
    bool addPatientFromGUI(string id, string name, string phone, bool isNoiTru);
    bool addInpatientFromQT(string id, string name, string dob, string gender, string phone,string recordId, double discount, string admDate, string disDate, string room, string bed);
    bool addOutpatientFromQT(string id, string name, string dob, string gender, string phone, string recordId, double discount, string apptDate, string clinic);
    
    vector<Doctor*> getAvailableDoctors(string day, string shift);
    int bookAppointmentFromQT(string patientId, string doctorId, string receptionistId,string specialty ,string day, string time);
    int createInvoiceFromQT(string patientId, double& outTotalFee);

    // ==========================================
    // Bác Sĩ (Doctor)
    // ==========================================
    int createMedicalRecordFromQT(string apptId, string symp, string diag, string notes);
    int prescribeMedicineFromQT(string recordId, string medId, string dosage, int qty);
    vector<MedicalRecord*> getPatientHistory(string patientId);

    // ==========================================
    // Dùng chung: Bác Sĩ & Lễ Tân
    // ==========================================
    int cancelAppointmentFromQT(string apptId);

    // ==========================================
    // Quản Lý Kho / Dược Sĩ (Inventory Menu & Admin Menu)
    // ==========================================
    int addMedicineFromQT(string medicineId,string medicineName,double unitPrice,string dosage,int addQty);
};

#endif