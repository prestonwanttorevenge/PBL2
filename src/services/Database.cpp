#include "services/Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

Database::Database() {
    // Để rỗng ở đây
}

Database::~Database() {
    vector<Patient**> allPatients = patients.getAllPointers();
    // Duyệt qua mảng và tiêu hủy các vùng nhớ con (Inpatient / Outpatient)
    for (size_t i = 0; i < allPatients.size(); i++) {
        delete *(allPatients[i]); // Giải tham chiếu để gọi delete lên Patient*
    }
}
void Database::sanitizeInput(string& input) {
    for (size_t i = 0; i < input.length(); i++) {
        if (input[i] == ';') {
            input[i] = ',';
        }
    }
}

//LOAD DATA


void Database::loadAllData() {
    cout << "Dang tai co so du lieu..." << endl;

    // ==========================================
    // 1. DOCTORS (BST)
    // ==========================================
    ifstream fDoc(doctorFile);
    if (fDoc.is_open()) {
        string line;
        doctors.clear(); // Xóa sạch cây cũ
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
            
            // Thay push_back bằng insert
            doctors.insert(id, Doctor(id, name, dob, gender, phone, spec, exp));
        }
        fDoc.close();
    } else {
        cout << "- Chua co file " << doctorFile << " (Se tao moi khi luu)\n";
    }

    // Lịch trực Bác sĩ
    std::ifstream fSched("data_doctor_schedules.csv");
    if (fSched.is_open()) {
        std::string line;
        while (std::getline(fSched, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string docId, day, time;
            std::getline(ss, docId, ';');
            std::getline(ss, day, ';');
            std::getline(ss, time, ';');
            
            Doctor* doc = findDoctorById(docId); // Đã dùng cây bên trong hàm này
            if (doc != nullptr) {
                doc->setWorkSchedule(day, time);
            }
        }
        fSched.close();
    }
    
    // ==========================================
    // 2. PATIENTS (BST CON TRỎ)
    // ==========================================
    ifstream fPat(patientFile);
    if (fPat.is_open()) {
        string line;

        // Xóa dữ liệu cũ một cách an toàn để chống tràn RAM
        vector<Patient**> oldPats = patients.getAllPointers();
        for (size_t i = 0; i < oldPats.size(); i++) {
            delete *(oldPats[i]); // Giải phóng vùng nhớ con trỏ
        }
        patients.clear(); // Chặt cây

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

    // ==========================================
    // 3. INVOICES & DETAILS (GIỮ NGUYÊN VECTOR)
    // ==========================================
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
    } else {
        cout << "- Chua co file " << invoiceFile << " (Se tao moi khi luu)\n";
    }

    ifstream fDetail("data_invoice_details.csv");
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
    } else {
        cout << "- Chua co file " << invoiceDetailsFile << " (Se tao moi khi luu)\n";
    }

    // ==========================================
    // 4. USERS (BST)
    // ==========================================
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
    } else {
        cout << "- Chua co file " << userFile << " (Se tao moi khi luu)\n";
    }

    // ==========================================
    // 5. MEDICINES (BST)
    // ==========================================
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

    // ==========================================
    // 6. SERVICES (BST)
    // ==========================================
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

    // ==========================================
    // 7. APPOINTMENTS (GIỮ NGUYÊN VECTOR)
    // ==========================================
    ifstream fAppt(appointmentFile);
    if (fAppt.is_open()) {
        string line;
        appointments.clear();
        while (getline(fAppt, line)) {
            if (line.empty()) continue;
            stringstream ss(line);
            string aId, pId, dId, rId, date, status;

            getline(ss, aId, ';');
            getline(ss, pId, ';');
            getline(ss, dId, ';');
            getline(ss, rId, ';');
            getline(ss, date, ';');
            getline(ss, status, ';');

            appointments.push_back(Appointment(aId, pId, dId, rId, date, status));
        }
        fAppt.close();
    }
 
    // ==========================================
    // 8. MEDICAL RECORDS & PRESCRIPTIONS (VECTOR)
    // ==========================================
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
            medicalRecords.push_back(MedicalRecord(rId, aId, symp, diag, notes));
        }
        fMedRec.close();
    }

    ifstream fPresc("data_prescription_details.csv");
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
                    // Sửa lại cách truyền hàm addMedicine dựa trên cấu trúc cũ
                    record.addMedicine(mId, dosage, qty);
                    break;
                }
            }
        }
        fPresc.close();
    } else {
        std::cout << "- Chua co file data_prescription_details.csv" << endl;
    }

    cout << "-> Tai du lieu hoan tat!" << endl;
}


//SAVE DATA


void Database::saveAllData() {
    cout << "Dang luu co so du lieu..." << endl;
    
    // ==========================================
    // 1. SERVICES (ĐÃ CHUYỂN SANG BST)
    // ==========================================
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
                  << appt.getDayOfWeek() << ";"
                  << appt.getTimeSlot() << ";"
                  << appt.getStatus() << "\n";
        }
        fappt.close();
    }

    // ==========================================
    // 3. MEDICINES (ĐÃ CHUYỂN SANG BST)
    // ==========================================
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
    ofstream fPresc("data_prescription_details.csv"); 

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

    // ==========================================
    // 5. DOCTORS & SCHEDULES
    // ==========================================
    std::ofstream fDoc(doctorFile);
    std::ofstream fSched(doctorScheduleFile); 

    if (fDoc.is_open() && fSched.is_open()) {
        vector<Doctor*> allDocs = doctors.getAllPointers();
        
        for (size_t i = 0; i < allDocs.size(); i++) {
            // Ghi file Doctors
            fDoc << allDocs[i]->getId() << ";"
                 << allDocs[i]->getFullName() << ";"
                 << allDocs[i]->getDateOfBirth() << ";"
                 << allDocs[i]->getGender() << ";"
                 << allDocs[i]->getPhoneNumber() << ";"
                 << allDocs[i]->getSpecialty() << ";"
                 << allDocs[i]->getYearsOfExperience() << "\n";
            
            // Ghi file Schedules (Lịch trực)
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

    // ==========================================
    // 6. USERS (ĐÃ CHUYỂN SANG BST)
    // ==========================================
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
        defaultAdmin.setPassword("admin123"); // Tự băm mật khẩu mã hóa chuẩn
        users.insert("ADMIN01", defaultAdmin);
    }
    }

    // ==========================================
    // 7. PATIENTS (BST )
    // ==========================================
    ofstream fPat(patientFile);
    if (fPat.is_open()) {
        vector<Patient**> allPats = patients.getAllPointers();
        for (size_t i = 0; i < allPats.size(); i++) {
            // Phá vỏ con trỏ cấp 2 để lấy con trỏ Patient* thật
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
Doctor* Database::findDoctorById(string id) {
    return doctors.search(id);
}

// 2. Tìm Bệnh nhân
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
        string id = pat->getId();
        string name = pat->getFullName();
        // Nếu ID khớp chính xác HOẶC Tên có chứa từ khóa
        if (id == keyword || name.find(keyword) != string::npos) {
            results.push_back(pat);
        }
    }
    
    return results;
}
// 3. Tìm Thuốc
Medicine* Database::findMedicineById(string id) {
    return medicines.search(id);
}
// 4.tim cuoc hen
Appointment* Database::findAppointmentById(string id) {
    for (size_t i = 0; i < appointments.size(); i++) {
        if (appointments[i].getAppointmentId() == id) {
            return &appointments[i];
        }
    }
    return nullptr;
}
// 5.tim benh an
MedicalRecord* Database::findMedicalRecordById(string id){
    for (size_t i = 0;i < medicalRecords.size();i++) {
        if (medicalRecords[i].getRecordId() == id) {
            return &medicalRecords[i];
        }
    }
    return nullptr;
}
void Database::deleteDoctor(string id) {
    // Gọi thẳng hàm remove của Cây. Nếu xóa thành công nó sẽ trả về true.
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
    // BƯỚC 1: Phải tìm được con trỏ Patient* đang giữ vùng nhớ (new)
    Patient* pat = findPatientById(id);
    
    if (pat != nullptr) {
        // BƯỚC 2: Gọi delete để tiêu hủy vùng nhớ Inpatient/Outpatient
        delete pat; 
        
        // BƯỚC 3: Xóa cái Node (lá cây) đang chứa cái con trỏ rỗng đó đi
        patients.remove(id); 
        
        cout << "=> Da xoa Benh nhan thanh cong!\n";
    } else {
        cout << "Loi: Khong tim thay ID Benh nhan!\n";
    }
}
void Database::updateDoctor(string id) {
    Doctor* doc = findDoctorById(id);
    
    if (doc == nullptr) {
        cout << "Loi: Khong tim thay Bac si co ID: " << id << "\n";
        return;
    }

    cout << "--- CAP NHAT THONG TIN BAC SI: " << doc->getFullName() << " ---\n";
    cout << "(Nhan Enter de bo qua neu khong muon doi thong tin do)\n";

    string input;

    cout << "So dien thoai moi (Hien tai: " << doc->getPhoneNumber() << "): ";
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
    cout << "=> Cap nhat thong tin thanh cong!\n";
}
void Database::updateMedicine(string id) {
    Medicine* med = findMedicineById(id);
    
    if (med == nullptr) {
        cout << "Loi: Khong tim thay Thuoc!\n";
        return;
    }

    string input;
    cout << "Gia moi (Hien tai: " << med->getUnitPrice() << "): ";
    getline(cin, input);
    sanitizeInput(input);
    
    if (!input.empty()) {
        try {
            double newPrice = stod(input);
            med->setUnitPrice(newPrice);
        } catch (...) {
            cout << "Gia tien khong hop le, giu nguyen gia cu.\n";
        }
    }
    cout << "=> Cap nhat thuoc thanh cong!\n";
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

    cout << "Ma Bao Hiem Y Te moi (Hien tai: " << pat->getMedicalRecordId() << "): ";
    getline(cin, input);
    sanitizeInput(input);
    if (!input.empty()) {
        pat->setMedicalRecordId(input); 
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
    string id, name, dob, gender, phone, recordId, discountStr;
    
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

    cout << "Nhap Phan tram giam gia BHYT (VD: 0.8 cho 80%): ";
    getline(cin, discountStr); sanitizeInput(discountStr);
    double discount = 0.0;
    try { discount = stod(discountStr); } catch (...) { discount = 0.0; }


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
        patients.insert(id,new Inpatient(id, name, dob, gender, phone, recordId, discount, admDate, disDate, room, bed));
        cout << "=> Da them Benh nhan Noi tru thanh cong!\n";

    } else if (choice == "2") {
        string apptDate, clinic;
        cout << "Nhap Ngay hen kham: "; getline(cin, apptDate); sanitizeInput(apptDate);
        cout << "Nhap Phong kham (VD: Khoa Noi, Phong 102): "; getline(cin, clinic); sanitizeInput(clinic);
        patients.insert(id,new Outpatient(id, name, dob, gender, phone, recordId, discount, apptDate, clinic));
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

bool Database::addInpatientFromQT(string id, string name, string dob, string gender, string phone, string recordId, double discount, string admDate, string disDate, string room, string bed) {
    if (findPatientById(id) != nullptr) return false;
    patients.insert(id,new Inpatient(id, name, dob, gender, phone, recordId, discount, admDate, disDate, room, bed));
    return true;
}

bool Database::addOutpatientFromQT(string id, string name, string dob, string gender, string phone, string recordId, double discount, string apptDate, string clinic) {
    if (findPatientById(id) != nullptr) return false;
    patients.insert(id,new Outpatient(id, name, dob, gender, phone, recordId, discount, apptDate, clinic));
    return true;
}
bool Database::addDoctorFromQT(string id, string fullName, string dob, string gender, string phone, string specialty, int expYears){
    if(findDoctorById(id) !=nullptr) return false;
    doctors.insert(id,Doctor(id, fullName, dob ,gender ,phone , specialty,expYears));
    return true; 
}
bool Database::addShiftToDoctorFromQT(string doctorId, string day, string time) {
    Doctor* doc = findDoctorById(doctorId);
    if (doc != nullptr) {
        doc->setWorkSchedule(day, time);
        return true;
    }
    return false;
}
bool Database::addPatientFromGUI(string id, string name, string phone, bool isNoiTru){
    if (findPatientById(id) != nullptr){
        return false;
    }
    if (isNoiTru){
        patients.insert(id, new Inpatient(id, name, "", "", phone, "", 0.0, "", "", "", ""));
    } else{
        patients.insert(id, new Outpatient(id, name, "", "", phone, "", 0.0, "", ""));
    }
    return true;
}
 int Database::addMedicineFromQT(string medicineId,string medicineName,double unitPrice,string dosage ,int addQty){
    Medicine* existingMed = findMedicineById(medicineId);
    
    if (existingMed != nullptr){
        int newStock = existingMed->getStock() + addQty;
        existingMed->setStock(newStock);
        return 2;
    } 
    else {
        medicines.insert(medicineId,Medicine(medicineId, medicineName, unitPrice, dosage, addQty));
        
        return 1;
    }
 }
int Database::addServiceFromQT(string id, string name, double price){
    if (services.search(id) != nullptr){
        return -1;
    }
    services.insert(id, Service(id, name, price));
    return 1;
}
vector<Doctor*> Database::getAvailableDoctors(string day, string shift) {
    vector<Doctor*> availableDocs;
    // 1. Kéo toàn bộ danh bạ con trỏ Bác sĩ từ Cây ra
    vector<Doctor*> allDocs = doctors.getAllPointers();
    // 2. Duyệt qua mảng con trỏ này
    for (size_t i = 0; i < allDocs.size(); i++) {
        for (const auto& w : allDocs[i]->getSchedule()) {
            if (w.dayOfWeek == day && w.timeSlot == shift) {
                // allDocs[i] bản chất đã là Doctor*, nên nhét thẳng vào mảng luôn
                availableDocs.push_back(allDocs[i]);
                break;
            }
        }
    }
    
    return availableDocs;
}
int Database::bookAppointmentFromQT(string patientId, string doctorId, string receptionistId, string day, string time) {
    if (findPatientById(patientId) == nullptr) {
        return -1; // Lỗi: Không tìm thấy Bệnh nhân
    }
    Doctor* doc = findDoctorById(doctorId);
    if (doc == nullptr) {
        return -2; // Lỗi: Không tìm thấy Bác sĩ
    }
    bool isAvailable = false;
    for (const auto& shift : doc->getSchedule()) {
        if (shift.dayOfWeek == day && shift.timeSlot == time) {
            isAvailable = true;
            break;
        }
    }
    if (!isAvailable) {
        return -3;
    }
    string newAppId = "APP" + to_string(appointments.size() + 1);
    appointments.push_back(Appointment(newAppId, patientId, doctorId, receptionistId, day, time, "Cho Kham"));
    return 1; // Thành công
}
int Database::createMedicalRecordFromQT(string apptId, string symp, string diag, string notes) {
    if(findAppointmentById(apptId) == nullptr) return -1;
    // Tự động sinh mã Bệnh án mới (VD: MR1, MR2)
    string newRecordId = "MR" + to_string(medicalRecords.size() + 1);
    medicalRecords.push_back(MedicalRecord(newRecordId, apptId, symp, diag, notes));

    return 1;
}
int Database::prescribeMedicineFromQT(string recordId, string medId, string dosage, int qty) {
    MedicalRecord* record = findMedicalRecordById(recordId);
    if (record == nullptr) return -1;

    // 2. Kiểm tra Thuốc có trong kho không
    Medicine* med = findMedicineById(medId);
    if (med == nullptr) return -2;

    // 3. Kiểm tra số lượng tồn kho (stock) có đủ để kê không
    if (med->getStock() < qty) return -3;

    // 4. Mọi thứ OK -> Trừ tồn kho
    med->setStock(med->getStock() - qty);

    // 5. Thêm thuốc vào mảng prescriptions của đối tượng MedicalRecord
    record->addMedicine(medId, dosage, qty);

    return 1;
}
int Database::createInvoiceFromQT(string patientId, double& outTotalFee) {
    // 1. Kiểm tra Bệnh nhân có thật không
    Patient* pat = findPatientById(patientId);
    if (pat == nullptr) return -1;

    // 2. TRUY VẾT 1: Tìm Lịch hẹn mới nhất của Bệnh nhân này
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

    // 3. BẮT ĐẦU TÍNH TIỀN TỰ ĐỘNG (Phí khám 100k + Tiền thuốc)
    double totalFee = 100000; 
    for (const auto& rx : record->getPrescriptions()) {
        Medicine* med = findMedicineById(rx.medicineId);
        if (med != nullptr) {
            totalFee += (med->getUnitPrice() * rx.quantity);
        }
    }

    // 5. Khấu trừ Bảo Hiểm Y Tế
    double discount = pat->getInsuranceDiscount();
    double finalFee = totalFee * (1.0 - discount); 
    
    outTotalFee = finalFee; // Ném tiền ra cho giao diện

    // 6. Tạo Hóa đơn và lưu lại
    string newInvId = "INV" + to_string(invoices.size() + 1);
    invoices.push_back(Invoice(newInvId, patientId, finalFee));

    return 1;
}
vector<MedicalRecord*> Database::getPatientHistory(string patientId) {
    vector<MedicalRecord*> history;

    // 1. Quét toàn bộ mảng Cuộc hẹn
    for (size_t i = 0; i < appointments.size(); i++) {
        // Nếu tìm thấy lịch hẹn của bệnh nhân này
        if (appointments[i].getPatientId() == patientId) {
            string apptId = appointments[i].getAppointmentId();

            // 2. Chui vào mảng Bệnh án tìm cái tờ Bệnh án gắn với lịch hẹn đó
            for (size_t j = 0; j < medicalRecords.size(); j++) {
                if (medicalRecords[j].getAppointmentId() == apptId) {
                    // Tìm thấy thì nhét vào danh sách kết quả
                    history.push_back(&medicalRecords[j]);
                    break;
                }
            }
        }
    }
    
    // Trả về danh sách tất cả các bệnh án đã gom được
    return history;
}
//login 
    User* Database::loginFromQT(string username, string password) {
    // CHỐT CHẶN AN TOÀN: Nếu hệ thống chưa có tài khoản nào, tự tạo Admin ngay tại chỗ
    if (users.isEmpty() && username == "admin" && password == "admin123") {
        User defaultAdmin("ADMIN01", "admin", "", "Quan Tri Vien", "Admin", true);
        defaultAdmin.setPassword("admin123");
        users.insert("ADMIN01", defaultAdmin);
        return users.search("ADMIN01");
    }
       vector<User*> allusers = users.getAllPointers();
    for (size_t i = 0; i < allusers.size(); i++) {
        if (allusers[i]->getUsername() == username) {
            
            // 2. Tìm thấy tài khoản -> Gọi hàm authenticate (bcrypt)
            if (allusers[i]->authenticate(password)) {
                return allusers[i];
            } else {
                return nullptr; 
            }
        }
    }
    return nullptr; // Không tìm thấy Username trên hệ thống
}
// Hàm này nằm trong Database.cpp
int Database::addUserFromQT(string id, string username, string plainPassword, string fullName, string role) {
    // 1. Kiểm tra trùng lặp ID: Tận dụng hàm search của Cây để tìm siêu tốc O(log n)
    if (users.search(id) != nullptr) {
        return -1; // Báo lỗi trùng lặp ID
    }
    // 2. Kiểm tra trùng lặp Username: Cây bị mù với Username nên phải quét danh bạ O(n)
    vector<User*> allUsers = users.getAllPointers();
    for (size_t i = 0; i < allUsers.size(); i++) {
        if (allUsers[i]->getUsername() == username) {
            return -1; // Báo lỗi trùng lặp Username
        }
    }
    // 3. Khởi tạo đối tượng User mới. Tạm thời để trống phần passwordHash
    User newUser(id, username, "", fullName, role, true);
    newUser.setPassword(plainPassword);
    // 4. Đưa User mới lên Cây
    users.insert(id, newUser); 
    return 1;
}
int Database::addDoctorWithAccount(string id, string name, string dob, string gender, string phone, string specialty, int expYears, string username, string plainPassword) {
   
     if (doctors.search(id) != nullptr) {
        return -1; // Báo lỗi trùng lặp ID
    }
    // 2. Kiểm tra trùng lặp Username: Cây bị mù với Username nên phải quét danh bạ O(n)
    vector<Doctor*> allDocs = doctors.getAllPointers();
    for (size_t i = 0; i < allDocs.size(); i++) {
        if (allDocs[i]->getFullName() == username) {
            return -2; // Báo lỗi trùng lặp Username
        }
    }

    // 2. LỆNH PUSH_BACK ĐÃ ĐƯỢC SỬA LỖI (Truyền đủ 7 tham số)
    doctors.insert(id,Doctor(id, name, dob, gender, phone, specialty, expYears));

    // 3. TẠO TÀI KHOẢN KÈM BĂM MẬT KHẨU
    User newAccount(id, username, "", name, "Doctor", true); 
    newAccount.setPassword(plainPassword); 
    users.insert(id,newAccount);

    return 1;
}