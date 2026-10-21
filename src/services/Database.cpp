#include "services/Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <iomanip> 

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

// --- LUỒNG MỚI: SINH MÃ BỆNH ÁN DUY NHẤT TĂNG TIẾN (BA00001...) ---
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
    else if (role == "QuanLyKho") prefix = "KHO";
    
    userCounters[prefix]++;
    stringstream ss;
    ss << prefix << setfill('0') << setw(2) << userCounters[prefix];
    return ss.str();
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
            string id, patientId, medFeeStr;
            
            getline(ss, id, ';');
            getline(ss, patientId, ';');
            getline(ss, medFeeStr, ';');
            
            double medFee = 0;
            try { medFee = stod(medFeeStr); } catch (...) {}

            invoices.push_back(Invoice(id, patientId, medFee));
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
            
            InvoiceDetail detail(dId, iId , sId, qty, price);
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
            string aId, pId, dId, rId, spec ,date, status;

            getline(ss, aId, ';');
            getline(ss, pId, ';');
            getline(ss, dId, ';');
            getline(ss, rId, ';');
            getline(ss,spec,';');
            getline(ss, date, ';');
            getline(ss, status, ';');

            if (aId.length() > 3) {
                int num = 0;
                try { num = stoi(aId.substr(3)); } catch(...) {}
                if (num > appointmentCounter) appointmentCounter = num;
            }

            appointments.push_back(Appointment(aId, pId, dId, rId, spec,date, status));
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
            string rId, aId, symp, diag, notes;

            getline(ss, rId, ';');
            getline(ss, aId, ';');
            getline(ss, symp, ';');
            getline(ss, diag, ';');
            getline(ss, notes, ';');

            // Đọc và đồng bộ bộ đếm cho mã BA...
            if (rId.length() > 2) {
                int num = 0;
                try { num = stoi(rId.substr(2)); } catch(...) {}
                if (num > medicalRecordCounter) {
                    medicalRecordCounter = num;
                }
            }

            medicalRecords.push_back(MedicalRecord(rId, aId, symp, diag, notes));
        }
        fMedRec.close();
    }

    ifstream fPresc(prescriptionDetailFile);
    if (fPresc.is_open()) {
        string line;
        while (getline(fPresc, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string rId, mId, dosage, qtyStr;

            getline(ss, rId, ';');       
            getline(ss, mId, ';');       
            getline(ss, dosage, ';');    
            getline(ss, qtyStr, ';');    

            int qty = 0;
            try { qty = stoi(qtyStr); } catch (...) {}

            for (auto& record : medicalRecords){
                if (record.getRecordId() == rId){
                    record.addMedicine(mId, dosage, qty);
                    break;
                }
            }
        }
        fPresc.close();
    } 

    cout << "-> Tai du lieu hoan tat!" << endl;
}

// ==========================================
// SAVE DATA TO CSV
// ==========================================
void Database::saveAllData() {
    cout << "Dang luu co so du lieu..." << endl;
    
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
                  << appt.getStatus() << "\n";
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
                    << record.getNotes() << "\n";
            for (const auto& rx : record.getPrescriptions()) {
                fPresc << record.getRecordId() << ";"
                       << rx.medicineId << ";"
                       << rx.dosage << ";"
                       << rx.quantity << "\n"; 
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

        if (users.isEmpty()) {
            User defaultAdmin("ADMIN01", "admin", "", "Quan Tri Vien", "Admin", true);
            defaultAdmin.setPassword("admin123"); 
            users.insert("ADMIN01", defaultAdmin);
        }
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
                 << inv.getMedicineFee() << "\n";

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

// ==========================================
// CÁC HÀM TÌM KIẾM
// ==========================================
Doctor* Database::findDoctorById(string id) { return doctors.search(id); }

Patient* Database::findPatientById(string id) {
   Patient** p = patients.search(id); 
    if (p != nullptr) { return *p;}
    return nullptr;
}

vector<Patient*> Database::searchPatients(string keyword) {
   vector<Patient*> results;
    vector<Patient**> allPats = patients.getAllPointers();
    for (size_t i = 0; i < allPats.size(); i++) {
        Patient* pat = *(allPats[i]); 
        if (pat->getId() == keyword || pat->getFullName().find(keyword) != string::npos) {
            results.push_back(pat);
        }
    }
    return results;
}

Medicine* Database::findMedicineById(string id) { return medicines.search(id); }

Appointment* Database::findAppointmentById(string id) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].getAppointmentId() == id) {
            return &appointments[i];
        }
    }
    return nullptr;
}

MedicalRecord* Database::findMedicalRecordById(string id){
    for (size_t i = 0;i < medicalRecords.size();i++) {
        if (medicalRecords[i].getRecordId() == id) {
            return &medicalRecords[i];
        }
    }
    return nullptr;
}

// ==========================================
// CÁC HÀM XÓA & CẬP NHẬT
// ==========================================
bool Database::deleteUserFromQT(const std::string& id) {
    if (users.search(id) == nullptr) return false; 
    users.remove(id); 
    return true;
}

int Database::deleteDoctorFromQT(const std::string& id) {
    if (doctors.search(id) == nullptr) return -1;
    for (const auto& appt : appointments) {
        if (appt.getDoctorId() == id && appt.getStatus() == "Pending") {
            return -2; 
        }
    }
    doctors.remove(id);
    if (users.search(id) != nullptr) { users.remove(id); }
    return 1;
}

int Database::deleteMedicineFromQT(const std::string& id) {
    if (medicines.search(id) == nullptr) return -1;
    for (const auto& record : medicalRecords) {
        for (const auto& rx : record.getPrescriptions()) {
            if (rx.medicineId == id) {
                return -2; 
            }
        }
    }
    medicines.remove(id);
    return 1;
}

bool Database::deleteServiceFromQT(const std::string& id) {
    if (services.search(id) == nullptr) return false;
    services.remove(id);
    return true;
}

// ==========================================
// CÁC HÀM XỬ LÝ NGHIỆP VỤ (LỄ TÂN & BÁC SĨ)
// ==========================================

bool Database::addPatientFromGUI(string name, string phone, string BH, bool isNoiTru){
    string newId = generatePatientId();
    if (isNoiTru){
        patients.insert(newId, new Inpatient(newId, name, "", "", phone, "", BH, "", "", "", ""));
    } else{
        patients.insert(newId, new Outpatient(newId, name, "", "", phone, "", BH, "", ""));
    }
    return true;
}

// --- MỤC C: LỄ TÂN TẠO LỊCH ĐỒNG THỜI SINH LUÔN MÃ BỆNH ÁN RỖNG (BA...) ---
int Database::bookAppointmentFromQT(string patientId, string doctorId, string receptionistId, string specialty, string day, string time) {
    if (findPatientById(patientId) == nullptr) return -1; 
    Doctor* doc = findDoctorById(doctorId);
    if (doc == nullptr) return -2; 
    
    bool isAvailable = false;
    for (const auto& shift : doc->getSchedule()) {
        if (shift.dayOfWeek == day && shift.timeSlot == time) {
            isAvailable = true;
            break;
        }
    }
    if (!isAvailable) return -3;
    
    string newAppId = generateAppointmentId(); 
    appointments.push_back(Appointment(newAppId, patientId, doctorId, receptionistId, specialty, day, time, "Pending"));
    
    // Tự động sinh mã bệnh án BA... và nhét vào kho lưu trữ ngay từ khâu Lễ Tân
    string newRecordId = generateMedicalRecordId(); 
    medicalRecords.push_back(MedicalRecord(newRecordId, newAppId, "", "", ""));
    
    saveAllData();
    return 1; 
}

// --- MỤC D: BÁC SĨ GỌI HÀM CẬP NHẬT THAY VÌ TỰ CHẾ MÃ MỚI ---
string Database::updateMedicalRecordFromQT(string apptId, string symp, string diag, string notes) {
    Appointment* appt = findAppointmentById(apptId);
    if (appt == nullptr) return "";
    
    for (auto& record : medicalRecords) {
        if (record.getAppointmentId() == apptId) {
            record.setSymptoms(symp);
            record.setDiagnosis(diag);
            record.setNotes(notes);
            
            appt->setStatus("Completed");
            saveAllData(); 
            return record.getRecordId(); // Trả về mã BA... chuẩn xác đã được cấp từ Lễ tân
        }
    }
    return "";
}

// ==========================================
// CÁC HÀM ĐĂNG NHẬP VÀ THÊM DỮ LIỆU KHÁC
// ==========================================
User* Database::loginFromQT(string username, string password) {
    if (users.isEmpty() && username == "admin" && password == "admin123") {
        User defaultAdmin("ADMIN01", "admin", "", "Quan Tri Vien", "Admin", true);
        defaultAdmin.setPassword("admin123");
        users.insert("ADMIN01", defaultAdmin);
        return users.search("ADMIN01");
    }
    vector<User*> allusers = users.getAllPointers();
    for (size_t i = 0; i < allusers.size(); i++) {
        if (allusers[i]->getUsername() == username) {
            if (allusers[i]->authenticate(password)) return allusers[i];
            else return nullptr; 
        }
    }
    return nullptr; 
}

int Database::addUserFromQT(string id, string username, string plainPassword, string fullName, string role) {
    if (users.search(id) != nullptr) return -1; 
    vector<User*> allUsers = users.getAllPointers();
    for (size_t i = 0; i < allUsers.size(); i++) {
        if (allUsers[i]->getUsername() == username) return -1; 
    }
    User newUser(id, username, "", fullName, role, true);
    newUser.setPassword(plainPassword);
    users.insert(id, newUser); 
    return 1;
}

int Database::addDoctorWithAccount(string id, string name, string dob, string gender, string phone, string specialty, int expYears, string username, string plainPassword) {
     if (doctors.search(id) != nullptr) return -1; 
    vector<Doctor*> allDocs = doctors.getAllPointers();
    for (size_t i = 0; i < allDocs.size(); i++) {
        if (allDocs[i]->getFullName() == username) return -2; 
    }
    doctors.insert(id,Doctor(id, name, dob, gender, phone, specialty, expYears));
    User newAccount(id, username, "", name, "Doctor", true); 
    newAccount.setPassword(plainPassword); 
    users.insert(id,newAccount);
    return 1;
}

int Database::addMedicineFromQT(string prefix, string medicineName, double unitPrice, string dosage, int addQty, string& outId) {
    vector<Medicine*> allMeds = medicines.getAllPointers();
    for (auto med : allMeds) {
        if (med->getMedicineName() == medicineName) {
            med->setStock(med->getStock() + addQty); 
            outId = med->getMedicineId(); 
            return 2; 
        }
    }
    string newId = generateMedicineId(prefix);
    medicines.insert(newId, Medicine(newId, medicineName, unitPrice, dosage, addQty));
    outId = newId; 
    return 1; 
}

int Database::addServiceFromQT(string prefix, string name, double price, string& outId) {
    vector<Service*> allSrv = services.getAllPointers();
    for (auto srv : allSrv) {
        if (srv->getServiceName() == name) {
            outId = srv->getId(); 
            return -1; 
        }
    }
    string newId = generateServiceId(prefix);
    services.insert(newId, Service(newId, name, price));
    outId = newId; 
    return 1; 
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
    if (med->getStock() < qty) return -3;
    med->setStock(med->getStock() - qty);
    record->addMedicine(medId, dosage, qty);
    return 1;
}

int Database::createInvoiceFromQT(string patientId, double& outTotalFee) {
    Patient* pat = findPatientById(patientId);
    if (pat == nullptr) return -1;

    string targetApptId = "";
    for (int i = appointments.size() - 1; i >= 0; i--) {
        if (appointments[i].getPatientId() == patientId) {
            targetApptId = appointments[i].getAppointmentId();
            break;
        }
    }
    if (targetApptId == "") return -2;

    MedicalRecord* record = nullptr;
    for (size_t i = 0; i < medicalRecords.size(); i++) {
        if (medicalRecords[i].getAppointmentId() == targetApptId) {
            record = &medicalRecords[i];
            break;
        }
    }
    if (record == nullptr || record->getDiagnosis().empty()) return -3;

    bool isBHYT = pat->hasBHYT();
    double medFee = 0.0;       
    double finalMedFee = 0.0;  

    for (const auto& rx : record->getPrescriptions()) {
        Medicine* med = findMedicineById(rx.medicineId);
        if (med != nullptr) {
            double rawLinePrice = med->getUnitPrice() * rx.quantity;
            medFee += rawLinePrice; 

            if (isBHYT) {
                string mId = med->getMedicineId(); 
                string prefix = (mId.length() >= 2) ? mId.substr(0, 2) : "";
                
                if (prefix == "B1") {
                    finalMedFee += 0; 
                } 
                else if (prefix == "B2") {
                    finalMedFee += rawLinePrice * 0.5; 
                } 
                else if (prefix == "B3") {
                    finalMedFee += rawLinePrice * 0.2; 
                }
                else {
                    finalMedFee += rawLinePrice; 
                }
            } else {
                finalMedFee += rawLinePrice; 
            }
        }
    }

    string newInvId = "INV" + to_string(invoices.size() + 1);
    Invoice newInv(newInvId, patientId, medFee);

    string srvId = "S1_001";          
    double srvPrice = 100000.0;      
    vector<Service*> srvList = services.getAllPointers();
    
    if (!srvList.empty()) {
        Service* selectedSrv = srvList[0];
        for (auto s : srvList) {
            if (s->getId() == "S1_001") {
                selectedSrv = s;
                break;
            }
        }
        srvId = selectedSrv->getId();
        srvPrice = selectedSrv->getPrice();
    }

    double finalSrvPrice = srvPrice; 

    if (isBHYT) {
        string prefix = (srvId.length() >= 2) ? srvId.substr(0, 2) : "";

        if (prefix == "S1") {
            finalSrvPrice = 0; 
        } 
        else if (prefix == "S2") {
            finalSrvPrice = srvPrice * 0.3; 
        } 
        else if (prefix == "S3") {
            finalSrvPrice = srvPrice * 0.8; 
        }
        else {
            finalSrvPrice = srvPrice; 
        }
    }
    
    string detailId = newInvId + "_D1";
    InvoiceDetail consultDetail(detailId, srvId, newInvId, 1, srvPrice);
    newInv.addDetail(consultDetail);

    outTotalFee = finalSrvPrice + finalMedFee; 

    invoices.push_back(newInv);
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
    if (appt->getStatus() == "Completed") return -2; 
    appt->setStatus("Canceled");
    saveAllData(); 
    return 1; 
}

void Database::deleteDoctor(string id) {
    if (doctors.remove(id)) {
        cout << "=> Da xoa Bac si thanh cong!\n";
    } else {
        cout << "Loi: Khong tim thay ID Bac si nay!\n";
    }
}

void Database::deleteMedicine(string id) {
    if (medicines.remove(id)) {
        cout << "=> Da xoa Thuoc thanh cong!\n";
    } else {
        cout << "Loi: Khong tim thay ID Thuoc!\n";
    }
}

void Database::deletePatient(string id) {
    Patient* pat = findPatientById(id);
    if (pat != nullptr) {
        delete pat; 
        patients.remove(id); 
        cout << "=> Da xoa Benh nhan thanh cong!\n";
    } else {
        cout << "Loi: Khong tim thay ID Benh nhan!\n";
    }
}

void Database::updateDoctor(string id) {
    Doctor* doc = findDoctorById(id);
    if (doc == nullptr) return;
    string input;
    cout << "So dien thoai moi: ";
    getline(cin, input);
    sanitizeInput(input);
    if (!input.empty()) {
        doc->setPhoneNumber(input);
    }
    cout << "Ban co muon xep lai lich truc khong? (y/n): ";
    getline(cin, input);
    if (input == "y" || input == "Y") {
        doc->clearSchedule();
        string soCaStr;
        cout << "Nhap so luong ca truc moi: ";
        getline(cin, soCaStr);
        int soCa = 0;
        try { soCa = stoi(soCaStr); } catch (...) {}
        for (int i = 0; i < soCa; i++) {
            string day, time;
            cout << "- Ca " << i + 1 << ":\n";
            cout << "  + Thu: "; getline(cin, day); sanitizeInput(day);
            cout << "  + Buoi: "; getline(cin, time); sanitizeInput(time);
            doc->setWorkSchedule(day, time);
        }
    }
    cout << "=> Cap nhat thanh cong!\n";
}

void Database::updateMedicine(string id) {
    Medicine* med = findMedicineById(id);
    if (med == nullptr) return;
    string input;
    cout << "Gia moi: ";
    getline(cin, input);
    sanitizeInput(input);
    if (!input.empty()) {
        try {
            double newPrice = stod(input);
            med->setUnitPrice(newPrice);
        } catch (...) {}
    }
}
void Database::updatePatient(string id) {
    Patient* pat = findPatientById(id);
    if (pat == nullptr) {
        cout << "Loi: Khong tim thay Benh nhan co ID: " << id << "\n";
        return;
    }
    cout << "--- CAP NHAT THONG TIN BENH NHAN: " << pat->getFullName() << " ---\n";
    cout << "(Nhan Enter de bo qua neu khong muon doi thong tin do)\n";
    string input;
    // ==========================================
    // 1. CẬP NHẬT THÔNG TIN CHUNG (Lớp Person / Patient)
    // ==========================================
    cout << "So dien thoai moi (Hien tai: " << pat->getPhoneNumber() << "): ";
    getline(cin, input);
    sanitizeInput(input);
    if (!input.empty()) {
        pat->setPhoneNumber(input); 
    }

    cout << "Ma Bao Hiem Y Te moi (Hien tai: " << pat->getBHYT() << "): ";
    getline(cin, input);
    sanitizeInput(input);
    if (!input.empty()) {
        pat->setBHYT(input); 
    }
    // ==========================================
    // 2. PHÂN LOẠI VÀ CẬP NHẬT THÔNG TIN RIÊNG (Lớp con)
    // ==========================================
    if (Inpatient* inpat = dynamic_cast<Inpatient*>(pat)) {
        cout << "\n[Phat hien he thong: Benh Nhan Noi Tru]\n";
        cout << "So phong moi (Hien tai: " << inpat->getRoomNumber() << "): ";
        getline(cin, input);
        sanitizeInput(input);
        if (!input.empty()) {
            inpat->setRoomNumber(input);
        }
        cout << "So giuong moi (Hien tai: " << inpat->getBedNumber() << "): ";
        getline(cin, input);
        sanitizeInput(input);
        if (!input.empty()) {
            inpat->setBedNumber(input);
        }
    } 
    else if (Outpatient* outpat = dynamic_cast<Outpatient*>(pat)) {
        cout << "\n[Phat hien he thong: Benh Nhan Ngoai Tru]\n";
        
        cout << "Phong kham moi (Hien tai: " << outpat->getClinicRoom() << "): ";
        getline(cin, input);
        sanitizeInput(input);
        if (!input.empty()) {
            outpat->setClinicRoom(input);
        }

        cout << "Ngay tai kham moi (Hien tai: " << outpat->getAppointmentDate() << "): ";
        getline(cin, input);
        sanitizeInput(input);
        if (!input.empty()) {
            outpat->setAppointmentDate(input);
        }
    }

    cout << "=> Cap nhat thong tin Benh nhan thanh cong!\n";
}
void Database::addDoctor() {
    string id, name, dob, gender, phone, spec, schedule, expStr;
    
    cout << "\n--- THEM BAC SI MOI ---\n";
    cout << "Nhap ID: ";
    getline(cin, id);
    sanitizeInput(id);

    if (findDoctorById(id) != nullptr) {
        cout << "Loi: ID Bac si nay da ton tai trong he thong!\n";
        return; 
    }

    cout << "Nhap Ho Ten: ";
    getline(cin, name); sanitizeInput(name);

    cout << "Nhap Ngay sinh (DD/MM/YYYY): ";
    getline(cin, dob); sanitizeInput(dob);

    cout << "Nhap Gioi tinh: ";
    getline(cin, gender); sanitizeInput(gender);

    cout << "Nhap So dien thoai: ";
    getline(cin, phone); sanitizeInput(phone);

    cout << "Nhap Chuyen khoa: ";
    getline(cin, spec); sanitizeInput(spec);

    cout << "Nhap So nam kinh nghiem: ";
    getline(cin, expStr); sanitizeInput(expStr);
    int exp = 0;
    try { exp = stoi(expStr); } catch (...) { exp = 0; }
    Doctor newDoctor(id, name, dob, gender, phone, spec, exp);
    string soCaStr;
    cout << "Bac si nay lam may ca 1 tuan? : ";
    getline(cin, soCaStr);
    int soCa = 0;
    try { soCa = stoi(soCaStr); } catch (...) { soCa = 0; }

    for (int i = 0; i < soCa; i++) {
        string day, shift;
        cout << "- Ca thu " << i + 1 << ":\n";
        
        cout << "  + Thu (VD: Thu 2, Thu 3): ";
        getline(cin, day); sanitizeInput(day);
        
        cout << "  + Buoi (VD: Sang, Chieu, Toi): ";
        getline(cin, shift); sanitizeInput(shift);

        newDoctor.setWorkSchedule(day, shift);
    }
    doctors.insert(id,newDoctor);
    cout << "=> Da them Bac si thanh cong!\n";
}

void Database::showAllDoctors() const {
    cout << "\n--- DANH SACH BAC SI ---\n";
    if (doctors.isEmpty()) {
        cout << "Chua co du lieu!\n";
        return;
    }
    vector<Doctor*> allDocs = doctors.getAllPointers();
    for (size_t i = 0; i < allDocs.size(); i++){
        cout << i + 1 << ". ";
        allDocs[i]->displayData();
    }
}
void Database::addMedicine() {
    string id, name, priceStr, dosage;

    cout << "\n--- THEM THUOC MOI ---\n";
    cout << "Nhap ID Thuoc: ";
    getline(cin, id);
    sanitizeInput(id);

   Medicine* existingMed = findMedicineById(id);

    if (existingMed != nullptr) {
        cout << "Thuoc [" << existingMed->getMedicineName() << "] dang co " 
             << existingMed->getStock() << " vien trong kho.\n";
        
        string addQtyStr;
        cout << "Nhap so luong muon nhap them: ";
        getline(cin, addQtyStr);
        int addQty = 0;
        try { addQty = stoi(addQtyStr); } catch (...) {}

        existingMed->setStock(existingMed->getStock() + addQty);
        cout << "=> Da cap nhat so luong ton kho thanh cong!\n";
        return;
    }

    cout << "Nhap Ten thuoc: ";
    getline(cin, name); sanitizeInput(name);

    cout << "Nhap Don gia: ";
    getline(cin, priceStr); sanitizeInput(priceStr);
    double price = 0.0;
    try { price = stod(priceStr); } catch (...) { price = 0.0; }

    cout << "Nhap Lieu dung / Cach dung: ";
    getline(cin, dosage); sanitizeInput(dosage);

    medicines.insert(id,Medicine(id, name, price, dosage));
    cout << "=> Da them Thuoc thanh cong!\n";
}

void Database::showAllMedicine() const {
    cout << "\n--- DANH SACH THUOC ---\n";
    if (medicines.isEmpty()) {
        cout << "Chua co du lieu!\n";
        return;
    }
    vector<Medicine*> allMeds = medicines.getAllPointers();
    for (size_t i = 0; i < allMeds.size(); i++){
        cout << i + 1 << ". ";
        allMeds[i]->displayData(); 
    }
}
void Database::addPatient() {
    string id, name, dob, gender, phone, recordId, bhyt;
    
    cout << "\n--- THEM BENH NHAN MOI ---\n";
    cout << "Nhap ID: ";
    getline(cin, id); sanitizeInput(id);

    if (findPatientById(id) != nullptr) {
        cout << "Loi: ID Benh nhan nay da ton tai!\n";
        return;
    }

    cout << "Nhap Ho Ten: ";
    getline(cin, name); sanitizeInput(name);

    cout << "Nhap Ngay sinh: ";
    getline(cin, dob); sanitizeInput(dob);

    cout << "Nhap Gioi tinh: ";
    getline(cin, gender); sanitizeInput(gender);

    cout << "Nhap So dien thoai: ";
    getline(cin, phone); sanitizeInput(phone);

    cout << "Nhap Ma BHYT (Neu khong co thi de trong): ";
    getline(cin, recordId); sanitizeInput(recordId);

    cout << "Nhap BHYT: ";
    getline(cin, bhyt); sanitizeInput(bhyt);
    cout << "\nChon loai Benh nhan:\n1. Noi Tru (Nhap vien)\n2. Ngoai Tru (Kham xong ve)\nLua chon: ";
    string choice;
    getline(cin, choice);

    if (choice == "1") {
        string admDate, disDate, room, bed;
        cout << "Nhap Ngay nhap vien: "; getline(cin, admDate); sanitizeInput(admDate);
        cout << "Nhap Ngay xuat vien (De trong neu chua xuat): "; getline(cin, disDate); sanitizeInput(disDate);
        cout << "Nhap So phong: "; getline(cin, room); sanitizeInput(room);
        cout << "Nhap So giuong: "; getline(cin, bed); sanitizeInput(bed);

        // Tạo đối tượng Nội trú và đẩy con trỏ vào mảng
        patients.insert(id,new Inpatient(id, name, dob, gender, phone, recordId, bhyt, admDate, disDate, room, bed));
        cout << "=> Da them Benh nhan Noi tru thanh cong!\n";

    } else if (choice == "2") {
        string apptDate, clinic;
        cout << "Nhap Ngay hen kham: "; getline(cin, apptDate); sanitizeInput(apptDate);
        cout << "Nhap Phong kham (VD: Khoa Noi, Phong 102): "; getline(cin, clinic); sanitizeInput(clinic);
        patients.insert(id,new Outpatient(id, name, dob, gender, phone, recordId, bhyt, apptDate, clinic));
        cout << "=> Da them Benh nhan Ngoai tru thanh cong!\n";
        
    } else {
        cout << "Loi: Lua chon khong hop le. Da huy qua trinh them Benh nhan!\n";
    }
}

void Database::showAllPatients() const {
    cout << "\n--- DANH SACH BENH NHAN ---\n";
    if (patients.isEmpty()) {
        cout << "Chua co du lieu!\n";
        return;
    }
    vector<Patient**> allPats = patients.getAllPointers();
    for (size_t i = 0; i < allPats.size(); i++) {
        cout << i + 1 << ". ";
        (*allPats[i])->displayData();
    }
}
