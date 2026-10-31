#ifndef DATABASE_H
#define DATABASE_H
#include <string>
#include <vector>
#include <map>
#include "utils/CustomMap.h"
#include "utils/RBTree.h"
#include "models/User.h"
#include "models/Doctor.h"
#include "models/Patient.h"
#include "models/Inpatient.h"
#include "models/Outpatient.h"
#include "models/Medicine.h"
#include "models/Service.h"
#include "models/Appointment.h"
#include "models/MedicalRecord.h"
#include "models/Invoice.h"
#include "models/InvoiceDetail.h"

using namespace std;

class Database {
private:
    int patientCounter = 0;
    int appointmentCounter = 0;
    int doctorCounter = 0;
    int medicalRecordCounter = 0;

    // Bộ đếm ID theo nhóm; các danh mục chính được tra cứu theo khóa trong RBTree.
    CounterMap userCounters;
    CounterMap medicineCounters;
    CounterMap serviceCounters;

    string doctorFile = "data/doctors.csv";
    string doctorScheduleFile = "data/data_doctor_schedules.csv";
    string patientFile = "data/patients.csv";
    string invoiceFile = "data/invoices.csv";
    string invoiceDetailsFile = "data/data_invoice_details.csv";
    string userFile = "data/users.csv";
    string medicineFile = "data/medicines.csv";
    string serviceFile = "data/data_services.csv";
    string appointmentFile = "data/data_appointments.csv";
    string medicalRecordFile = "data/medical_records.csv";
    string prescriptionDetailFile = "data/data_prescription_details.csv";

    // Giữ thứ tự theo ID và tìm kiếm/xóa nhanh theo khóa.
    RBTree<string, Doctor> doctors;
    RBTree<string, Patient*> patients;
    RBTree<string, Medicine> medicines;
    RBTree<string, Service> services;
    RBTree<string, User> users;

    vector<Appointment> appointments;
    vector<MedicalRecord> medicalRecords;
    vector<Invoice> invoices;

    void sanitizeInput(string& input);

public:
    Database();
    ~Database();

    RBTree<string, User>& getUsers() { return users; }
    RBTree<string, Doctor>& getDoctors() { return doctors; }
    RBTree<string, Patient*>& getPatients() { return patients; }
    RBTree<string, Medicine>& getMedicines() { return medicines; }
    RBTree<string, Service>& getServices() { return services; }

    vector<Appointment>& getAppointments() { return appointments; }
    vector<Invoice>& getInvoices() { return invoices; }
    vector<MedicalRecord>& getMedicalRecords() { return medicalRecords; }

    string generatePatientId();
    string generateAppointmentId();
    string generateMedicalRecordId();
    string generateUserId(const string& role);
    string generateDoctorId();
    string generateMedicineId(const string& prefix);
    string generateServiceId(const string& prefix);

    void loadAllData();
    void saveAllData();

    Doctor* findDoctorById(string id);
    Patient* findPatientById(string id);
    vector<Patient*> searchPatients(string keyword);
    Medicine* findMedicineById(string id);
    Appointment* findAppointmentById(string id);
    MedicalRecord* findMedicalRecordById(string id);

    bool deleteUserFromQT(const string& id);
    bool setUserActiveFromQT(const string& id, bool active);
    int deleteDoctorFromQT(const string& id);
    int deleteMedicineFromQT(const string& id);
    bool deleteServiceFromQT(const string& id);

    bool addPatientFromGUI(string name, string dob, string gender, string phone,
                           string BH, bool isNoiTru, string admissionDate);
    int bookAppointmentFromQT(string patientId, string doctorId, string receptionistId, string specialty, string day, string time, string appointmentDate);
    // Chỉ xác nhận đến khi lịch Scheduled đúng ngày hiện tại.
    int checkInAppointmentFromQT(const string& appointmentId, const string& currentDate);
    string updateMedicalRecordFromQT(string apptId, string symp, string diag, string notes, string status = "Completed");
    User* loginFromQT(string username, string password);
    int addUserFromQT(string id, string username, string plainPassword, string fullName, string role);
    int addDoctorWithAccount(string id, string name, string dob, string gender, string phone,
                             string specialty, int expYears, string username, string plainPassword,
                             string workDay, string workShift);
    int addMedicineFromQT(string prefix, string medicineName, double unitPrice, string dosage, int addQty, string& outId);
    int assignServiceFromQT(string recordId, string serviceId);
    // Bản đầy đủ: chỉ ghi triệu chứng/chẩn đoán/ghi chú khi chỉ định hợp lệ rồi lưu file.
    int assignServiceFromQT(string recordId, string serviceId, string symp, string diag, string notes);
    int addServiceFromQT(string prefix, string name, double price, string& outId);
    double calculateTotalRevenue();
    vector<Doctor*> getAvailableDoctors(string day, string shift);
    int prescribeMedicineFromQT(string recordId, string medId, string dosage, int qty);
    // Cấp phát và trừ tồn kho sau khi kho xác nhận.
    int dispensePrescriptionFromQT(string recordId, string medicineId);
    // Bỏ các dòng thuốc chưa cấp (bệnh nhân không nhận) và kết thúc lượt chờ kho.
    int abandonUndispensedPrescriptionsFromQT(string apptId);
    int createInvoiceFromQT(string patientId, double& outTotalFee);
    vector<MedicalRecord*> getPatientHistory(string patientId);
    int cancelAppointmentFromQT(string apptId);

    map<string, vector<string>> getAvailableRoomMap();
    int updateInpatientBed(string patientId, string room, string bed);
    int dischargeInpatientFromQT(string patientId, string dischargeDate);

    void deleteDoctor(string id);
    void deleteMedicine(string id);
    void deletePatient(string id);
    void updateDoctor(string id);
    void updateMedicine(string id);
    void updatePatient(string id);
    void addDoctor();
    void showAllDoctors() const;
    void addMedicine();
    void showAllMedicine() const;
    void addPatient();
    void showAllPatients() const;
};

#endif // DATABASE_H