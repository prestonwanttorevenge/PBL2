#include "services/Database.h"
#include "utils/AppointmentQueue.h"

#include <cassert>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <limits>
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

int main() {
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

    return 0;
}
