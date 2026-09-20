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
        cout << "- Chua co file " << invoiceFile << " (Se tao moi khi luu)\n";
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

    ifstream fMed(medicinFile);
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
    // ==========================================
    // ĐỌC FILE MEDICAL RECORD (Bệnh án)
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
            // Cột 1: Mã bệnh án (Dùng để dò tìm xem thuốc này của ai)
            getline(ss, rId, ';');       
            // Các cột còn lại: Thông tin thuốc
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