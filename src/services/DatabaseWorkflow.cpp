#include "services/Database.h"
#include "utils/AppointmentQueue.h"
#include <cstdio>
#include <string>
#include <vector>
#include <map>

bool Database::addPatientFromGUI(string name, string dob, string gender, string phone,
                                 string BH, bool isNoiTru, string admissionDate) {
    if (name.empty() || dob.empty() || gender.empty() || phone.size() < 10 || phone.size() > 11) {
        return false;
    }
    for (char digit : phone) {
        if (digit < '0' || digit > '9') return false;
    }
    string newId = generatePatientId();
    if (isNoiTru){
        patients.insert(newId, new Inpatient(newId, name, dob, gender, phone, "", BH,
                                             admissionDate, "", "Chua xep", ""));
    } else{
        patients.insert(newId, new Outpatient(newId, name, dob, gender, phone, "", BH, "", ""));
    }
    return true;
}

int Database::bookAppointmentFromQT(string patientId, string doctorId, string receptionistId,
                                    string specialty, string day, string time, string appointmentDate) {
    if (findPatientById(patientId) == nullptr) return -1; 
    Doctor* doc = findDoctorById(doctorId);
    if (doc == nullptr) return -2; 
    if (appointmentDate.empty() || specialty.empty() || doc->getSpecialty() != specialty) return -3;

    bool isAvailable = false;
    for (const auto& shift : doc->getSchedule()) {
        if (shift.dayOfWeek == day && shift.timeSlot == time) {
            isAvailable = true;
            break;
        }
    }
    if (!isAvailable) return -3;

    for (const Appointment& appt : appointments) {
        if (appt.getAppointmentDate() == appointmentDate &&
            appt.getTimeSlot() == time &&
            appt.getStatus() != "Canceled" &&
            appt.getStatus() != "Cancelled") {
            if (appt.getPatientId() == patientId) return -5;
        }
    }

    string newAppId = generateAppointmentId(); 
    appointments.push_back(Appointment(newAppId, patientId, doctorId, receptionistId,
                                       specialty, day, time, "Scheduled", appointmentDate));

    string newRecordId = generateMedicalRecordId(); 
    medicalRecords.push_back(MedicalRecord(newRecordId, newAppId, "", "", ""));

    saveAllData();
    return 1; 
}

int Database::checkInAppointmentFromQT(const string& appointmentId, const string& currentDate) {
    Appointment* appointment = findAppointmentById(appointmentId);
    if (appointment == nullptr) return -1;
    if (appointment->getStatus() != "Scheduled") return -2;
    // Chỉ check-in đúng ngày đã đặt.
    if (appointment->getAppointmentDate() != currentDate) return -3;
    appointment->setStatus("Pending");
    saveAllData();
    return 1;
}

string Database::updateMedicalRecordFromQT(string apptId, string symp, string diag, string notes, string status) {
    Appointment* appt = findAppointmentById(apptId);
    if (appt == nullptr) return "";
    if (appt->getStatus() == "Pending" && !isNextPendingAppointment(appointments, *appt)) return "";
    if (diag.empty() || (status != "Completed" && status != "Waiting") ||
        appt->getStatus() == "Canceled" || appt->getStatus() == "Cancelled" ||
        appt->getStatus() == "Billed") return "";

    for (auto& record : medicalRecords) {
        if (record.getAppointmentId() == apptId) {
            if (status == "Waiting" && record.getAssignedServices().empty()) return "";
            record.setSymptoms(symp);
            record.setDiagnosis(diag);
            record.setNotes(notes);

            appt->setStatus(status);
            saveAllData(); 
            return record.getRecordId(); 
        }
    }
    return "";
}

// ==========================================
// CÁC HÀM ĐĂNG NHẬP VÀ THÊM DỮ LIỆU KHÁC
// ==========================================
int Database::assignServiceFromQT(string recordId, string serviceId) {
    MedicalRecord* record = findMedicalRecordById(recordId);
    if (record == nullptr) return -1;
    if (services.search(serviceId) == nullptr) return -2;
    Appointment* appointment = findAppointmentById(record->getAppointmentId());
    if (appointment == nullptr || appointment->getStatus() == "Canceled" ||
        appointment->getStatus() == "Cancelled" || appointment->getStatus() == "Billed" ||
        (appointment->getStatus() != "Pending" && appointment->getStatus() != "Waiting" &&
         appointment->getStatus() != "Completed")) return -4;
    if (record->getDiagnosis().empty()) return -5;
    if (!record->getPrescriptions().empty()) return -4;
    for (const Invoice& invoice : invoices) {
        if (invoice.getAppointmentId() == appointment->getAppointmentId()) return -4;
    }

    for (const string& assignedService : record->getAssignedServices()) {
        if (assignedService == serviceId) return -3;
    }

    record->addService(serviceId);
    appointment->setStatus("Waiting");
    return 1;
}

double Database::calculateTotalRevenue() {
    double total = 0.0;
    for (const Invoice& invoice : invoices) {
        total += invoice.getTotalAmount();
    }
    return total;
}

vector<Doctor*> Database::getAvailableDoctors(string day, string shift) {
    vector<Doctor*> availableDocs;
    vector<Doctor*> allDocs = doctors.getAllPointers();
    for (size_t i = 0; i < allDocs.size(); i++) {
        for (const auto& w : allDocs[i]->getSchedule()) {
            if (w.dayOfWeek == day && w.timeSlot == shift) {
                availableDocs.push_back(allDocs[i]);
                break;
            }
        }
    }
    return availableDocs;
}

int Database::prescribeMedicineFromQT(string recordId, string medId, string dosage, int qty) {
    MedicalRecord* record = findMedicalRecordById(recordId);
    if (record == nullptr) return -1;
    Medicine* med = findMedicineById(medId);
    if (med == nullptr) return -2;
    if (qty <= 0 || dosage.empty()) return -4;
    if (med->getStock() < qty) return -3;
    Appointment* appointment = findAppointmentById(record->getAppointmentId());
    if (appointment == nullptr || appointment->getStatus() != "Completed") return -5;
    if (record->hasMedicine(medId)) return -6;
    record->addMedicine(medId, dosage, qty, med->getUnitPrice(), false);
    return 1;
}

int Database::dispensePrescriptionFromQT(string recordId, string medicineId) {
    MedicalRecord* record = findMedicalRecordById(recordId);
    if (record == nullptr) return -1;
    Appointment* appointment = findAppointmentById(record->getAppointmentId());
    if (appointment == nullptr || appointment->getStatus() != "Completed") return -2;
    if (!record->hasMedicine(medicineId)) return -5;
    if (record->isMedicineDispensed(medicineId)) return -4;

    Medicine* medicine = findMedicineById(medicineId);
    if (medicine == nullptr) return -5;
    int quantity = 0;
    for (const PrescriptionDetail& prescription : record->getPrescriptions()) {
        if (prescription.medicineId == medicineId) {
            quantity = prescription.quantity;
            break;
        }
    }
    if (quantity <= 0 || medicine->getStock() < quantity) return -3;
    if (!record->markMedicineDispensed(medicineId)) return -4;

    // Chỉ trừ kho sau khi cờ cấp phát được đặt thành công.
    medicine->setStock(medicine->getStock() - quantity);
    saveAllData();
    return 1;
}

int Database::createInvoiceFromQT(string patientId, double& outTotalFee) {
    outTotalFee = 0.0;
    Patient* pat = findPatientById(patientId);
    if (pat == nullptr) return -1;

    Appointment* targetAppointment = nullptr;
    bool hasAlreadyBilledVisit = false;
    for (auto it = appointments.rbegin(); it != appointments.rend(); ++it) {
        if (it->getPatientId() != patientId) continue;
        if (it->getStatus() == "Billed") {
            hasAlreadyBilledVisit = true;
            continue;
        }
        if (it->getStatus() == "Completed") {
            targetAppointment = &*it;
            break;
        }
    }
    if (targetAppointment == nullptr) return hasAlreadyBilledVisit ? -4 : -2;
    // Mỗi lượt khám chỉ được gắn với một hóa đơn.
    for (const Invoice& invoice : invoices) {
        if (invoice.getAppointmentId() == targetAppointment->getAppointmentId()) return -4;
    }

    MedicalRecord* record = nullptr;
    for (size_t i = 0; i < medicalRecords.size(); i++) {
        if (medicalRecords[i].getAppointmentId() == targetAppointment->getAppointmentId()) {
            record = &medicalRecords[i];
            break;
        }
    }
    if (record == nullptr || record->getDiagnosis().empty()) return -3;

    bool isBHYT = pat->hasBHYT();
    double finalMedFee = 0.0;
    for (const auto& rx : record->getPrescriptions()) {
        Medicine* med = findMedicineById(rx.medicineId);
        if (med == nullptr || !rx.dispensed) return -5;
        {
            double rawLinePrice = rx.unitPrice * rx.quantity;
            if (isBHYT) {
                string mId = med->getMedicineId();
                string prefix = (mId.length() >= 2) ? mId.substr(0, 2) : "";
                if (prefix == "B1") {
                    continue;
                } else if (prefix == "B2") {
                    finalMedFee += rawLinePrice * 0.5;
                } else if (prefix == "B3") {
                    finalMedFee += rawLinePrice * 0.2;
                } else {
                    finalMedFee += rawLinePrice;
                }
            } else {
                finalMedFee += rawLinePrice;
            }
        }
    }

    int nextInvoiceNumber = 1;
    string newInvId;
    bool idExists = true;
    while (idExists) {
        newInvId = "INV" + to_string(nextInvoiceNumber++);
        idExists = false;
        for (const Invoice& invoice : invoices) {
            if (invoice.getId() == newInvId) {
                idExists = true;
                break;
            }
        }
    }

    Invoice newInv(newInvId, patientId, finalMedFee, targetAppointment->getAppointmentId());
    vector<Service*> srvList = services.getAllPointers();

    vector<string> serviceIds = record->getAssignedServices();
    bool hasConsultationService = false;
    for (const string& serviceId : serviceIds) {
        if (serviceId.rfind("S1", 0) == 0) {
            hasConsultationService = true;
            break;
        }
    }

    string consultationServiceId;
    if (services.search("S1_001") != nullptr) {
        consultationServiceId = "S1_001";
    } else {
        for (Service* service : srvList) {
            if (service->getId().rfind("S1", 0) == 0) {
                consultationServiceId = service->getId();
                break;
            }
        }
    }

    if (!hasConsultationService && !consultationServiceId.empty()) {
        serviceIds.insert(serviceIds.begin(), consultationServiceId);
    }
    if (serviceIds.empty()) {
        serviceIds.push_back(consultationServiceId.empty() ? "S1_001" : consultationServiceId);
    }

    int detailNumber = 1;
    for (const string& serviceId : serviceIds) {
        Service* service = services.search(serviceId);
        const double listedPrice = service == nullptr ? 100000.0 : service->getPrice();
        const string prefix = serviceId.substr(0, serviceId.find('_'));
        double patientPrice = listedPrice;
        if (isBHYT && prefix == "S1") {
            patientPrice = 0.0;
        } else if (isBHYT && prefix == "S2") {
            patientPrice = listedPrice * 0.3;
        } else if (isBHYT && prefix == "S3") {
            patientPrice = listedPrice * 0.8;
        }

        const string detailId = newInvId + "_D" + to_string(detailNumber++);
        newInv.addDetail(InvoiceDetail(detailId, serviceId, newInvId, 1, patientPrice));
    }

    outTotalFee = newInv.getTotalAmount();
    invoices.push_back(newInv);
    targetAppointment->setStatus("Billed");
    saveAllData();
    return 1;
}


vector<MedicalRecord*> Database::getPatientHistory(string patientId) {
    vector<MedicalRecord*> history;
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].getPatientId() == patientId) {
            string apptId = appointments[i].getAppointmentId();
            for (size_t j = 0; j < medicalRecords.size(); j++) {
                if (medicalRecords[j].getAppointmentId() == apptId) {
                    history.push_back(&medicalRecords[j]);
                    break;
                }
            }
        }
    }
    return history;
}

int Database::cancelAppointmentFromQT(string apptId) {
    Appointment* appt = findAppointmentById(apptId);
    if (appt == nullptr) return -1; 
    if (appt->getStatus() == "Completed" || appt->getStatus() == "Billed") return -2;
    if (appt->getStatus() == "Waiting") return -3;
    if (appt->getStatus() == "Canceled" || appt->getStatus() == "Cancelled") return -4;
    appt->setStatus("Canceled");
    saveAllData(); 
    return 1; 
}

// ==========================================
// CÁC HÀM XÓA/CẬP NHẬT CLI (GIỮ NGUYÊN)
// ==========================================
map<string, vector<string>> Database::getAvailableRoomMap() {
    CustomRoomMap roomBeds;

    // Khởi tạo toàn bộ phòng trống
    const vector<string> standardBeds = {"G1", "G2", "G3", "G4", "G5"};
    for (int f = 1; f <= 5; f++) {
        for (int r = 1; r <= 10; r++) {
            char buffer[10];
            sprintf(buffer, "P%d%02d", f, r);
            roomBeds.insert(string(buffer), standardBeds);
        }
    }

    // Quét qua danh sách bệnh nhân nội trú để trừ đi giường đã có người
    vector<Patient**> allPats = patients.getAllPointers();
    for (auto pPtr : allPats) {
        if (Inpatient* inpat = dynamic_cast<Inpatient*>(*pPtr)) {
            string r = inpat->getRoomNumber();
            string b = inpat->getBedNumber();

            if (!r.empty() && r != "Chua xep" && r != "Da xuat vien" && !roomBeds.contains(r)) {
                roomBeds.insert(r, standardBeds);
            }
            if (!r.empty() && r != "Chua xep" && r != "Da xuat vien") {
                roomBeds.removeBedFromRoom(r, b); // Trừ đi giường b khỏi phòng r
            }
        }
    }

    // Trả về danh sách những phòng còn trống dưới dạng mảng cấu trúc RoomInfo
    map<string, vector<string>> availableRooms;
    for (const RoomInfo& room : roomBeds.getValidRooms()) {
        availableRooms.emplace(room.first, room.second);
    }
    return availableRooms;
}

int Database::updateInpatientBed(string patientId, string room, string bed) {
    Patient* pat = findPatientById(patientId);
    if (!pat) return -1; 

    Inpatient* inpat = dynamic_cast<Inpatient*>(pat);
    if (!inpat) return -2; 
    if (inpat->getRoomNumber() == "Da xuat vien") return -5;
    if (inpat->getRoomNumber() == room && inpat->getBedNumber() == bed) {
        return 1;
    }

    bool roomAvailable = false;
    bool bedAvailable = false;
    for (const auto& roomInfo : getAvailableRoomMap()) {
        if (roomInfo.first == room) {
            roomAvailable = true;
            for (const string& availableBed : roomInfo.second) {
                if (availableBed == bed) {
                    bedAvailable = true;
                    break;
                }
            }
            break;
        }
    }
    if (!roomAvailable) return -4;
    if (!bedAvailable) return -3;

    inpat->setRoomNumber(room);
    inpat->setBedNumber(bed);
    return 1;
}

int Database::dischargeInpatientFromQT(string patientId, string dischargeDate) {
    Patient* pat = findPatientById(patientId);
    if (!pat) return -1;

    Inpatient* inpat = dynamic_cast<Inpatient*>(pat);
    if (!inpat) return -2; 
    if (inpat->getRoomNumber() == "Da xuat vien" || inpat->getRoomNumber().empty() || inpat->getRoomNumber() == "Chua xep") {
        return -3; 
    }
    for (const Appointment& appointment : appointments) {
        if (appointment.getPatientId() == patientId &&
            (appointment.getStatus() == "Pending" || appointment.getStatus() == "Waiting")) {
            return -4;
        }
    }

    inpat->setRoomNumber("Da xuat vien");
    inpat->setBedNumber("0");
    inpat->setDischargeDate(dischargeDate);

    return 1;
}