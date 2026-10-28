#include "services/Database.h"
#include "utils/Constants.h"
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// ==========================================
// CÁC HÀM TÌM KIẾM, QUẢN LÝ TÀI KHOẢN VÀ DANH MỤC
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
// CÁC HÀM XÓA & CẬP NHẬT TỪ QT
// ==========================================
bool Database::deleteUserFromQT(const std::string& id) {
    User* user = users.search(id);
    if (user == nullptr || user->getRole() == "Doctor" || user->getRole() == "BacSi") return false;
    if (user->getRole() == "Admin" || user->getRole() == "QuanTri") {
        int activeAdmins = 0;
        for (User* candidate : users.getAllPointers()) {
            if ((candidate->getRole() == "Admin" || candidate->getRole() == "QuanTri") &&
                candidate->getIsActive()) {
                ++activeAdmins;
            }
        }
        // Luôn giữ lại ít nhất một Admin đang hoạt động.
        if (user->getIsActive() && activeAdmins <= 1) return false;
    }
    users.remove(id); 
    return true;
}

bool Database::setUserActiveFromQT(const std::string& id, bool active) {
    User* user = users.search(id);
    if (user == nullptr) return false;
    if (!active && (user->getRole() == "Admin" || user->getRole() == "QuanTri") &&
        user->getIsActive()) {
        int activeAdmins = 0;
        for (User* candidate : users.getAllPointers()) {
            if ((candidate->getRole() == "Admin" || candidate->getRole() == "QuanTri") &&
                candidate->getIsActive()) {
                ++activeAdmins;
            }
        }
        // Không để thao tác khóa làm mất Admin hoạt động cuối cùng.
        if (activeAdmins <= 1) return false;
    }
    user->setIsActive(active);
    return true;
}

int Database::deleteDoctorFromQT(const std::string& id) {
    if (doctors.search(id) == nullptr) return -1;
    for (const auto& appt : appointments) {
        if (appt.getDoctorId() == id) {
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
    for (const MedicalRecord& record : medicalRecords) {
        for (const string& assignedService : record.getAssignedServices()) {
            if (assignedService == id) return false;
        }
    }
    for (const Invoice& invoice : invoices) {
        for (const InvoiceDetail& detail : invoice.getDetails()) {
            if (detail.getServiceId() == id) return false;
        }
    }
    services.remove(id);
    return true;
}

// ==========================================
// CÁC HÀM XỬ LÝ NGHIỆP VỤ (LỄ TÂN & BÁC SĨ)
// ==========================================

User* Database::loginFromQT(string username, string password) {
    if (!User::isValidUsername(username)) return nullptr;
    if (users.isEmpty() && username == "admin" && password == "admin123") {
        User defaultAdmin("ADMIN01", "admin", "", "Quan Tri Vien", "Admin", true);
        defaultAdmin.setPassword("admin123");
        users.insert("ADMIN01", defaultAdmin);
        return users.search("ADMIN01");
    }
    vector<User*> allusers = users.getAllPointers();
    for (size_t i = 0; i < allusers.size(); i++) {
        if (allusers[i]->getUsername() == username) {
            if (allusers[i]->getIsActive() && allusers[i]->authenticate(password)) return allusers[i];
            else return nullptr; 
        }
    }
    return nullptr; 
}

int Database::addUserFromQT(string id, string username, string plainPassword, string fullName, string role) {
    if (id.empty() || !User::isValidUsername(username) || plainPassword.empty() || fullName.empty() ||
        users.search(id) != nullptr) return -1;
    vector<User*> allUsers = users.getAllPointers();
    for (size_t i = 0; i < allUsers.size(); i++) {
        if (allUsers[i]->getUsername() == username) return -1; 
    }
    User newUser(id, username, "", fullName, role, true);
    newUser.setPassword(plainPassword);
    users.insert(id, newUser); 
    return 1;
}

int Database::addDoctorWithAccount(string id, string name, string dob, string gender, string phone,
                                   string specialty, int expYears, string username, string plainPassword,
                                   string workDay, string workShift) {
    if (id.empty() || name.empty() || dob.empty() || gender.empty() || phone.empty() ||
        !Constants::isMedicalSpecialty(specialty) || expYears < 0 ||
        !User::isValidUsername(username) || plainPassword.empty() ||
        workDay.empty() || workShift.empty() || doctors.search(id) != nullptr ||
        users.search(id) != nullptr) return -1;
    vector<User*> allUsers = users.getAllPointers();
    for (User* user : allUsers) {
        if (user->getUsername() == username) return -2;
    }
    Doctor doctor(id, name, dob, gender, phone, specialty, expYears);
    doctor.setWorkSchedule(workDay, workShift);
    doctors.insert(id, doctor);
    User newAccount(id, username, "", name, "Doctor", true);
    newAccount.setPassword(plainPassword);
    users.insert(id, newAccount);
    return 1;
}

int Database::addMedicineFromQT(string prefix, string medicineName, double unitPrice, string dosage, int addQty, string& outId) {
    if ((prefix != "B1" && prefix != "B2" && prefix != "B3" && prefix != "DV") ||
        medicineName.empty() || dosage.empty() || !std::isfinite(unitPrice) ||
        unitPrice < 0.0 || addQty <= 0) return -2;
    vector<Medicine*> allMeds = medicines.getAllPointers();
    for (auto med : allMeds) {
        if (med->getMedicineName() == medicineName) {
            if (med->getMedicineId().rfind(prefix + "_", 0) != 0) return -3;
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
    if ((prefix != "S1" && prefix != "S2" && prefix != "S3") ||
        name.empty() || !std::isfinite(price) || price < 0.0) return -2;
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
    vector<const Doctor*> allDocs = doctors.getAllPointers();
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
    vector<const Medicine*> allMeds = medicines.getAllPointers();
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
    vector<Patient* const*> allPats = patients.getAllPointers();
    for (size_t i = 0; i < allPats.size(); i++) {
        cout << i + 1 << ". ";
        (*allPats[i])->displayData();
    }
}

// ==========================================
// THUẬT TOÁN QUẢN LÝ KHÔNG GIAN BỆNH VIỆN TỪ QT
// ==========================================
