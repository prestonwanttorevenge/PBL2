#include "services/Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <iomanip> 
#include <cctype>
#include <cstdio>

Database::Database() {
}

Database::~Database() {
    vector<Patient**> allPatients = patients.getAllPointers();
    for (size_t i = 0; i < allPatients.size(); i++) {
        delete *(allPatients[i]); 
    }
}
void Database::sanitizeInput(string& input) {
    for (size_t i = 0; i < input.length(); i++) {
        if (input[i] == ';') {
            input[i] = ',';
        }
    }
}

// ==========================================
// CÁC HÀM SINH MÃ TỰ ĐỘNG
// ==========================================

string Database::generatePatientId() {
    patientCounter++;
    stringstream ss;
    ss << "BN" << setfill('0') << setw(5) << patientCounter;
    return ss.str();
}

string Database::generateAppointmentId() {
    appointmentCounter++;
    stringstream ss;
    ss << "APP" << setfill('0') << setw(4) << appointmentCounter;
    return ss.str();
}

string Database::generateMedicalRecordId() {
    medicalRecordCounter++;
    stringstream ss;
    ss << "BA" << setfill('0') << setw(5) << medicalRecordCounter;
    return ss.str();
}

string Database::generateUserId(const string& role) {
    string prefix = "U";
    if (role == "Admin") prefix = "ADMIN";
    else if (role == "Receptionist") prefix = "RECEP";
    else if (role == "Inventory" || role == "QuanLyKho" || role == "DuocSi") prefix = "KHO";

    string id;
    do {
        ++userCounters[prefix];
        stringstream ss;
        ss << prefix << setfill('0') << setw(2) << userCounters[prefix];
        id = ss.str();
    } while (users.search(id) != nullptr);
    return id;
}

string Database::generateDoctorId() {
    doctorCounter++;
    stringstream ss;
    ss << "BS" << setfill('0') << setw(2) << doctorCounter;
    return ss.str();
}

string Database::generateMedicineId(const string& prefix) {
    medicineCounters[prefix]++;
    stringstream ss;
    ss << prefix << "_" << setfill('0') << setw(3) << medicineCounters[prefix];
    return ss.str();
}

string Database::generateServiceId(const string& prefix) {
    serviceCounters[prefix]++;
    stringstream ss;
    ss << prefix << "_" << setfill('0') << setw(3) << serviceCounters[prefix];
    return ss.str();
}

// ==========================================
// LOAD DATA TỪ CSV 
// ==========================================
void Database::loadAllData() {
    cout << "Dang tai co so du lieu..." << endl;

    patientCounter = 0;
    appointmentCounter = 0;
    doctorCounter = 0;
    medicalRecordCounter = 0;
    medicineCounters.clear();
    serviceCounters.clear();
    userCounters.clear();

    ifstream fDoc(doctorFile);
    if (fDoc.is_open()) {
        string line;
        doctors.clear(); 
        while (getline(fDoc, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string id, name, dob, gender, phone, spec, tempExp;

            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, dob, ';');
            getline(ss, gender, ';');
            getline(ss, phone, ';');
            getline(ss, spec, ';');
            getline(ss, tempExp, ';');
            int exp = 0;
            try { exp = stoi(tempExp); } catch (...) {}

            if (id.substr(0, 2) == "BS") {
                int num = 0;
                try { num = stoi(id.substr(2)); } catch(...) {}
                if (num > doctorCounter) doctorCounter = num;
            }

            doctors.insert(id, Doctor(id, name, dob, gender, phone, spec, exp));
        }
        fDoc.close();
    }

    std::ifstream fSched(doctorScheduleFile);
    if (fSched.is_open()) {
        std::string line;
        while (std::getline(fSched, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string docId, day, time;
            std::getline(ss, docId, ';');
            std::getline(ss, day, ';');
            std::getline(ss, time, ';');

            Doctor* doc = findDoctorById(docId); 
            if (doc != nullptr) {
                doc->setWorkSchedule(day, time);
            }
        }
        fSched.close();
    }

    ifstream fPat(patientFile);
    if (fPat.is_open()) {
        string line;
        vector<Patient**> oldPats = patients.getAllPointers();
        for (size_t i = 0; i < oldPats.size(); i++) {
            delete *(oldPats[i]); 
        }
        patients.clear(); 

        while (getline(fPat, line)) {
            if (line.empty()) continue;
            stringstream ss(line);

            string type, id, name, dob, gender, phone, recordId, BHYTstr;
            getline(ss, type, ';'); 
            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, dob, ';');
            getline(ss, gender, ';');
            getline(ss, phone, ';');
            getline(ss, recordId, ';');
            getline(ss, BHYTstr, ';');

            if (id.length() > 2) {
                int num = 0;
                try { num = stoi(id.substr(2)); } catch(...) {}
                if (num > patientCounter) patientCounter = num;
            }
            if (type == "NoiTru") {
                string admDate, disDate, room, bed;
                getline(ss, admDate, ';');
                getline(ss, disDate, ';');
                getline(ss, room, ';');
                getline(ss, bed, ';');
                patients.insert(id, new Inpatient(id, name, dob, gender, phone, recordId, BHYTstr, admDate, disDate, room, bed));
            } 
            else if (type == "NgoaiTru") {
                string appDate, clinicRoom;
                getline(ss, appDate, ';');
                getline(ss, clinicRoom, ';');
                patients.insert(id, new Outpatient(id, name, dob, gender, phone, recordId, BHYTstr, appDate, clinicRoom));
            }
        }
        fPat.close();
    }

    ifstream fInv(invoiceFile);
    if (fInv.is_open()) {
        string line;
        invoices.clear();
        while (getline(fInv, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, patientId, medFeeStr, appointmentId;

            getline(ss, id, ';');
            getline(ss, patientId, ';');
            getline(ss, medFeeStr, ';');
            getline(ss, appointmentId, ';');

            double medFee = 0;
            try { medFee = stod(medFeeStr); } catch (...) {}

            invoices.push_back(Invoice(id, patientId, medFee, appointmentId));
        }
        fInv.close();
    } 

    ifstream fDetail(invoiceDetailsFile);
    if (fDetail.is_open()) {
        string line;
        while (getline(fDetail, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string dId, iId, sId, qtyStr, priceStr;

            getline(ss, dId, ';');    
            getline(ss, iId, ';');    
            getline(ss, sId, ';');   
            getline(ss, qtyStr, ';');
            getline(ss, priceStr, ';');

            int qty = 0;
            double price = 0;
            try { 
                qty = stoi(qtyStr); 
                price = stod(priceStr); 
            } catch (...) {}

            InvoiceDetail detail(dId, sId, iId, qty, price);
            for (auto& inv : invoices) { 
                if (inv.getId() == iId) {
                    inv.addDetail(detail); 
                    break;
                }
            }
        }
        fDetail.close();
    } 

    ifstream fUser(userFile);
    if (fUser.is_open()) {
        string line;
        users.clear();
        while (getline(fUser, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string id, Username , pass , fullName , role, activestr;
            getline(ss, id, ';');
            getline(ss, Username, ';');
            getline(ss, pass, ';');
            getline(ss, fullName, ';');
            getline(ss, role, ';');
            getline(ss, activestr, ';');
            if (role == "QuanLyKho" || role == "DuocSi") role = "Inventory";

            bool isActive = false;
            if (activestr == "1" || activestr == "true") { isActive = true; }

            string prefix = ""; string numStr = "";
            for (char c : id) {
                if (isalpha(c)) prefix += c;
                else if (isdigit(c)) numStr += c;
            }
            if (!numStr.empty()) {
                int num = 0;
                try { num = stoi(numStr); } catch(...) {}
                if (num > userCounters[prefix]) userCounters[prefix] = num;
            }

            users.insert(id, User(id, Username, pass, fullName, role, isActive));
        }
        fUser.close();
    }

    ifstream fMed(medicineFile);
    if (fMed.is_open()) {
        string line;
        medicines.clear();
        while (getline(fMed, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, name, priceStr, dosage, stockStr;

            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, priceStr, ';');
            getline(ss, dosage, ';');
            getline(ss, stockStr, ';');

            double price = 0.0;
            int stock = 0;
            try { price = stod(priceStr); } catch (...) {}
            try { stock = stoi(stockStr); } catch (...) {}

            if (id.find('_') != string::npos){
                string prefix = id.substr(0, id.find('_'));
                string numStr = id.substr(id.find('_') + 1);

                int num = 0;
                try { num = stoi(numStr); } catch(...) {}
                if (num > medicineCounters[prefix]) {
                    medicineCounters[prefix] = num;
                }
            }

            medicines.insert(id, Medicine(id, name, price, dosage, stock));
        }
        fMed.close();
    }

    ifstream fSrv(serviceFile);
    if (fSrv.is_open()) {
        string line;
        services.clear();
        while (getline(fSrv, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string id, name, priceStr;

            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, priceStr, ';');

           double price = 0.0;
            try { price = stod(priceStr); } catch (...) {}

            if (id.find('_') != string::npos) {
                string prefix = id.substr(0, id.find('_'));
                string numStr = id.substr(id.find('_') + 1);

                int num = 0;
                try { num = stoi(numStr); } catch(...) {}
                if (num > serviceCounters[prefix]) {
                    serviceCounters[prefix] = num;
                }
            }

            services.insert(id, Service(id, name, price));
        }
        fSrv.close();
    }

    ifstream fAppt(appointmentFile);
    if (fAppt.is_open()) {
        string line;
        appointments.clear();
        while (getline(fAppt, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string aId, pId, dId, rId, spec, day, time, status, date;

            getline(ss, aId, ';');
            getline(ss, pId, ';');
            getline(ss, dId, ';');
            getline(ss, rId, ';');
            getline(ss, spec, ';');
            getline(ss, day, ';');
            getline(ss, time, ';');
            getline(ss, status, ';');
            getline(ss, date, ';');

            if (aId.length() > 3) {
                int num = 0;
                try { num = stoi(aId.substr(3)); } catch(...) {}
                if (num > appointmentCounter) appointmentCounter = num;
            }

            appointments.push_back(Appointment(aId, pId, dId, rId, spec, day, time, status, date));
        }
        fAppt.close();
    }

    ifstream fMedRec(medicalRecordFile);
    if (fMedRec.is_open()) {
        string line;
        medicalRecords.clear();
        while (getline(fMedRec, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string rId, aId, symp, diag, notes, assignedServices;

            getline(ss, rId, ';');
            getline(ss, aId, ';');
            getline(ss, symp, ';');
            getline(ss, diag, ';');
            getline(ss, notes, ';');
            getline(ss, assignedServices, ';');

            if (rId.length() > 2) {
                int num = 0;
                try { num = stoi(rId.substr(2)); } catch(...) {}
                if (num > medicalRecordCounter) {
                    medicalRecordCounter = num;
                }
            }

            medicalRecords.push_back(MedicalRecord(rId, aId, symp, diag, notes));
            if (!assignedServices.empty()) {
                stringstream servicesStream(assignedServices);
                string serviceId;
                while (getline(servicesStream, serviceId, ',')) {
                    if (!serviceId.empty()) {
                        medicalRecords.back().addService(serviceId);
                    }
                }
            }
        }
        fMedRec.close();
    }

    ifstream fPresc(prescriptionDetailFile);
    if (fPresc.is_open()) {
        string line;
        while (getline(fPresc, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string rId, mId, dosage, qtyStr, priceStr, dispensedStr;

            getline(ss, rId, ';');       
            getline(ss, mId, ';');       
            getline(ss, dosage, ';');    
            getline(ss, qtyStr, ';');    
            getline(ss, priceStr, ';');
            getline(ss, dispensedStr, ';');

            int qty = 0;
            try { qty = stoi(qtyStr); } catch (...) {}
            Medicine* medicine = findMedicineById(mId);
            double price = medicine == nullptr ? 0.0 : medicine->getUnitPrice();
            if (!priceStr.empty()) {
                try { price = stod(priceStr); } catch (...) {}
            }
            const bool dispensed = priceStr.empty() || dispensedStr == "1" || dispensedStr == "true";

            for (auto& record : medicalRecords){
                if (record.getRecordId() == rId){
                    record.addMedicine(mId, dosage, qty, price, dispensed);
                    break;
                }
            }
        }
        fPresc.close();
    } 

    for (Invoice& invoice : invoices) {
        if (!invoice.getAppointmentId().empty()) {
            Appointment* appt = findAppointmentById(invoice.getAppointmentId());
            if (appt != nullptr && appt->getStatus() == "Completed") {
                appt->setStatus("Billed");
            }
            continue;
        }

        for (auto it = appointments.rbegin(); it != appointments.rend(); ++it) {
            if (it->getPatientId() == invoice.getPatientId() && it->getStatus() == "Completed") {
                invoice.setAppointmentId(it->getAppointmentId());
                it->setStatus("Billed");
                break;
            }
        }
    }

    cout << "-> Tai du lieu hoan tat!" << endl;
}

// ==========================================
// SAVE DATA TO CSV
// ==========================================
void Database::saveAllData() {
    cout << "Dang luu co so du lieu..." << endl;

    if (users.isEmpty()) {
        User defaultAdmin("ADMIN01", "admin", "", "Quan Tri Vien", "Admin", true);
        defaultAdmin.setPassword("admin123");
        users.insert("ADMIN01", defaultAdmin);
    }

    ofstream fsrv(serviceFile);
    if (fsrv.is_open()) {
        vector<Service*> allSrv = services.getAllPointers();
        for (size_t i = 0; i < allSrv.size(); i++) {
            fsrv << allSrv[i]->getId() << ";"
                 << allSrv[i]->getServiceName() << ";"
                 << allSrv[i]->getPrice() << "\n";
        }
        fsrv.close();
    }

    ofstream fappt(appointmentFile);
    if (fappt.is_open()) {
        for (const auto& appt : appointments){
            fappt << appt.getAppointmentId() << ";"
                  << appt.getPatientId() << ";"
                  << appt.getDoctorId() << ";"
                  << appt.getReceptionistId() << ";"
                  << appt.getSpecialty() << ";"
                  << appt.getDayOfWeek() << ";"
                  << appt.getTimeSlot() << ";"
                  << appt.getStatus() << ";"
                  << appt.getAppointmentDate() << "\n";
        }
        fappt.close();
    }

    ofstream fmed(medicineFile);
    if (fmed.is_open()) {
        vector<Medicine*> allMed = medicines.getAllPointers();
        for (size_t i = 0; i < allMed.size(); i++) {
            fmed << allMed[i]->getMedicineId() << ";"
                 << allMed[i]->getMedicineName() << ";"
                 << allMed[i]->getUnitPrice() << ";"
                 << allMed[i]->getDosage() << ";"
                 << allMed[i]->getStock() << "\n";
        }
        fmed.close();
    }

    ofstream fMedRec(medicalRecordFile);
    ofstream fPresc(prescriptionDetailFile); 

    if (fMedRec.is_open() && fPresc.is_open()) {
        for (const auto& record : medicalRecords) {
            fMedRec << record.getRecordId() << ";"
                    << record.getAppointmentId() << ";"
                    << record.getSymptoms() << ";"
                    << record.getDiagnosis() << ";"
                    << record.getNotes() << ";";
            const vector<string> assignedServices = record.getAssignedServices();
            for (size_t i = 0; i < assignedServices.size(); ++i) {
                if (i > 0) fMedRec << ",";
                fMedRec << assignedServices[i];
            }
            fMedRec << "\n";
            for (const auto& rx : record.getPrescriptions()) {
                fPresc << record.getRecordId() << ";"
                       << rx.medicineId << ";"
                       << rx.dosage << ";"
                       << rx.quantity << ";"
                       << rx.unitPrice << ";"
                       << (rx.dispensed ? "1" : "0") << "\n";
            }
        }
        fMedRec.close();
        fPresc.close();
    }

    std::ofstream fDoc(doctorFile);
    std::ofstream fSched(doctorScheduleFile); 

    if (fDoc.is_open() && fSched.is_open()) {
        vector<Doctor*> allDocs = doctors.getAllPointers();

        for (size_t i = 0; i < allDocs.size(); i++) {
            fDoc << allDocs[i]->getId() << ";"
                 << allDocs[i]->getFullName() << ";"
                 << allDocs[i]->getDateOfBirth() << ";"
                 << allDocs[i]->getGender() << ";"
                 << allDocs[i]->getPhoneNumber() << ";"
                 << allDocs[i]->getSpecialty() << ";"
                 << allDocs[i]->getYearsOfExperience() << "\n";

            const std::vector<WorkShift>& sched = allDocs[i]->getSchedule();
            for (size_t j = 0; j < sched.size(); j++) {
                fSched << allDocs[i]->getId() << ";"
                       << sched[j].dayOfWeek << ";"
                       << sched[j].timeSlot << "\n";
            }
        }
        fDoc.close();
        fSched.close();
    }

    ofstream fUser(userFile);
    if (fUser.is_open()) {
        vector<User*> allUsers = users.getAllPointers();
        for (size_t i = 0; i < allUsers.size(); i++) {
            fUser << allUsers[i]->getId() << ";"
                  << allUsers[i]->getUsername() << ";"
                  << allUsers[i]->getPassword() << ";"
                  << allUsers[i]->getFullName() << ";"
                  << allUsers[i]->getRole() << ";"
                  << (allUsers[i]->getIsActive() ? "1" : "0") << "\n";
        }
        fUser.close();

    }

    ofstream fPat(patientFile);
    if (fPat.is_open()) {
        vector<Patient**> allPats = patients.getAllPointers();
        for (size_t i = 0; i < allPats.size(); i++) {
            Patient* pat = *(allPats[i]); 

            if (Inpatient* inpat = dynamic_cast<Inpatient*>(pat)) {
                fPat << "NoiTru;"
                     << inpat->getId() << ";"
                     << inpat->getFullName() << ";"
                     << inpat->getDateOfBirth() << ";"
                     << inpat->getGender() << ";"
                     << inpat->getPhoneNumber() << ";"
                     << inpat->getMedicalRecordId() << ";"
                     << inpat->getBHYT() << ";"
                     << inpat->getAdmissionDate() << ";"
                     << inpat->getDischargeDate() << ";"
                     << inpat->getRoomNumber() << ";"
                     << inpat->getBedNumber() << "\n";
            } 
            else if (Outpatient* outpat = dynamic_cast<Outpatient*>(pat)) {
                fPat << "NgoaiTru;"
                     << outpat->getId() << ";"
                     << outpat->getFullName() << ";"
                     << outpat->getDateOfBirth() << ";"
                     << outpat->getGender() << ";"
                     << outpat->getPhoneNumber() << ";"
                     << outpat->getMedicalRecordId() << ";"
                     << outpat->getBHYT() << ";"
                     << outpat->getAppointmentDate() << ";"
                     << outpat->getClinicRoom() << "\n";
            }
        }
        fPat.close();
    }

    ofstream fInv(invoiceFile);
    ofstream fInvDetail(invoiceDetailsFile);

    if (fInv.is_open() && fInvDetail.is_open()) {
        for (const auto& inv : invoices) {
            fInv << inv.getId() << ";"
                 << inv.getPatientId() << ";"
                 << inv.getMedicineFee() << ";"
                 << inv.getAppointmentId() << "\n";

            for (const auto& detail : inv.getDetails()) {
                fInvDetail << detail.getDetailId() << ";"
                           << inv.getId() << ";"
                           << detail.getServiceId() << ";"
                           << detail.getQuantity() << ";"
                           << detail.getUnitPrice() << "\n";
            }
        }
        fInv.close();
        fInvDetail.close();
    }
}
