#include "ui/DoctorMenu.h"
#include "ui_DoctorMenu.h"
#include "ui/Login.h"
#include "services/Database.h"
#include <QMessageBox>
#include <QTableWidgetItem>

DoctorMenu::DoctorMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DoctorMenu), db(database), auth(authService) {
    ui->setupUi(this);
    loadPatientQueue();
    loadMedicineComboBox(); // Nạp thuốc vào ComboBox ngay khi mở
}

DoctorMenu::~DoctorMenu() {
    delete ui;
}

void DoctorMenu::loadMedicineComboBox() {
    ui->cbMedicine->clear();
    vector<Medicine*> meds = db.getMedicines().getAllPointers();
    if (meds.empty()) {
        ui->cbMedicine->addItem("Không có thuốc trong kho");
        ui->cbMedicine->setEnabled(false);
    } else {
        for (auto med : meds) {
            QString displayText = QString::fromStdString(med->getMedicineId() + " - " + med->getMedicineName() + " (Tồn: " + std::to_string(med->getStock()) + ")");
            ui->cbMedicine->addItem(displayText, QString::fromStdString(med->getMedicineId()));
        }
        ui->cbMedicine->setEnabled(true);
    }
}

void DoctorMenu::on_btnLogout_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Đăng xuất", "Bác sĩ có chắc chắn muốn đăng xuất?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        Login *loginForm = new Login(this->db);
        loginForm->show();
        this->close(); 
    }
}

void DoctorMenu::on_btnViewSchedule_clicked() {
    User* currentUser = auth.getCurrentUser();
    if (currentUser == nullptr) {
        QMessageBox::critical(this, "Lỗi", "Phiên đăng nhập không hợp lệ!");
        return;
    }
    string currentDoctorId = currentUser->getId();

    Doctor* doc = db.findDoctorById(currentDoctorId);
    if (doc == nullptr) {
        QMessageBox::critical(this, "Lỗi", "Không tìm thấy dữ liệu của bạn trên hệ thống!");
        return;
    }
    vector<WorkShift> schedule = doc->getSchedule();

    if (schedule.empty()) {
        QMessageBox::information(this, "Lịch Trực", "Tuần này bạn chưa được xếp ca trực nào.");
        return;
    }
    QString msg = "Lịch trực của Bác sĩ " + QString::fromStdString(doc->getFullName()) + ":\n\n";
    
    for (size_t i = 0; i < schedule.size(); i++) {
        msg += "- " + QString::fromStdString(schedule[i].dayOfWeek) 
             + ", Ca: " + QString::fromStdString(schedule[i].timeSlot) + "\n";
    }
    
    QMessageBox::information(this, "Lịch Trực", msg);
}

void DoctorMenu::on_btnCreateMedicalRecord_clicked() {
    string apptId = ui->txtAppointmentId->text().toStdString();
    string symp = ui->txtSymptoms->text().toStdString();
    string diag = ui->txtDiagnosis->text().toStdString();
    string notes = ui->txtNotes->text().toStdString();

    if (apptId.empty() || diag.empty()) {
        QMessageBox::warning(this, "Thiếu thông tin", "Mã Lịch hẹn và Chẩn đoán không được để trống!");
        return;
    }

    int result = db.createMedicalRecordFromQT(apptId, symp, diag, notes);

    if (result == 1) {
        db.saveAllData(); // Lưu CSDL
        QMessageBox::information(this, "Thành công", "Đã tạo Hồ sơ Bệnh án mới thành công!");
        ui->txtAppointmentId->clear();
        ui->txtSymptoms->clear();
        ui->txtDiagnosis->clear();
        ui->txtNotes->clear();
        loadPatientQueue();
    } else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Lịch hẹn không hợp lệ!");
    }
}

void DoctorMenu::on_btnPrescribeMedicine_clicked() {
    string recordId = ui->txtRecordId->text().toStdString();
    string medId = ui->cbMedicine->currentData().toString().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    int qty = ui->spinBoxQuantity->value();

    if (recordId.empty() || medId.empty() || qty <= 0) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đủ thông tin và số lượng lớn hơn 0.");
        return;
    }

    int result = db.prescribeMedicineFromQT(recordId, medId, dosage, qty);

    if (result == 1) {
        db.saveAllData(); // Lưu ngay xuống CSV
        QMessageBox::information(this, "Kê đơn", "Đã thêm thuốc vào Bệnh án thành công!");
        ui->txtDosage->clear();
        ui->spinBoxQuantity->setValue(1);
        loadMedicineComboBox(); // Làm mới lại số lượng tồn kho trên ComboBox
    } 
    else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh án không tồn tại!");
    } 
    else if (result == -2) {
        QMessageBox::critical(this, "Lỗi", "Thuốc không tồn tại trên hệ thống!");
    } 
    else if (result == -3) {
        QMessageBox::critical(this, "Hết hàng", "Số lượng thuốc trong kho không đủ để kê đơn này!");
    }
}

void DoctorMenu::on_btnViewPatientHistory_clicked() {
    string patientId = ui->txtPatientId_History->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng nhập Mã Bệnh Nhân để tra cứu!");
        return;
    }

    if (db.findPatientById(patientId) == nullptr) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh nhân không tồn tại trên hệ thống!");
        return;
    }

    vector<MedicalRecord*> history = db.getPatientHistory(patientId);

    if (history.empty()) {
        QMessageBox::information(this, "Lịch Sử", "Bệnh nhân này chưa từng khám hoặc chưa có bệnh án nào được ghi nhận.");
        return;
    }

    QString msg = "LỊCH SỬ KHÁM CỦA BỆNH NHÂN: " + QString::fromStdString(patientId) + "\n\n";
    
    for (size_t i = 0; i < history.size(); i++) {
        msg += "--- Lần khám thứ " + QString::number(i + 1) + " ---\n";
        msg += "+ Mã Bệnh Án: " + QString::fromStdString(history[i]->getRecordId()) + "\n";
        msg += "+ Triệu chứng: " + QString::fromStdString(history[i]->getSymptoms()) + "\n";
        msg += "+ Chẩn đoán: " + QString::fromStdString(history[i]->getDiagnosis()) + "\n";
        msg += "+ Ghi chú: " + QString::fromStdString(history[i]->getNotes()) + "\n";
        msg += "+ Số loại thuốc đã kê: " + QString::number(history[i]->getPrescriptions().size()) + " loại\n";
        msg += "--------------------------\n\n";
    }

    QMessageBox::information(this, "Lịch Sử Khám Bệnh", msg);
    ui->txtPatientId_History->clear();
}

void DoctorMenu::on_btnCancelAppointment_clicked() {
    int row = ui->tableAppointments->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một lịch hẹn trên bảng để hủy!");
        return;
    }

    string apptId = ui->tableAppointments->item(row, 0)->text().toStdString();

    auto reply = QMessageBox::question(this, "Xác nhận Hủy", 
                                       "Bệnh nhân vắng mặt? Bạn có chắc muốn Hủy lịch hẹn này không?", 
                                       QMessageBox::Yes | QMessageBox::No);
                                       
    if (reply == QMessageBox::Yes) {
        int result = db.cancelAppointmentFromQT(apptId);
        
        if (result == 1) {
            QMessageBox::information(this, "Thành công", "Đã hủy lịch hẹn thành công!");
            loadPatientQueue(); 
        } 
        else if (result == -2) {
            QMessageBox::warning(this, "Lỗi", "Lịch hẹn này đã khám xong, không thể hủy!");
        }
    }
}

void DoctorMenu::loadPatientQueue() {
    ui->tableAppointments->setColumnCount(4);
    ui->tableAppointments->setHorizontalHeaderLabels({"Mã Lịch Hẹn", "Mã Bệnh Nhân", "Thời Gian", "Trạng Thái"});
    ui->tableAppointments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableAppointments->setRowCount(0);
    auto& appts = db.getAppointments();
    
    int row = 0;
    for (const auto& appt : appts) {
        if (appt.getStatus() == "Pending") {
            ui->tableAppointments->insertRow(row);
            ui->tableAppointments->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(appt.getAppointmentId())));
            ui->tableAppointments->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(appt.getPatientId())));
            ui->tableAppointments->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(appt.getDate())));
            ui->tableAppointments->setItem(row, 3, new QTableWidgetItem(QString::fromUtf8("Chờ Khám")));
            row++;
        }
    }
}