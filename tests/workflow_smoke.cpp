#include "services/Database.h"
#include "utils/AppointmentQueue.h"
#include "models/bcrypt.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <set>
#include <string>

#define CHECK(expr)                                                                      \
    do {                                                                                 \
        if (!(expr)) {                                                                   \
            std::cerr << "CHECK failed at " << __FILE__ << ":" << __LINE__               \
                      << " -> " << #expr << '\n';                                        \
            std::exit(1);                                                                \
        }                                                                                \
    } while (false)

namespace {

class TemporaryDataDirectory {
public:
    TemporaryDataDirectory()
        : originalPath_(std::filesystem::current_path()),
          testPath_(std::filesystem::temp_directory_path() /
                    ("pbl2-workflow-" + std::to_string(
                        std::chrono::steady_clock::now().time_since_epoch().count()))) {
        std::filesystem::create_directories(testPath_ / "data");
        std::filesystem::current_path(testPath_);

        write("doctors.csv",
              "D01;Doctor One;01/01/1980;Nam;0900000000;Khoa Nội tổng hợp;5\n");
        write("data_doctor_schedules.csv", "D01;Thứ 2;Sáng\n");
        write("patients.csv",
              "NgoaiTru;P1;Patient One;01/01/1990;Nam;0900000001;;No;01/01/2026;P101\n"
              "NgoaiTru;P2;Patient Two;01/01/1990;Nam;0900000002;;No;01/01/2026;P102\n"
              "NgoaiTru;P3;Patient Three;01/01/1990;Nam;0900000003;;No;01/01/2026;P103\n"
              "NgoaiTru;P4;Patient Four;01/01/1990;Nam;0900000004;;No;01/01/2026;P104\n"
              "NgoaiTru;P5;Patient Five;01/01/1990;Nam;0900000005;;123456789;01/01/2026;P105\n"
              "NoiTru;P6;Inpatient One;01/01/1990;Nam;0900000006;;No;01/01/2026;;Chua xep;\n"
              "NoiTru;P7;Inpatient Two;01/01/1990;Nam;0900000007;;No;01/01/2026;;Chua xep;\n");
        write("medicines.csv",
              "M1;Medicine One;1000;1 per day;20\n"
              "M2;Medicine Two;2000;2 per day;20\n"
              "B1_001;Covered Medicine;1000;1 per day;20\n");
        write("data_services.csv",
              "S1_001;Consultation;100000\n"
              "S2_001;Lab One;50000\n"
              "S2_002;Lab Two;75000\n");
    }

    ~TemporaryDataDirectory() {
        std::filesystem::current_path(originalPath_);
        std::filesystem::remove_all(testPath_);
    }

private:
    std::filesystem::path originalPath_;
    std::filesystem::path testPath_;

    static void write(const std::string& filename, const std::string& content) {
        std::ofstream file(std::filesystem::path("data") / filename);
        CHECK(file.is_open());
        file << content;
        CHECK(file.good());
    }
};

MedicalRecord* findRecord(Database& database, const std::string& appointmentId) {
    for (MedicalRecord& record : database.getMedicalRecords()) {
        if (record.getAppointmentId() == appointmentId) return &record;
    }
    return nullptr;
}

const Invoice* findInvoice(Database& database, const std::string& appointmentId) {
    for (const Invoice& invoice : database.getInvoices()) {
        if (invoice.getAppointmentId() == appointmentId) return &invoice;
    }
    return nullptr;
}

std::string nextMondayDate() {
    const std::time_t now = std::time(nullptr);
    const std::tm* localNow = std::localtime(&now);
    CHECK(localNow != nullptr);
    std::tm nextMonday = *localNow;
    int daysUntilMonday = (8 - nextMonday.tm_wday) % 7;
    if (daysUntilMonday == 0) daysUntilMonday = 7;
    nextMonday.tm_mday += daysUntilMonday;
    nextMonday.tm_hour = 12;
    nextMonday.tm_min = 0;
    nextMonday.tm_sec = 0;
    const std::time_t timestamp = std::mktime(&nextMonday);
    CHECK(timestamp != static_cast<std::time_t>(-1));
    char formattedDate[11];
    CHECK(std::strftime(formattedDate, sizeof(formattedDate), "%d/%m/%Y",
                        std::localtime(&timestamp)) > 0);
    return formattedDate;
}

} // namespace

void runSyntheticWorkflowTests() {
    TemporaryDataDirectory testData;
    Database database;
    database.loadAllData();

    // Kiểm tra sinh muối Bcrypt ngẫu nhiên (không trùng salt khi gọi liên tiếp cùng giây).
    const std::string h1 = bcrypt::generateHash("test_same_pwd");
    const std::string h2 = bcrypt::generateHash("test_same_pwd");
    CHECK(h1.size() == 60 && h2.size() == 60);
    CHECK(h1 != h2);
    CHECK(h1.substr(7, 22) != h2.substr(7, 22));
    CHECK(bcrypt::validatePassword("test_same_pwd", h1));
    CHECK(bcrypt::validatePassword("test_same_pwd", h2));

    // Kiểm tra hasBHYT từ chối chuỗi rác, chấp nhận mã thẻ hợp lệ.
    Outpatient pInvalid1("T1", "A", "01/01/2000", "Nam", "0900000001", "", "abc", "", "");
    Outpatient pInvalid2("T2", "B", "01/01/2000", "Nam", "0900000002", "", "z", "", "");
    Outpatient pInvalid3("T3", "C", "01/01/2000", "Nam", "0900000003", "", "1", "", "");
    Outpatient pInvalid4("T4", "D", "01/01/2000", "Nam", "0900000004", "", "Không", "", "");
    Outpatient pValid1("T5", "E", "01/01/2000", "Nam", "0900000005", "", "DN448100142997", "", "");
    CHECK(!pInvalid1.hasBHYT());
    CHECK(!pInvalid2.hasBHYT());
    CHECK(!pInvalid3.hasBHYT());
    CHECK(!pInvalid4.hasBHYT());
    CHECK(pValid1.hasBHYT());

    // Kiểm tra quy tắc BHYT dùng chung tầng nghiệp vụ.
    CHECK(Database::calculateMedicineCoPayFactor("B1_001", true) == 0.0);
    CHECK(Database::calculateMedicineCoPayFactor("B2_001", true) == 0.5);
    CHECK(Database::calculateMedicineCoPayFactor("B3_001", true) == 0.2);
    CHECK(Database::calculateMedicineCoPayFactor("DV_001", true) == 1.0);
    CHECK(Database::calculateMedicineCoPayFactor("B1_001", false) == 1.0);
    CHECK(Database::calculateServiceCoPayFactor("S1_001", true) == 0.0);
    CHECK(Database::calculateServiceCoPayFactor("S2_001", true) == 0.3);
    CHECK(Database::calculateServiceCoPayFactor("S3_001", true) == 0.8);
    CHECK(Database::calculateServiceCoPayFactor("SV_001", true) == 1.0);

    const std::string date = nextMondayDate();
    const std::string day = "Thứ 2";
    const std::string shift = "Sáng";
    CHECK(database.addDoctorWithAccount("D02", "Doctor Two", "01/01/1980", "Nam",
                                        "0900000002", "Khoa Khám bệnh", 5,
                                        "doctor2", "password", day, shift) == -1);
    CHECK(database.findDoctorById("D02") == nullptr);
    std::string invalidId;
    CHECK(database.addMedicineFromQT("INVALID", "Invalid Medicine", 100.0,
                                     "1 per day", 1, invalidId) == -2);
    CHECK(database.addMedicineFromQT("B1", "Invalid Price",
                                     std::numeric_limits<double>::quiet_NaN(),
                                     "1 per day", 1, invalidId) == -2);
    CHECK(database.addServiceFromQT("INVALID", "Invalid Service", 100.0,
                                    invalidId) == -2);
    CHECK(database.addServiceFromQT("S1", "Invalid Price",
                                    std::numeric_limits<double>::quiet_NaN(),
                                    invalidId) == -2);
    CHECK(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Khám bệnh",
                                         day, shift, date) == -3);
    CHECK(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp",
                                         day, shift, "28/10/2026") == -3);
    CHECK(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp",
                                         day, shift, "31/02/2026") == -3);
    CHECK(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp",
                                         "Thứ 7", shift, "01/01/2000") == -3);
    CHECK(database.getAppointments().empty());
    CHECK(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    CHECK(database.bookAppointmentFromQT("P2", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    CHECK(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == -5);

    const std::string firstId = database.getAppointments()[0].getAppointmentId();
    const std::string secondId = database.getAppointments()[1].getAppointmentId();
    CHECK(database.checkInAppointmentFromQT(firstId, date) == 1);
    CHECK(database.checkInAppointmentFromQT(secondId, date) == 1);
    CHECK(database.updateMedicalRecordFromQT(secondId, "Symptom", "Diagnosis", "", "Completed").empty());
    CHECK(!database.updateMedicalRecordFromQT(firstId, "Symptom", "Diagnosis", "", "Completed").empty());
    CHECK(database.updateMedicalRecordFromQT(secondId, "Symptom", "Diagnosis", "", "Completed").size() > 0);

    CHECK(database.bookAppointmentFromQT("P3", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    CHECK(database.bookAppointmentFromQT("P4", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    const std::string clsId = database.getAppointments()[2].getAppointmentId();
    const std::string nextId = database.getAppointments()[3].getAppointmentId();
    CHECK(database.checkInAppointmentFromQT(clsId, date) == 1);
    CHECK(database.checkInAppointmentFromQT(nextId, date) == 1);

    Appointment* clsAppointment = database.findAppointmentById(clsId);
    Appointment* nextAppointment = database.findAppointmentById(nextId);
    CHECK(clsAppointment != nullptr && nextAppointment != nullptr);
    CHECK(isNextPendingAppointment(database.getAppointments(), *clsAppointment));
    CHECK(!isNextPendingAppointment(database.getAppointments(), *nextAppointment));

    MedicalRecord* clsRecord = findRecord(database, clsId);
    CHECK(clsRecord != nullptr);
    clsRecord->setSymptoms("Symptom");
    clsRecord->setDiagnosis("Diagnosis");
    clsRecord->setNotes("Notes");
    CHECK(database.assignServiceFromQT(clsRecord->getRecordId(), "S2_001") == 1);
    CHECK(database.assignServiceFromQT(clsRecord->getRecordId(), "S2_002") == 1);
    CHECK(clsAppointment->getStatus() == "Pending");
    CHECK(database.updateMedicalRecordFromQT(clsId, "Symptom", "Diagnosis", "Notes", "Waiting") ==
          clsRecord->getRecordId());
    CHECK(clsAppointment->getStatus() == "Waiting");
    CHECK(isNextPendingAppointment(database.getAppointments(), *nextAppointment));

    CHECK(!database.updateMedicalRecordFromQT(nextId, "Symptom", "Diagnosis", "", "Completed").empty());
    CHECK(database.prescribeMedicineFromQT(clsRecord->getRecordId(), "M1", "1 per day", 2) == 1);
    CHECK(clsAppointment->getStatus() == "WaitingInventory");
    double invoiceTotal = 123.0;
    CHECK(database.createInvoiceFromQT("P3", invoiceTotal) == -5);
    CHECK(invoiceTotal == 0.0);
    CHECK(database.cancelAppointmentFromQT(clsId) == -5);
    CHECK(clsAppointment->getStatus() == "WaitingInventory");
    CHECK(database.prescribeMedicineFromQT(clsRecord->getRecordId(), "M2", "2 per day", 1) == 1);
    CHECK(clsAppointment->getStatus() == "WaitingInventory");
    CHECK(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M1") == 1);
    CHECK(clsAppointment->getStatus() == "WaitingInventory");
    CHECK(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M1") == -4);
    CHECK(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M2") == 1);
    CHECK(clsAppointment->getStatus() == "Completed");
    CHECK(database.findMedicineById("M1")->getStock() == 18);
    CHECK(database.findMedicineById("M2")->getStock() == 19);

    invoiceTotal = 0.0;
    CHECK(database.createInvoiceFromQT("P3", invoiceTotal) == 1);
    CHECK(invoiceTotal == 229000.0);
    CHECK(database.getInvoices().back().getAppointmentId() == clsId);
    database.saveAllData();

    CHECK(database.bookAppointmentFromQT("P5", "D01", "R01", "Khoa Nội tổng hợp",
                                         day, shift, date) == 1);
    const std::string bhytAppointmentId = database.getAppointments().back().getAppointmentId();
    CHECK(database.checkInAppointmentFromQT(bhytAppointmentId, date) == 1);
    MedicalRecord* bhytRecord = findRecord(database, bhytAppointmentId);
    CHECK(bhytRecord != nullptr);
    bhytRecord->setDiagnosis("Diagnosis");
    CHECK(database.assignServiceFromQT(bhytRecord->getRecordId(), "S2_001") == 1);
    CHECK(database.updateMedicalRecordFromQT(bhytAppointmentId, "Symptom", "Diagnosis",
                                             "", "Waiting") == bhytRecord->getRecordId());
    CHECK(database.updateMedicalRecordFromQT(bhytAppointmentId, "Symptom", "Diagnosis",
                                             "", "Completed") == bhytRecord->getRecordId());
    CHECK(database.prescribeMedicineFromQT(bhytRecord->getRecordId(), "B1_001",
                                           "1 per day", 2) == 1);
    CHECK(database.dispensePrescriptionFromQT(bhytRecord->getRecordId(), "B1_001") == 1);
    CHECK(database.createInvoiceFromQT("P5", invoiceTotal) == 1);
    CHECK(invoiceTotal == 15000.0);

    CHECK(database.updateInpatientBed("P6", "P101", "G1") == 1);
    CHECK(database.updateInpatientBed("P7", "P101", "G1") == -3);
    CHECK(database.updateInpatientBed("P6", "P999", "G1") == -4);
    CHECK(database.updateInpatientBed("P6", "P101", "G9") == -3);
    CHECK(database.updateInpatientBed("P6", "P101", "G2") == 1);
    CHECK(database.updateInpatientBed("P7", "P101", "G1") == 1);
    CHECK(database.dischargeInpatientFromQT("P6", date) == 1);
    CHECK(database.updateInpatientBed("P6", "P101", "G3") == -5);
    CHECK(database.dischargeInpatientFromQT("P6", date) == -3);
    CHECK(database.updateInpatientBed("P7", "P101", "G2") == 1);

    // Kiểm tra lưu và nạp lại số tiền lớn có phần lẻ không bị làm tròn sang e+06.
    std::string largeMedId;
    CHECK(database.addMedicineFromQT("B3", "High Precision Med", 1234567.0, "1 lo", 5, largeMedId) == 1);
    std::string largeSrvId;
    CHECK(database.addServiceFromQT("S3", "High Precision Srv", 2345678.0, largeSrvId) == 1);

    database.saveAllData();
    {
        std::ifstream medCsv("data/medicines.csv");
        std::string content((std::istreambuf_iterator<char>(medCsv)), std::istreambuf_iterator<char>());
        CHECK(content.find("1234567") != std::string::npos);
        CHECK(content.find("e+") == std::string::npos);
        std::ifstream srvCsv("data/data_services.csv");
        std::string srvContent((std::istreambuf_iterator<char>(srvCsv)), std::istreambuf_iterator<char>());
        CHECK(srvContent.find("2345678") != std::string::npos);
        CHECK(srvContent.find("e+") == std::string::npos);
    }

    Database reloaded;
    reloaded.loadAllData();
    CHECK(reloaded.findMedicineById(largeMedId) != nullptr);
    CHECK(std::llround(reloaded.findMedicineById(largeMedId)->getUnitPrice()) == 1234567);
    CHECK(reloaded.getServices().search(largeSrvId) != nullptr);
    CHECK(std::llround(reloaded.getServices().search(largeSrvId)->getPrice()) == 2345678);
    CHECK(reloaded.deleteMedicineFromQT(largeMedId) == 1);
    CHECK(reloaded.deleteServiceFromQT(largeSrvId));

    const Appointment* reloadedAppointment = reloaded.findAppointmentById(clsId);
    CHECK(reloadedAppointment != nullptr && reloadedAppointment->getStatus() == "Billed");
    CHECK(findInvoice(reloaded, clsId) != nullptr);
    CHECK(findInvoice(reloaded, bhytAppointmentId) != nullptr);
    const MedicalRecord* reloadedRecord = findRecord(reloaded, clsId);
    CHECK(reloadedRecord != nullptr);
    CHECK(reloadedRecord->getAssignedServices().size() == 2);
    CHECK(reloadedRecord->getPrescriptions().size() == 2);
    CHECK(reloadedRecord->getPrescriptions()[0].dispensed);
    CHECK(reloaded.findMedicineById("M1")->getStock() == 18);
    CHECK(reloaded.findMedicineById("M2")->getStock() == 19);
    CHECK(reloaded.findMedicineById("B1_001")->getStock() == 18);
    CHECK(reloaded.findPatientById("P6") != nullptr);
    CHECK(static_cast<Inpatient*>(reloaded.findPatientById("P6"))->getRoomNumber() ==
          "Da xuat vien");
    CHECK(static_cast<Inpatient*>(reloaded.findPatientById("P7"))->getRoomNumber() == "P101");

    // Kiểm tra quản lý tài khoản và phân tách mã lỗi (-2 vs -1)
    CHECK(reloaded.getUsers().search("ADMIN01") != nullptr);
    CHECK(reloaded.addUserFromQT("", "user1", "pass123", "User One", "Admin") == -2);
    CHECK(reloaded.addUserFromQT("U01", "bad user", "pass123", "User One", "Admin") == -2);
    CHECK(reloaded.addUserFromQT("U01", "u1", "pass123", "User One", "Admin") == -2);
    CHECK(reloaded.addUserFromQT("U01", "user1", "   ", "User One", "Admin") == -2);
    CHECK(reloaded.addUserFromQT("U01", "user1", "pass123", "   ", "Admin") == -2);
    CHECK(reloaded.addUserFromQT("U01", "user1", "pass123", "User One", "   ") == -2);
    CHECK(reloaded.addUserFromQT("ADMIN01", "admin_other", "pass123", "Dup Id", "Admin") == -2);
    CHECK(reloaded.addUserFromQT("RECEP01", "admin", "pass123", "Dup Username", "Receptionist") == -1);
    CHECK(reloaded.addUserFromQT("RECEP01", "recep1", "pass123", "Recep One", "Receptionist") == 1);
    CHECK(!reloaded.setUserActiveFromQT("ADMIN01", false));
    CHECK(!reloaded.deleteUserFromQT("ADMIN01"));
    CHECK(reloaded.addUserFromQT("ADMIN02", "admin2", "pass123", "Admin Two", "Admin") == 1);
    CHECK(reloaded.setUserActiveFromQT("ADMIN01", false));
    CHECK(!reloaded.setUserActiveFromQT("ADMIN02", false));
    CHECK(reloaded.deleteUserFromQT("ADMIN01"));
    CHECK(!reloaded.deleteUserFromQT("ADMIN02"));

    // Kiểm tra thêm/xóa bác sĩ cùng validation ngày sinh, SĐT, ca trực và chuẩn hóa thứ/ca không dấu
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "31/02/1990", "Nam", "0912345678",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thứ 3", "Chiều") == -1);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/2099", "Nam", "0912345678",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thứ 3", "Chiều") == -1);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "12345",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thứ 3", "Chiều") == -1);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "091234567a",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thứ 3", "Chiều") == -1);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thứ 9", "Chiều") == -1);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thứ 3", "Đêm") == -1);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                        "Khoa Nội tổng hợp", 5, "recep1", "pass123",
                                        "Thứ 3", "Chiều") == -2);
    CHECK(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                        "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                        "Thu 3", "Chieu") == 1);
    CHECK(reloaded.findDoctorById("D03") != nullptr);
    CHECK(reloaded.findDoctorById("D03")->getSchedule()[0].dayOfWeek == "Thứ 3");
    CHECK(reloaded.findDoctorById("D03")->getSchedule()[0].timeSlot == "Chiều");
    CHECK(!reloaded.deleteUserFromQT("D03"));
    CHECK(reloaded.deleteDoctorFromQT("NOPE") == -1);
    CHECK(reloaded.deleteDoctorFromQT("D01") == -2);
    CHECK(reloaded.deleteDoctorFromQT("D03") == 1);
    CHECK(reloaded.getUsers().search("D03") == nullptr);

    // Kiểm tra thêm/cộng dồn/xóa thuốc
    std::string tempId;
    CHECK(reloaded.addMedicineFromQT("B1", "Zero Price Med", 0.0, "1 vien", 5, tempId) == -2);
    CHECK(reloaded.addMedicineFromQT("B1", "Neg Price Med", -500.0, "1 vien", 5, tempId) == -2);
    CHECK(reloaded.addMedicineFromQT("B1", "Zero Qty Med", 1000.0, "1 vien", 0, tempId) == -2);
    std::string newMedId;
    CHECK(reloaded.addMedicineFromQT("B1", "Amoxicillin 500mg", 5000.0, "1 vien", 10, newMedId) == 1);
    std::string restockId;
    CHECK(reloaded.addMedicineFromQT("B1", "  amoxicillin 500mg  ", 5000.0, "1 vien", 5, restockId) == 2);
    CHECK(restockId == newMedId);
    CHECK(reloaded.findMedicineById(newMedId)->getStock() == 15);
    CHECK(reloaded.addMedicineFromQT("B2", "Amoxicillin 500mg", 5000.0, "1 vien", 5, tempId) == -3);
    CHECK(reloaded.addMedicineFromQT("B1", "Amoxicillin 500mg", 5000.0, "1 vien",
                                     std::numeric_limits<int>::max(), tempId) == -2);
    CHECK(reloaded.deleteMedicineFromQT("NOPE") == -1);
    CHECK(reloaded.deleteMedicineFromQT("M1") == -2);
    CHECK(reloaded.deleteMedicineFromQT(newMedId) == 1);

    // Kiểm tra thêm/trùng/xóa dịch vụ
    CHECK(reloaded.addServiceFromQT("S2", "Zero Price Service", 0.0, tempId) == -2);
    CHECK(reloaded.addServiceFromQT("S2", "Neg Price Service", -1000.0, tempId) == -2);
    std::string newSrvId;
    CHECK(reloaded.addServiceFromQT("S2", "Sieu Am Tim", 250000.0, newSrvId) == 1);
    std::string dupSrvId;
    CHECK(reloaded.addServiceFromQT("S2", "  sieu am tim ", 250000.0, dupSrvId) == -1);
    CHECK(dupSrvId == newSrvId);
    CHECK(!reloaded.deleteServiceFromQT("S2_001"));
    CHECK(!reloaded.deleteServiceFromQT("NOPE"));
    CHECK(reloaded.deleteServiceFromQT(newSrvId));

    // Kiểm tra thêm bệnh nhân (DOB, SĐT), tìm kiếm và trả mã -3 khi chưa hoàn tất khám
    CHECK(!reloaded.addPatientFromGUI("Bad Feb", "29/02/2023", "Nam", "0911223344", "Không", false, date));
    CHECK(!reloaded.addPatientFromGUI("Future Dob", "01/01/2099", "Nam", "0911223344", "Không", false, date));
    CHECK(!reloaded.addPatientFromGUI("Bad Phone", "01/01/1995", "Nam", "12345", "Không", false, date));
    CHECK(reloaded.addPatientFromGUI("Nguyen Van Test", "29/02/2000", "Nam", "0911223344", "Không", false, date));
    CHECK(reloaded.searchPatients("   ").size() == 8);
    CHECK(reloaded.searchPatients("VAN TEST").size() == 1);
    auto foundByPhone = reloaded.searchPatients("0911223344");
    CHECK(foundByPhone.size() == 1);
    const std::string newPatId = foundByPhone[0]->getId();

    CHECK(reloaded.bookAppointmentFromQT(newPatId, "D01", "RECEP01", "Khoa Nội tổng hợp",
                                         day, shift, date) == 1);
    const std::string pendingFlowApptId = reloaded.getAppointments().back().getAppointmentId();
    double feeCheck = 0.0;
    CHECK(reloaded.createInvoiceFromQT(newPatId, feeCheck) == -3);
    CHECK(reloaded.checkInAppointmentFromQT(pendingFlowApptId, date) == 1);
    CHECK(reloaded.createInvoiceFromQT(newPatId, feeCheck) == -3);
    MedicalRecord* pendingFlowRec = findRecord(reloaded, pendingFlowApptId);
    CHECK(pendingFlowRec != nullptr);
    pendingFlowRec->setDiagnosis("Preliminary");
    CHECK(reloaded.assignServiceFromQT(pendingFlowRec->getRecordId(), "S2_001") == 1);
    CHECK(!reloaded.updateMedicalRecordFromQT(pendingFlowApptId, "S", "Preliminary", "", "Waiting").empty());
    CHECK(reloaded.createInvoiceFromQT(newPatId, feeCheck) == -3);
}

void runProjectDataConsistencyTests() {
    if (!std::filesystem::exists("data/patients.csv") &&
        std::filesystem::exists("../data/patients.csv")) {
        std::filesystem::current_path("..");
    }
    CHECK(std::filesystem::exists("data/patients.csv"));

    Database realDb;
    realDb.loadAllData();

    CHECK(!realDb.getPatients().getAllPointers().empty());
    CHECK(!realDb.getInvoices().empty());

    std::set<std::string> seenAppointmentIds;
    for (const Appointment& appt : realDb.getAppointments()) {
        CHECK(!appt.getAppointmentId().empty());
        CHECK(seenAppointmentIds.insert(appt.getAppointmentId()).second);
        CHECK(realDb.findPatientById(appt.getPatientId()) != nullptr);
    }

    for (const MedicalRecord& rec : realDb.getMedicalRecords()) {
        CHECK(realDb.findAppointmentById(rec.getAppointmentId()) != nullptr);
    }

    for (const Invoice& inv : realDb.getInvoices()) {
        CHECK(realDb.findPatientById(inv.getPatientId()) != nullptr);
        if (!inv.getAppointmentId().empty()) {
            const Appointment* linkedAppt = realDb.findAppointmentById(inv.getAppointmentId());
            CHECK(linkedAppt != nullptr);
            CHECK(linkedAppt->getStatus() == "Billed");
        }
        if (inv.getId().rfind("INV", 0) == 0 && inv.getId().size() > 3) {
            int invNum = 0;
            try { invNum = std::stoi(inv.getId().substr(3)); } catch (...) {}
            if (invNum >= 1 && invNum <= 45) {
                CHECK(!inv.getAppointmentId().empty());
            }
        }
        for (const InvoiceDetail& detail : inv.getDetails()) {
            CHECK(detail.getServiceId().rfind("INV", 0) != 0);
        }
    }

    const auto roomMap = realDb.getAvailableRoomMap();
    CHECK(!roomMap.empty());
    for (const auto& [room, beds] : roomMap) {
        CHECK(room.size() == 4 && room[0] == 'P');
        CHECK(room[1] >= '1' && room[1] <= '5');
        const int roomNum = std::stoi(room.substr(2));
        CHECK(roomNum >= 1 && roomNum <= 10);
        for (const std::string& bed : beds) {
            CHECK(bed == "G1" || bed == "G2" || bed == "G3" || bed == "G4" || bed == "G5");
        }
    }

    // Kiểm tra xuất viện nội trú trên dữ liệu thực tế (chỉ trong bộ nhớ, không ghi file)
    CHECK(realDb.dischargeInpatientFromQT("BN00002", "20/11/2026") == 1);
}

int main() {
    runSyntheticWorkflowTests();
    runProjectDataConsistencyTests();
    std::cout << "All workflow checks passed\n";
    return 0;
}
