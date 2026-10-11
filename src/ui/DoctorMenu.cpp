#include "../../include/ui/DoctorMenu.h"
#include "ui_DoctorMenu.h"
#include <QMessageBox>
#include "../../include/services/Database.h"

DoctorMenu::DoctorMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DoctorMenu), db(database), auth(authService) {
    ui->setupUi(this);
    loadPatientQueue();
}

DoctorMenu::~DoctorMenu() {
    delete ui;
}

void DoctorMenu::on_btnLogout_clicked() {
    auth.logout();
    QMessageBox::information(this, "Đăng xuất", "Bác sĩ đã đăng xuất an toàn!");
    this->close();
}

void DoctorMenu::on_btnViewSchedule_clicked() {
    QMessageBox::information(this, "Lịch Trực", "Hiển thị lịch trực của bác sĩ trong tuần...");
    string currentDoctorId = "BS01"; 

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
    // 1. Lấy dữ liệu từ giao diện
    string apptId = ui->txtAppointmentId->text().toStdString();
    string symp = ui->txtSymptoms->text().toStdString();
    string diag = ui->txtDiagnosis->text().toStdString();
    string notes = ui->txtNotes->text().toStdString();

    if (apptId.empty() || diag.empty()) {
        QMessageBox::warning(this, "Thiếu thông tin", "Mã Lịch hẹn và Chẩn đoán không được để trống!");
        return;
    }

    // 2. Gọi Backend
    int result = db.createMedicalRecordFromQT(apptId, symp, diag, notes);

    // 3. Xử lý kết quả
    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã tạo Hồ sơ Bệnh án mới thành công!");
        ui->txtAppointmentId->clear();
        ui->txtSymptoms->clear();
        ui->txtDiagnosis->clear();
        ui->txtNotes->clear();
    } else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Lịch hẹn không hợp lệ!");
    }
}
void DoctorMenu::on_btnPrescribeMedicine_clicked() {
    QMessageBox::information(this, "Kê Đơn Thuốc", "Mở danh sách thuốc để kê đơn.");
    string recordId = ui->txtRecordId->text().toStdString(); // Mã bệnh án Bác sĩ đang thao tác
    string medId = ui->cbMedicine->currentData().toString().toStdString(); // Lấy ID Thuốc đang ẩn
    string dosage = ui->txtDosage->text().toStdString();
    int qty = ui->spinBoxQuantity->value(); // Hộp nhập số lượng

    if (recordId.empty() || medId.empty() || qty <= 0) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đủ thông tin và số lượng lớn hơn 0.");
        return;
    }

    // 2. Chuyển cho Backend gánh vác việc kiểm tra
    int result = db.prescribeMedicineFromQT(recordId, medId, dosage, qty);

    // 3. Phản hồi lại giao diện
    if (result == 1) {
        QMessageBox::information(this, "Kê đơn", "Đã thêm thuốc vào Bệnh án thành công!");
        ui->txtDosage->clear();
        ui->spinBoxQuantity->setValue(1);
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
    QMessageBox::information(this, "Lịch Sử Khám", "Tra cứu lịch sử khám bệnh cũ của bệnh nhân.");
    // 1. Lấy Mã Bệnh Nhân do Bác sĩ gõ
    string patientId = ui->txtPatientId_History->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng nhập Mã Bệnh Nhân để tra cứu!");
        return;
    }

    // 2. Chốt chặn: Kiểm tra Bệnh nhân có thật không
    if (db.findPatientById(patientId) == nullptr) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh nhân không tồn tại trên hệ thống!");
        return;
    }

    // 3. Gọi Backend lấy danh sách lịch sử
    vector<MedicalRecord*> history = db.getPatientHistory(patientId);

    // 4. Xử lý kết quả trả về
    if (history.empty()) {
        QMessageBox::information(this, "Lịch Sử", "Bệnh nhân này chưa từng khám hoặc chưa có bệnh án nào được ghi nhận.");
        return;
    }

    // 5. Hiển thị Lịch sử ra màn hình (Gom các bệnh án thành 1 chuỗi văn bản dài)
    QString msg = "LỊCH SỬ KHÁM CỦA BỆNH NHÂN: " + QString::fromStdString(patientId) + "\n\n";
    
    // Duyệt qua từng tờ bệnh án trong quá khứ
    for (size_t i = 0; i < history.size(); i++) {
        msg += "--- Lần khám thứ " + QString::number(i + 1) + " ---\n";
        msg += "+ Mã Bệnh Án: " + QString::fromStdString(history[i]->getRecordId()) + "\n";
        msg += "+ Triệu chứng: " + QString::fromStdString(history[i]->getSymptoms()) + "\n";
        msg += "+ Chẩn đoán: " + QString::fromStdString(history[i]->getDiagnosis()) + "\n";
        msg += "+ Ghi chú: " + QString::fromStdString(history[i]->getNotes()) + "\n";
        msg += "+ Số loại thuốc đã kê: " + QString::number(history[i]->getPrescriptions().size()) + " loại\n";
        msg += "--------------------------\n\n";
    }

    // Hiển thị khung thông báo chứa toàn bộ chuỗi msg
    QMessageBox::information(this, "Lịch Sử Khám Bệnh", msg);
    
    ui->txtPatientId_History->clear();
}

void DoctorMenu::loadPatientQueue() {
    ui->tableAppointments->setRowCount(0);
    auto& appts = db.getAppointments();
    
    int row = 0;
    for (const auto& appt : appts) {
        if (appt.getStatus() == "Pending") {
            ui->tableAppointments->insertRow(row);
            ui->tableAppointments->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(appt.getAppointmentId())));
            ui->tableAppointments->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(appt.getPatientId())));
            ui->tableAppointments->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(appt.getAppointmentDate())));
            row++;
        }
    }
}
