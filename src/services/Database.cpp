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

string Database::generateMedicalRecordId(const string& specialty) {
    string prefix = "KKB";
    if (specialty.find("Hồi sức") != string::npos) prefix = "KHS";
    else if (specialty.find("Nội") != string::npos) prefix = "KNT";
    else if (specialty.find("Truyền nhiễm") != string::npos) prefix = "KTN";
    else if (specialty.find("Nhi") != string::npos) prefix = "KNH";
    else if (specialty.find("Ngoại") != string::npos) prefix = "KNG";
    else if (specialty.find("Phụ sản") != string::npos) prefix = "KPS";
    else if (specialty.find("Liên chuyên khoa") != string::npos) prefix = "LCK";
    else if (specialty.find("Xét nghiệm") != string::npos) prefix = "KXN";
    else if (specialty.find("hình ảnh") != string::npos) prefix = "KHA";
    else if (specialty.find("Giải phẫu") != string::npos) prefix = "KGP";
    else if (specialty.find("nhiễm khuẩn") != string::npos) prefix = "KNK";
    else if (specialty.find("Dược") != string::npos) prefix = "KDU";
    else if (specialty.find("Dinh dưỡng") != string::npos) prefix = "KDD";

    medicalRecordCounters[prefix]++;
    stringstream ss;
    ss << prefix << setfill('0') << setw(4) << medicalRecordCounters[prefix];
    return ss.str();
}

// ==========================================
// LOAD DATA TỪ CSV 
// ==========================================
void Database::loadAllData() {
    cout << "Dang tai co so du lieu..." << endl;

    patientCounter = 0;
    appointmentCounter = 0;
    medicalRecordCounters.clear();

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

            string type, id, name, dob, gender, phone, recordId, discountStr;
            getline(ss, type, ';'); 
            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, dob, ';');
            getline(ss, gender, ';');
            getline(ss, phone, ';');
            getline(ss, recordId, ';');
            getline(ss, discountStr, ';');
            
            if (id.length() > 2) {
                int num = 0;
                try { num = stoi(id.substr(2)); } catch(...) {}
                if (num > patientCounter) patientCounter = num;
            }

            double discount = 0;
            try { discount = stod(discountStr); } catch (...) {}

            if (type == "NoiTru") {
                string admDate, disDate, room, bed;
                getline(ss, admDate, ';');
                getline(ss, disDate, ';');
                getline(ss, room, ';');
                getline(ss, bed, ';');
                patients.insert(id, new Inpatient(id, name, dob, gender, phone, recordId, discount, admDate, disDate, room, bed));
            } 
            else if (type == "NgoaiTru") {
                string appDate, clinicRoom;
                getline(ss, appDate, ';');
                getline(ss, clinicRoom, ';');
                patients.insert(id, new Outpatient(id, name, dob, gender, phone, recordId, discount, appDate, clinicRoom));
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
            string id, name, priceStr, dosage;

            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, priceStr, ';');
            getline(ss, dosage, ';');

            double price = 0.0;
            try { price = stod(priceStr); } catch (...) {}

            medicines.insert(id, Medicine(id, name, price, dosage));
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

            if (rId.length() >= 7) {
                string prefix = rId.substr(0, 3);
                int num = 0;
                try { num = stoi(rId.substr(3)); } catch(...) {}
                if (num > medicalRecordCounters[prefix]) {
                    medicalRecordCounters[prefix] = num;
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
                 << allMed[i]->getDosage() << "\n";
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
                     << inpat->getInsuranceDiscount() << ";"
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
                     << outpat->getInsuranceDiscount() << ";"
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
// CÁC HÀM XÓA (CẬP NHẬT RÀNG BUỘC)
// ==========================================
bool Database::deleteUserFromQT(const std::string& id) {
    if (users.search(id) == nullptr) return false; 
    users.remove(id); 
    return true;
}

int Database::deleteDoctorFromQT(const std::string& id) {
    if (doctors.search(id) == nullptr) return -1;
    
    // TÍNH NĂNG 1: BẢO TOÀN DỮ LIỆU
    // Chặn xóa nếu Bác sĩ đang có lịch khám Pending
    for (const auto& appt : appointments) {
        if (appt.getDoctorId() == id && appt.getStatus() == "Pending") {
            return -2; // Lỗi ràng buộc
        }
    }

    doctors.remove(id);
    if (users.search(id) != nullptr) { users.remove(id); }
    return 1;
}

int Database::deleteMedicineFromQT(const std::string& id) {
    if (medicines.search(id) == nullptr) return -1;

    // TÍNH NĂNG 1: BẢO TOÀN DỮ LIỆU
    // Chặn xóa nếu Thuốc đã từng được kê vào Bệnh án
    for (const auto& record : medicalRecords) {
        for (const auto& rx : record.getPrescriptions()) {
            if (rx.medicineId == id) {
                return -2; // Lỗi ràng buộc
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
// CÁC HÀM CẬP NHẬT TỪ QT (AUTO ID)
// ==========================================

bool Database::addPatientFromGUI(string name, string phone, bool isNoiTru){
    string newId = generatePatientId(); // Tự sinh mã
    if (isNoiTru){
        patients.insert(newId, new Inpatient(newId, name, "", "", phone, "", 0.0, "", "", "", ""));
    } else{
        patients.insert(newId, new Outpatient(newId, name, "", "", phone, "", 0.0, "", ""));
    }
    return true;
}

int Database::bookAppointmentFromQT(string patientId, string doctorId, string receptionistId,string specialty, string day, string time) {
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
    return 1; 
}

string Database::createMedicalRecordFromQT(string apptId, string symp, string diag, string notes) {
    Appointment * appt = findAppointmentById(apptId);
    if(appt == nullptr) return "";
    
    string specialty = appt->getSpecialty(); 
    string newRecordId = generateMedicalRecordId(specialty); 
    
    medicalRecords.push_back(MedicalRecord(newRecordId, apptId, symp, diag, notes));
    appt->setStatus("Completed");
    saveAllData(); 
    return newRecordId;
}

// ==========================================
// CÁC HÀM ĐĂNG NHẬP VÀ THÊM DỮ LIỆU CÒN LẠI
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

int Database::addMedicineFromQT(string medicineId,string medicineName,double unitPrice,string dosage ,int addQty){
    Medicine* existingMed = findMedicineById(medicineId);
    if (existingMed != nullptr){
        existingMed->setStock(existingMed->getStock() + addQty);
        return 2;
    } else {
        medicines.insert(medicineId,Medicine(medicineId, medicineName, unitPrice, dosage, addQty));
        return 1;
    }
}

int Database::addServiceFromQT(string id, string name, double price){
    if (services.search(id) != nullptr) return -1;
    services.insert(id, Service(id, name, price));
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

int Database::createInvoiceFromQT(string patientId, double& outTotalFee){
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
    if (record == nullptr) return -3;

    double medFee = 0.0; 
    for (const auto& rx : record->getPrescriptions()) {
        Medicine* med = findMedicineById(rx.medicineId);
        if (med != nullptr) {
            medFee += (med->getUnitPrice() * rx.quantity);
        }
    }

    string newInvId = "INV" + to_string(invoices.size() + 1);
    Invoice newInv(newInvId, patientId, medFee);

    string srvId = "SRV01";          
    double srvPrice = 100000.0;      
    vector<Service*> srvList = services.getAllPointers();
    if (!srvList.empty()) {
        Service* selectedSrv = srvList[0];
        for (auto s : srvList) {
            if (s->getId() == "SRV01" || s->getId() == "DV01") {
                selectedSrv = s;
                break;
            }
        }
        srvId = selectedSrv->getId();
        srvPrice = selectedSrv->getPrice();
    }

    string detailId = newInvId + "_D1";
    InvoiceDetail consultDetail(detailId, srvId, newInvId, 1, srvPrice);
    newInv.addDetail(consultDetail);

    double discount = pat->getInsuranceDiscount();
    double discountRate = (discount > 1.0) ? (discount / 100.0) : discount;
    double rawTotal = srvPrice + medFee; 
    outTotalFee = rawTotal * (1.0 - discountRate); 

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