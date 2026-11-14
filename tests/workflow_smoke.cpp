#include "services/Database.h"
#include "utils/AppointmentQueue.h"

#include <cassert>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <limits>
#include <set>
#include <string>

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
        assert(file.is_open());
        file << content;
        assert(file.good());
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
    assert(localNow != nullptr);
    std::tm nextMonday = *localNow;
    int daysUntilMonday = (8 - nextMonday.tm_wday) % 7;
    if (daysUntilMonday == 0) daysUntilMonday = 7;
    nextMonday.tm_mday += daysUntilMonday;
    nextMonday.tm_hour = 12;
    nextMonday.tm_min = 0;
    nextMonday.tm_sec = 0;
    const std::time_t timestamp = std::mktime(&nextMonday);
    assert(timestamp != static_cast<std::time_t>(-1));
    char formattedDate[11];
    assert(std::strftime(formattedDate, sizeof(formattedDate), "%d/%m/%Y",
                         std::localtime(&timestamp)) > 0);
    return formattedDate;
}

}

void runSyntheticWorkflowTests() {
    TemporaryDataDirectory testData;
    Database database;
    database.loadAllData();

    const std::string date = nextMondayDate();
    const std::string day = "Thứ 2";
    const std::string shift = "Sáng";
    assert(database.addDoctorWithAccount("D02", "Doctor Two", "01/01/1980", "Nam",
                                         "0900000002", "Khoa Khám bệnh", 5,
                                         "doctor2", "password", day, shift) == -1);
    assert(database.findDoctorById("D02") == nullptr);
    std::string invalidId;
    assert(database.addMedicineFromQT("INVALID", "Invalid Medicine", 100.0,
                                      "1 per day", 1, invalidId) == -2);
    assert(database.addMedicineFromQT("B1", "Invalid Price",
                                      std::numeric_limits<double>::quiet_NaN(),
                                      "1 per day", 1, invalidId) == -2);
    assert(database.addServiceFromQT("INVALID", "Invalid Service", 100.0,
                                     invalidId) == -2);
    assert(database.addServiceFromQT("S1", "Invalid Price",
                                     std::numeric_limits<double>::quiet_NaN(),
                                     invalidId) == -2);
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Khám bệnh",
                                          day, shift, date) == -3);
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp",
                                          day, shift, "28/10/2026") == -3);
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp",
                                          day, shift, "31/02/2026") == -3);
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp",
                                          "Thứ 7", shift, "01/01/2000") == -3);
    assert(database.getAppointments().empty());
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    assert(database.bookAppointmentFromQT("P2", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == -5);

    const std::string firstId = database.getAppointments()[0].getAppointmentId();
    const std::string secondId = database.getAppointments()[1].getAppointmentId();
    assert(database.checkInAppointmentFromQT(firstId, date) == 1);
    assert(database.checkInAppointmentFromQT(secondId, date) == 1);
    assert(database.updateMedicalRecordFromQT(secondId, "Symptom", "Diagnosis", "", "Completed").empty());
    assert(!database.updateMedicalRecordFromQT(firstId, "Symptom", "Diagnosis", "", "Completed").empty());
    assert(database.updateMedicalRecordFromQT(secondId, "Symptom", "Diagnosis", "", "Completed").size() > 0);

    assert(database.bookAppointmentFromQT("P3", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    assert(database.bookAppointmentFromQT("P4", "D01", "R01", "Khoa Nội tổng hợp", day, shift, date) == 1);
    const std::string clsId = database.getAppointments()[2].getAppointmentId();
    const std::string nextId = database.getAppointments()[3].getAppointmentId();
    assert(database.checkInAppointmentFromQT(clsId, date) == 1);
    assert(database.checkInAppointmentFromQT(nextId, date) == 1);

    Appointment* clsAppointment = database.findAppointmentById(clsId);
    Appointment* nextAppointment = database.findAppointmentById(nextId);
    assert(clsAppointment != nullptr && nextAppointment != nullptr);
    assert(isNextPendingAppointment(database.getAppointments(), *clsAppointment));
    assert(!isNextPendingAppointment(database.getAppointments(), *nextAppointment));

    MedicalRecord* clsRecord = findRecord(database, clsId);
    assert(clsRecord != nullptr);
    clsRecord->setSymptoms("Symptom");
    clsRecord->setDiagnosis("Diagnosis");
    clsRecord->setNotes("Notes");
    assert(database.assignServiceFromQT(clsRecord->getRecordId(), "S2_001") == 1);
    assert(database.assignServiceFromQT(clsRecord->getRecordId(), "S2_002") == 1);
    assert(clsAppointment->getStatus() == "Pending");
    assert(database.updateMedicalRecordFromQT(clsId, "Symptom", "Diagnosis", "Notes", "Waiting") ==
           clsRecord->getRecordId());
    assert(clsAppointment->getStatus() == "Waiting");
    assert(isNextPendingAppointment(database.getAppointments(), *nextAppointment));

    assert(!database.updateMedicalRecordFromQT(nextId, "Symptom", "Diagnosis", "", "Completed").empty());
    assert(database.prescribeMedicineFromQT(clsRecord->getRecordId(), "M1", "1 per day", 2) == 1);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    double invoiceTotal = 123.0;
    assert(database.createInvoiceFromQT("P3", invoiceTotal) == -5);
    assert(invoiceTotal == 0.0);
    assert(database.cancelAppointmentFromQT(clsId) == -5);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    assert(database.prescribeMedicineFromQT(clsRecord->getRecordId(), "M2", "2 per day", 1) == 1);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    assert(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M1") == 1);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    assert(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M1") == -4);
    assert(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M2") == 1);
    assert(clsAppointment->getStatus() == "Completed");
    assert(database.findMedicineById("M1")->getStock() == 18);
    assert(database.findMedicineById("M2")->getStock() == 19);

    invoiceTotal = 0.0;
    assert(database.createInvoiceFromQT("P3", invoiceTotal) == 1);
    assert(invoiceTotal == 229000.0);
    assert(database.getInvoices().back().getAppointmentId() == clsId);
    database.saveAllData();

    assert(database.bookAppointmentFromQT("P5", "D01", "R01", "Khoa Nội tổng hợp",
                                          day, shift, date) == 1);
    const std::string bhytAppointmentId = database.getAppointments().back().getAppointmentId();
    assert(database.checkInAppointmentFromQT(bhytAppointmentId, date) == 1);
    MedicalRecord* bhytRecord = findRecord(database, bhytAppointmentId);
    assert(bhytRecord != nullptr);
    bhytRecord->setDiagnosis("Diagnosis");
    assert(database.assignServiceFromQT(bhytRecord->getRecordId(), "S2_001") == 1);
    assert(database.updateMedicalRecordFromQT(bhytAppointmentId, "Symptom", "Diagnosis",
                                              "", "Waiting") == bhytRecord->getRecordId());
    assert(database.updateMedicalRecordFromQT(bhytAppointmentId, "Symptom", "Diagnosis",
                                              "", "Completed") == bhytRecord->getRecordId());
    assert(database.prescribeMedicineFromQT(bhytRecord->getRecordId(), "B1_001",
                                            "1 per day", 2) == 1);
    assert(database.dispensePrescriptionFromQT(bhytRecord->getRecordId(), "B1_001") == 1);
    assert(database.createInvoiceFromQT("P5", invoiceTotal) == 1);
    assert(invoiceTotal == 15000.0);

    assert(database.updateInpatientBed("P6", "P101", "G1") == 1);
    assert(database.updateInpatientBed("P7", "P101", "G1") == -3);
    assert(database.updateInpatientBed("P6", "P999", "G1") == -4);
    assert(database.updateInpatientBed("P6", "P101", "G9") == -3);
    assert(database.updateInpatientBed("P6", "P101", "G2") == 1);
    assert(database.updateInpatientBed("P7", "P101", "G1") == 1);
    assert(database.dischargeInpatientFromQT("P6", date) == 1);
    assert(database.updateInpatientBed("P6", "P101", "G3") == -5);
    assert(database.dischargeInpatientFromQT("P6", date) == -3);
    assert(database.updateInpatientBed("P7", "P101", "G2") == 1);

    database.saveAllData();
    Database reloaded;
    reloaded.loadAllData();
    const Appointment* reloadedAppointment = reloaded.findAppointmentById(clsId);
    assert(reloadedAppointment != nullptr && reloadedAppointment->getStatus() == "Billed");
    assert(findInvoice(reloaded, clsId) != nullptr);
    assert(findInvoice(reloaded, bhytAppointmentId) != nullptr);
    const MedicalRecord* reloadedRecord = findRecord(reloaded, clsId);
    assert(reloadedRecord != nullptr);
    assert(reloadedRecord->getAssignedServices().size() == 2);
    assert(reloadedRecord->getPrescriptions().size() == 2);
    assert(reloadedRecord->getPrescriptions()[0].dispensed);
    assert(reloaded.findMedicineById("M1")->getStock() == 18);
    assert(reloaded.findMedicineById("M2")->getStock() == 19);
    assert(reloaded.findMedicineById("B1_001")->getStock() == 18);
    assert(reloaded.findPatientById("P6") != nullptr);
    assert(static_cast<Inpatient*>(reloaded.findPatientById("P6"))->getRoomNumber() ==
           "Da xuat vien");
    assert(static_cast<Inpatient*>(reloaded.findPatientById("P7"))->getRoomNumber() == "P101");

    // Kiểm tra quản lý tài khoản và phân tách mã lỗi (-2 vs -1)
    assert(reloaded.getUsers().search("ADMIN01") != nullptr);
    assert(reloaded.addUserFromQT("", "user1", "pass123", "User One", "Admin") == -2);
    assert(reloaded.addUserFromQT("U01", "bad user", "pass123", "User One", "Admin") == -2);
    assert(reloaded.addUserFromQT("U01", "u1", "pass123", "User One", "Admin") == -2);
    assert(reloaded.addUserFromQT("U01", "user1", "   ", "User One", "Admin") == -2);
    assert(reloaded.addUserFromQT("U01", "user1", "pass123", "   ", "Admin") == -2);
    assert(reloaded.addUserFromQT("U01", "user1", "pass123", "User One", "   ") == -2);
    assert(reloaded.addUserFromQT("ADMIN01", "admin_other", "pass123", "Dup Id", "Admin") == -2);
    assert(reloaded.addUserFromQT("RECEP01", "admin", "pass123", "Dup Username", "Receptionist") == -1);
    assert(reloaded.addUserFromQT("RECEP01", "recep1", "pass123", "Recep One", "Receptionist") == 1);
    assert(!reloaded.setUserActiveFromQT("ADMIN01", false));
    assert(!reloaded.deleteUserFromQT("ADMIN01"));
    assert(reloaded.addUserFromQT("ADMIN02", "admin2", "pass123", "Admin Two", "Admin") == 1);
    assert(reloaded.setUserActiveFromQT("ADMIN01", false));
    assert(!reloaded.setUserActiveFromQT("ADMIN02", false));
    assert(reloaded.deleteUserFromQT("ADMIN01"));
    assert(!reloaded.deleteUserFromQT("ADMIN02"));

    // Kiểm tra thêm/xóa bác sĩ cùng validation ngày sinh, SĐT, ca trực
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "31/02/1990", "Nam", "0912345678",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 3", "Chiều") == -1);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/2099", "Nam", "0912345678",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 3", "Chiều") == -1);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "12345",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 3", "Chiều") == -1);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "091234567a",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 3", "Chiều") == -1);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 9", "Chiều") == -1);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 3", "Đêm") == -1);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                         "Khoa Nội tổng hợp", 5, "recep1", "pass123",
                                         "Thứ 3", "Chiều") == -2);
    assert(reloaded.addDoctorWithAccount("D03", "Doctor Three", "01/01/1990", "Nam", "0912345678",
                                         "Khoa Nội tổng hợp", 5, "doctor3", "pass123",
                                         "Thứ 3", "Chiều") == 1);
    assert(!reloaded.deleteUserFromQT("D03"));
    assert(reloaded.deleteDoctorFromQT("NOPE") == -1);
    assert(reloaded.deleteDoctorFromQT("D01") == -2);
    assert(reloaded.deleteDoctorFromQT("D03") == 1);
    assert(reloaded.getUsers().search("D03") == nullptr);

    // Kiểm tra thêm/cộng dồn/xóa thuốc
    std::string tempId;
    assert(reloaded.addMedicineFromQT("B1", "Zero Price Med", 0.0, "1 vien", 5, tempId) == -2);
    assert(reloaded.addMedicineFromQT("B1", "Neg Price Med", -500.0, "1 vien", 5, tempId) == -2);
    assert(reloaded.addMedicineFromQT("B1", "Zero Qty Med", 1000.0, "1 vien", 0, tempId) == -2);
    std::string newMedId;
    assert(reloaded.addMedicineFromQT("B1", "Amoxicillin 500mg", 5000.0, "1 vien", 10, newMedId) == 1);
    std::string restockId;
    assert(reloaded.addMedicineFromQT("B1", "  amoxicillin 500mg  ", 5000.0, "1 vien", 5, restockId) == 2);
    assert(restockId == newMedId);
    assert(reloaded.findMedicineById(newMedId)->getStock() == 15);
    assert(reloaded.addMedicineFromQT("B2", "Amoxicillin 500mg", 5000.0, "1 vien", 5, tempId) == -3);
    assert(reloaded.addMedicineFromQT("B1", "Amoxicillin 500mg", 5000.0, "1 vien",
                                      std::numeric_limits<int>::max(), tempId) == -2);
    assert(reloaded.deleteMedicineFromQT("NOPE") == -1);
    assert(reloaded.deleteMedicineFromQT("M1") == -2);
    assert(reloaded.deleteMedicineFromQT(newMedId) == 1);

    // Kiểm tra thêm/trùng/xóa dịch vụ
    assert(reloaded.addServiceFromQT("S2", "Zero Price Service", 0.0, tempId) == -2);
    assert(reloaded.addServiceFromQT("S2", "Neg Price Service", -1000.0, tempId) == -2);
    std::string newSrvId;
    assert(reloaded.addServiceFromQT("S2", "Sieu Am Tim", 250000.0, newSrvId) == 1);
    std::string dupSrvId;
    assert(reloaded.addServiceFromQT("S2", "  sieu am tim ", 250000.0, dupSrvId) == -1);
    assert(dupSrvId == newSrvId);
    assert(!reloaded.deleteServiceFromQT("S2_001"));
    assert(!reloaded.deleteServiceFromQT("NOPE"));
    assert(reloaded.deleteServiceFromQT(newSrvId));

    // Kiểm tra thêm bệnh nhân (DOB, SĐT), tìm kiếm và trả mã -3 khi chưa hoàn tất khám
    assert(!reloaded.addPatientFromGUI("Bad Feb", "29/02/2023", "Nam", "0911223344", "Không", false, date));
    assert(!reloaded.addPatientFromGUI("Future Dob", "01/01/2099", "Nam", "0911223344", "Không", false, date));
    assert(!reloaded.addPatientFromGUI("Bad Phone", "01/01/1995", "Nam", "12345", "Không", false, date));
    assert(reloaded.addPatientFromGUI("Nguyen Van Test", "29/02/2000", "Nam", "0911223344", "Không", false, date));
    assert(reloaded.searchPatients("   ").size() == 8);
    assert(reloaded.searchPatients("VAN TEST").size() == 1);
    auto foundByPhone = reloaded.searchPatients("0911223344");
    assert(foundByPhone.size() == 1);
    const std::string newPatId = foundByPhone[0]->getId();

    assert(reloaded.bookAppointmentFromQT(newPatId, "D01", "RECEP01", "Khoa Nội tổng hợp",
                                          day, shift, date) == 1);
    const std::string pendingFlowApptId = reloaded.getAppointments().back().getAppointmentId();
    double feeCheck = 0.0;
    assert(reloaded.createInvoiceFromQT(newPatId, feeCheck) == -3);
    assert(reloaded.checkInAppointmentFromQT(pendingFlowApptId, date) == 1);
    assert(reloaded.createInvoiceFromQT(newPatId, feeCheck) == -3);
    MedicalRecord* pendingFlowRec = findRecord(reloaded, pendingFlowApptId);
    assert(pendingFlowRec != nullptr);
    pendingFlowRec->setDiagnosis("Preliminary");
    assert(reloaded.assignServiceFromQT(pendingFlowRec->getRecordId(), "S2_001") == 1);
    assert(!reloaded.updateMedicalRecordFromQT(pendingFlowApptId, "S", "Preliminary", "", "Waiting").empty());
    assert(reloaded.createInvoiceFromQT(newPatId, feeCheck) == -3);
}

void runProjectDataConsistencyTests() {
    if (!std::filesystem::exists("data/patients.csv") &&
        std::filesystem::exists("../data/patients.csv")) {
        std::filesystem::current_path("..");
    }
    assert(std::filesystem::exists("data/patients.csv"));

    Database realDb;
    realDb.loadAllData();

    assert(!realDb.getPatients().getAllPointers().empty());
    assert(!realDb.getInvoices().empty());

    std::set<std::string> seenAppointmentIds;
    for (const Appointment& appt : realDb.getAppointments()) {
        assert(!appt.getAppointmentId().empty());
        assert(seenAppointmentIds.insert(appt.getAppointmentId()).second);
        assert(realDb.findPatientById(appt.getPatientId()) != nullptr);
    }

    for (const MedicalRecord& rec : realDb.getMedicalRecords()) {
        assert(realDb.findAppointmentById(rec.getAppointmentId()) != nullptr);
    }

    for (const Invoice& inv : realDb.getInvoices()) {
        assert(realDb.findPatientById(inv.getPatientId()) != nullptr);
        if (!inv.getAppointmentId().empty()) {
            const Appointment* linkedAppt = realDb.findAppointmentById(inv.getAppointmentId());
            assert(linkedAppt != nullptr);
            assert(linkedAppt->getStatus() == "Billed");
        }
        if (inv.getId().rfind("INV", 0) == 0 && inv.getId().size() > 3) {
            int invNum = 0;
            try { invNum = std::stoi(inv.getId().substr(3)); } catch (...) {}
            if (invNum >= 1 && invNum <= 45) {
                assert(!inv.getAppointmentId().empty());
            }
        }
        for (const InvoiceDetail& detail : inv.getDetails()) {
            assert(detail.getServiceId().rfind("INV", 0) != 0);
        }
    }

    const auto roomMap = realDb.getAvailableRoomMap();
    assert(!roomMap.empty());
    for (const auto& [room, beds] : roomMap) {
        assert(room.size() == 4 && room[0] == 'P');
        assert(room[1] >= '1' && room[1] <= '5');
        const int roomNum = std::stoi(room.substr(2));
        assert(roomNum >= 1 && roomNum <= 10);
        for (const std::string& bed : beds) {
            assert(bed == "G1" || bed == "G2" || bed == "G3" || bed == "G4" || bed == "G5");
        }
    }

    // Kiểm tra xuất viện nội trú trên dữ liệu thực tế (chỉ trong bộ nhớ, không ghi file)
    assert(realDb.dischargeInpatientFromQT("BN00002", "20/11/2026") == 1);
}

int main() {
    runSyntheticWorkflowTests();
    runProjectDataConsistencyTests();
    return 0;
}
