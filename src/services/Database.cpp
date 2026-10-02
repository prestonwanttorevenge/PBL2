#include "Database.h"
#include <fstream>
#include <sstream>
#include <iostream>
Database::~Database() {
    for (int i = 0; i < patients.size(); i++) {
        delete patients[i];
    }
    patients.clear();
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
    ifstream fDoc(doctorFile);
    if (fDoc.is_open()) {
        string line;
        doctors.clear();
        while (getline(fDoc, line)) {
            if (line.empty()) continue;
            
            stringstream ss(line);
            string id, name, dob, gender, phone, spec, tempExp, schedule;

            getline(ss, id, ';');
            getline(ss, name, ';');
            getline(ss, dob, ';');
            getline(ss, gender, ';');
            getline(ss, phone, ';');
            getline(ss, spec, ';');
            getline(ss, tempExp, ';');
            getline(ss, schedule, ';');

            int exp = 0;
            try { exp = stoi(tempExp); } catch (...) {}
            doctors.push_back(Doctor(id, name, dob, gender, phone, spec, exp, schedule));
        }
        fDoc.close();
    } else {
        cout << "- Chua co file " << doctorFile << " (Se tao moi khi luu)\n";
    }
    ifstream fPat(patientFile);
    if (fPat.is_open()) {
        string line;

        for(auto p : patients) delete p; 
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
            
            double discount = 0;
            try { discount = stod(discountStr); } catch (...) {}

            if (type == "NoiTru") {
                string admDate, disDate, room, bed;
                getline(ss, admDate, ';');
                getline(ss, disDate, ';');
                getline(ss, room, ';');
                getline(ss, bed, ';');
    
                patients.push_back(new Inpatient(id, name, dob, gender, phone, recordId, discount, admDate, disDate, room, bed));
            } 
            else if (type == "NgoaiTru") {
                string appDate, clinicRoom;
                getline(ss, appDate, ';');
                getline(ss, clinicRoom, ';');
                
                patients.push_back(new Outpatient(id, name, dob, gender, phone, recordId, discount, appDate, clinicRoom));
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
    else {
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
    }
    else {
        cout << "- Chua co file " << invoiceDetailsFile << " (Se tao moi khi luu)\n";
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
            users.push_back(User(id, Username, pass, fullName, role, isActive));
        }
        fDoc.close();
    } else {
        cout << "- Chua co file " << userFile << " (Se tao moi khi luu)\n";
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

            medicines.push_back(Medicine(id, name, price, dosage));
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

            services.push_back(Service(id, name, price));
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

            PrescriptionDetail detail = {mId, dosage, qty};

            for (auto& record : medicalRecords){
                if (record.getRecordId() == rId){
                    record.addMedicine(detail.medicineId, detail.dosage, detail.quantity);
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
    
    ofstream fsrv(serviceFile);
    if (fsrv.is_open()) {
        for (const auto& srv : services) {
            fsrv << srv.getId()
                    << srv.getServiceName()
                    << srv.getPrice();
        }
        fsrv.close();
    }
    ofstream fappt(appointmentFile);
    if (fappt.is_open()) {
        for (const auto& appt : appointments) {
            fsrv << appt.getAppointmentId()
                    << appt.getPatientId()
                    << appt.getDoctorId()
                    << appt.getReceptionistId()
                    << appt.getAppointmentDate()
                    << appt.getStatus();
        fsrv.close();
    }
}
    ofstream fmed(medicineFile);
    if (fmed.is_open()) {
        for (const auto& med : medicines) {
            fmed << med.getMedicineId()
                    << med.getMedicineName()
                    << med.getUnitPrice()
                    << med.getDosage();
        fmed.close();
    }
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
    ofstream fDoc(doctorFile);
     if (fDoc.is_open()) {
        for (const auto& doc : doctors) {
            fDoc  << doc.getId() << ";"
                    << doc.getFullName() << ";"
                    << doc.getDateOfBirth() << ";"
                    << doc.getGender() << ";"
                    << doc.getPhoneNumber() << ";"
                    << doc.getSpecialty() << ";"
                    << doc.getYearsOfExperience() << ";"
                    << doc.getWorkSchedule() << ";";
        }
        fDoc.close();
    }
    ofstream fUser(userFile);
    if (fUser.is_open()) {
        for (const auto& u : users) {
            fUser << u.getId() << ";"
                  << u.getUsername() << ";"
                  << u.getPassword() << ";"
                  << u.getFullName() << ";"
                  << u.getRole() << ";"
                  // Ép kiểu bool (true/false) thành chuỗi ("1"/"0")
                  << (u.getIsActive() ? "1" : "0") << "\n";
        }
        fUser.close();
    }
ofstream fPat(patientFile);
    if (fPat.is_open()) {
        for (const auto& pat : patients) {
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
            // Ép kiểu xuống Ngoại trú để lấy các biến riêng (Ngày hẹn, Phòng khám)
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
    for (size_t i = 0; i < doctors.size(); i++) {
        if (doctors[i].getId() == id) {
            return &doctors[i];
        }
    }
    return nullptr;
}

// 2. Tìm Bệnh nhân
Patient* Database::findPatientById(string id) {
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i]->getId() == id) {
            return patients[i];
        }
    }
    return nullptr;
}

// 3. Tìm Thuốc
Medicine* Database::findMedicineById(string id) {
    for (size_t i = 0; i < medicines.size(); i++) {
        if (medicines[i].getMedicineId() == id){
            return &medicines[i];
        }
    }
    return nullptr;
}
void Database::deleteDoctor(string id) {
    for (size_t i = 0; i < doctors.size(); i++) {
        if (doctors[i].getId() == id) {
            // Xóa phần tử tại vị trí thứ i
            doctors.erase(doctors.begin() + i);
            cout << "=> Da xoa Bac si thanh cong!\n";
            return;
        }
    }
    cout << "Loi: Khong tim thay ID Bac si nay!\n";
}

void Database::deleteMedicine(string id) {
    for (size_t i = 0; i < medicines.size(); i++) {
        if (medicines[i].getMedicineId() == id) {
            medicines.erase(medicines.begin() + i);
            cout << "=> Da xoa Thuoc thanh cong!\n";
            return;
        }
    }
    cout << "Loi: Khong tim thay ID Thuoc!\n";
}
void Database::deletePatient(string id){
    for (size_t i = 0; i < patients.size(); i++) {
        if (patients[i]->getId() == id) {
            delete patients[i]; 
            patients.erase(patients.begin() + i); 
            cout << "=> Da xoa Benh nhan thanh cong!\n";
            return;
        }
    }
    cout << "Loi: Khong tim thay ID Benh nhan!\n";
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

    cout << "Lich lam viec moi (Hien tai: " << doc->getWorkSchedule() << "): ";
    getline(cin, input);
    sanitizeInput(input);
    if (!input.empty()) {
        doc->setWorkSchedule(input);
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