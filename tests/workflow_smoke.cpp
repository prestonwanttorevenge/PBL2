#include "services/Database.h"
#include "utils/AppointmentQueue.h"

#include <cassert>
#include <chrono>
#include <filesystem>
#include <fstream>
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
              "D01;Doctor One;01/01/1980;Nam;0900000000;Khoa Nội;5\n");
        write("data_doctor_schedules.csv", "D01;Thứ 2;Sáng\n");
        write("patients.csv",
              "NgoaiTru;P1;Patient One;01/01/1990;Nam;0900000001;;No;01/01/2026;P101\n"
              "NgoaiTru;P2;Patient Two;01/01/1990;Nam;0900000002;;No;01/01/2026;P102\n"
              "NgoaiTru;P3;Patient Three;01/01/1990;Nam;0900000003;;No;01/01/2026;P103\n"
              "NgoaiTru;P4;Patient Four;01/01/1990;Nam;0900000004;;No;01/01/2026;P104\n");
        write("medicines.csv",
              "M1;Medicine One;1000;1 per day;20\n"
              "M2;Medicine Two;2000;2 per day;20\n");
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

}

int main() {
    TemporaryDataDirectory testData;
    Database database;
    database.loadAllData();

    const std::string date = "28/10/2026";
    const std::string day = "Thứ 2";
    const std::string shift = "Sáng";
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội", day, shift, date) == 1);
    assert(database.bookAppointmentFromQT("P2", "D01", "R01", "Khoa Nội", day, shift, date) == 1);
    assert(database.bookAppointmentFromQT("P1", "D01", "R01", "Khoa Nội", day, shift, date) == -5);

    const std::string firstId = database.getAppointments()[0].getAppointmentId();
    const std::string secondId = database.getAppointments()[1].getAppointmentId();
    assert(database.checkInAppointmentFromQT(firstId, date) == 1);
    assert(database.checkInAppointmentFromQT(secondId, date) == 1);
    assert(database.updateMedicalRecordFromQT(secondId, "Symptom", "Diagnosis", "", "Completed").empty());
    assert(!database.updateMedicalRecordFromQT(firstId, "Symptom", "Diagnosis", "", "Completed").empty());
    assert(database.updateMedicalRecordFromQT(secondId, "Symptom", "Diagnosis", "", "Completed").size() > 0);

    assert(database.bookAppointmentFromQT("P3", "D01", "R01", "Khoa Nội", day, shift, date) == 1);
    assert(database.bookAppointmentFromQT("P4", "D01", "R01", "Khoa Nội", day, shift, date) == 1);
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
    assert(database.cancelAppointmentFromQT(clsId) == -5);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    assert(database.prescribeMedicineFromQT(clsRecord->getRecordId(), "M2", "2 per day", 1) == 1);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    assert(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M1") == 1);
    assert(clsAppointment->getStatus() == "WaitingInventory");
    assert(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M1") == -4);
    assert(database.dispensePrescriptionFromQT(clsRecord->getRecordId(), "M2") == 1);
    assert(clsAppointment->getStatus() == "Completed");

    double invoiceTotal = 0.0;
    assert(database.createInvoiceFromQT("P3", invoiceTotal) == 1);
    assert(invoiceTotal > 0.0);
    assert(database.getInvoices().back().getAppointmentId() == clsId);
    database.saveAllData();

    Database reloaded;
    reloaded.loadAllData();
    const Appointment* reloadedAppointment = reloaded.findAppointmentById(clsId);
    assert(reloadedAppointment != nullptr && reloadedAppointment->getStatus() == "Billed");
    assert(reloaded.getInvoices().back().getAppointmentId() == clsId);
    const MedicalRecord* reloadedRecord = findRecord(reloaded, clsId);
    assert(reloadedRecord != nullptr);
    assert(reloadedRecord->getAssignedServices().size() == 2);
    assert(reloadedRecord->getPrescriptions().size() == 2);
    assert(reloadedRecord->getPrescriptions()[0].dispensed);

    return 0;
}
