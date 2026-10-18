#include "ui/ReceptionistMenu.h"
#include "ui_ReceptionistMenu.h"
#include "services/Database.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include "services/AuthService.h"

using namespace std;

ReceptionistMenu::ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ReceptionistMenu), db(database), auth(authService) {
    ui->setupUi(this);

    // --- NÂNG CẤP UX: KHÓA CHỈ CHO NHẬP SỐ ĐIỆN THOẠI (10-11 SỐ) ---
    QRegularExpression rx("^[0-9]{10,11}$");
    QValidator *validator = new QRegularExpressionValidator(rx, this);
    ui->txtPatientPhone->setValidator(validator);

    QStringList specialties = {
        QString::fromUtf8("Khoa Khám bệnh"),
        QString::fromUtf8("Khoa Hồi sức cấp cứu"),
        QString::fromUtf8("Khoa Nội tổng hợp"),
        QString::fromUtf8("Khoa Truyền nhiễm"),
        QString::fromUtf8("Khoa Nhi"),
        QString::fromUtf8("Khoa Ngoại tổng hợp"),
        QString::fromUtf8("Khoa Phụ sản"),
        QString::fromUtf8("Liên chuyên khoa (Tai - Mũi - Họng, Răng - Hàm - Mặt, Mắt)"),
        QString::fromUtf8("Khoa Xét nghiệm (Huyết học, Hóa sinh, Vi sinh)"),
        QString::fromUtf8("Khoa Chẩn đoán hình ảnh"),
        QString::fromUtf8("Khoa Giải phẫu bệnh"),
        QString::fromUtf8("Khoa Chống nhiễm khuẩn"),
        QString::fromUtf8("Khoa Dược"),
        QString::fromUtf8("Khoa Dinh dưỡng")
    };
    ui->cbSpecialty->addItems(specialties);

    connect(ui->cbSpecialty, &QComboBox::currentTextChanged, this, &ReceptionistMenu::on_cbSpecialty_currentIndexChanged);
    connect(ui->cbDay, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    connect(ui->cbShift, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    
    connect(ui->tablePatients, &QTableWidget::itemClicked, this, &ReceptionistMenu::on_tablePatients_itemClicked);

    // --- NÂNG CẤP UX: KẾT NỐI LIVE SEARCH TỰ ĐỘNG ---
    connect(ui->txtSearchPatient, &QLineEdit::textChanged, this, &ReceptionistMenu::on_btnSearchPatient_clicked);

    updateAvailableDoctors();
    refreshPatientTable();
    loadPendingAppointments(); 
}

ReceptionistMenu::~ReceptionistMenu() {
    delete ui;
}

void ReceptionistMenu::on_tablePatients_itemClicked(QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0) {
        QString patientId = ui->tablePatients->item(row, 0)->text();
        ui->txtBookPatientId->setText(patientId);
        ui->txtPatientId_Invoice->setText(patientId);
    }
}

void ReceptionistMenu::on_btnLogout_clicked() {
     QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Đăng xuất", "Bạn có chắc chắn muốn đăng xuất?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        Login *loginForm = new Login(this->db);
        loginForm->show();
        this->close(); 
    }
}

void ReceptionistMenu::on_btnRegisterPatient_clicked() {
    QString qName = ui->txtPatientName->text();
    QString qPhone = ui->txtPatientPhone->text();

    if(qName.isEmpty() || qPhone.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đầy đủ Tên và Số điện thoại hợp lệ (10-11 số)!");
        return;
    }

    string name = qName.toStdString();
    string phone = qPhone.toStdString();
    bool isNoiTru = ui->radioNoiTru->isChecked();

    bool success = db.addPatientFromGUI(name, phone, isNoiTru);
    if (success) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã thêm Bệnh nhân mới tự động!\nVui lòng xem danh sách bên phải để biết Mã BN.");
        
        ui->txtPatientName->clear();
        ui->txtPatientPhone->clear();
        refreshPatientTable();
    } else {
        QMessageBox::critical(this, "Lỗi", "Hệ thống gặp sự cố khi tạo mã Bệnh nhân!");
    }
}

void ReceptionistMenu::updateAvailableDoctors() {
    ui->cbDoctor->clear();

    string targetSpecialty = ui->cbSpecialty->currentText().toStdString();
    string day = ui->cbDay->currentText().toStdString();
    string shift = ui->cbShift->currentText().toStdString();

    vector<Doctor*> availableDocs = db.getAvailableDoctors(day, shift);
    vector<Doctor*> filteredDocs;
    for (Doctor* doc : availableDocs) {
        if (doc->getSpecialty() == targetSpecialty) {
            filteredDocs.push_back(doc);
        }
    }

    if (filteredDocs.empty()) {
        ui->cbDoctor->addItem("Không có Bác sĩ rảnh");
        ui->cbDoctor->setEnabled(false);
    } else {
        for (Doctor* doc : filteredDocs) {
            QString displayText = QString::fromStdString(doc->getId() + " - " + doc->getFullName());
            ui->cbDoctor->addItem(displayText, QString::fromStdString(doc->getId()));
        }
        ui->cbDoctor->setEnabled(true);
    }
}

void ReceptionistMenu::on_btnBookAppointment_clicked() {
    string patientId = ui->txtBookPatientId->text().toStdString();
    string day = ui->cbDay->currentText().toStdString();
    string time = ui->cbShift->currentText().toStdString();
    string doctorId = ui->cbDoctor->currentData().toString().toStdString();
    string selectedSpecialty = ui->cbSpecialty->currentText().toStdString();
    User* currentUser = auth.getCurrentUser(); 
    
    if (currentUser == nullptr) {
        QMessageBox::critical(this, "Lỗi", "Phiên đăng nhập đã hết hạn hoặc không hợp lệ!");
        return;
    }
    string receptionistId = currentUser->getId();

    if (patientId.empty() || doctorId.empty() || day.empty() || time.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng chọn Bệnh nhân trên bảng và Bác sĩ hợp lệ!");
        return;
    }

    int result = db.bookAppointmentFromQT(patientId, doctorId, receptionistId, selectedSpecialty, day, time);

    if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã chốt lịch hẹn thành công!\nHệ thống tự động cấp Mã Lịch Hẹn (APP).");
        ui->txtBookPatientId->clear();
        loadPendingAppointments(); 
    }
    else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh Nhân không tồn tại!");
    }
    else {
        QMessageBox::critical(this, "Lỗi Xếp Lịch", "Có lỗi xảy ra, Bác sĩ không hợp lệ!");
    }
}

void ReceptionistMenu::on_btnCreateInvoice_clicked() {
    string patientId = ui->txtPatientId_Invoice->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng chọn Mã Bệnh Nhân trên bảng để thanh toán!");
        return;
    }

    double finalFeeToPay = 0.0;
    int result = db.createInvoiceFromQT(patientId, finalFeeToPay);

    if (result == 1) {
        QString msg = "Lập Hóa đơn thành công!\n\nSố tiền Bệnh nhân cần thanh toán: "
                      + QString::number(finalFeeToPay, 'f', 0) + " VNĐ";
        QMessageBox::information(this, "Thanh Toán Thành Công", msg);
        ui->txtPatientId_Invoice->clear();
    }
    else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh Nhân không tồn tại!");
    }
    else if (result == -2) {
        QMessageBox::critical(this, "Lỗi", "Bệnh nhân này chưa có Lịch hẹn khám nào!");
    }
    else if (result == -3) {
        QMessageBox::critical(this, "Cảnh báo", "Bác sĩ chưa tạo Hồ sơ Bệnh án. Chưa thể tính tiền!");
    }
}

// NÂNG CẤP UX: KẾT HỢP LIVE SEARCH VÀ KHÔI PHỤC BẢNG KHI XÓA TỪ KHÓA
void ReceptionistMenu::on_btnSearchPatient_clicked() {
    string keyword = ui->txtSearchPatient->text().toStdString();

    if (keyword.empty()) {
        refreshPatientTable(); // Tự động hiển thị lại bảng gốc khi không có từ khóa
        return;
    }
    
    vector<Patient*> foundPatients = db.searchPatients(keyword);
    ui->tablePatients->setRowCount(0);

    if (!foundPatients.empty()) {
        for (size_t i = 0; i < foundPatients.size(); i++) {
            ui->tablePatients->insertRow(i);
            ui->tablePatients->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getId())));
            ui->tablePatients->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getFullName())));
            ui->tablePatients->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getPhoneNumber())));
            ui->tablePatients->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getType())));
        }
    }
}

void ReceptionistMenu::loadPendingAppointments() {
    ui->cbPendingAppointments->clear(); 
    auto& appts = db.getAppointments(); 
    bool hasPending = false;

    for (const auto& appt : appts) {
        if (appt.getStatus() == "Pending") {
            hasPending = true;
            
            string patientName = "Không rõ";
            Patient* pat = db.findPatientById(appt.getPatientId());
            if (pat != nullptr) {
                patientName = pat->getFullName();
            }

            QString displayText = QString::fromStdString(appt.getAppointmentId() + " - " + patientName 
                                  + " (" + appt.getDayOfWeek() + ", " + appt.getTimeSlot() + ")");
            
            ui->cbPendingAppointments->addItem(displayText, QString::fromStdString(appt.getAppointmentId()));
        }
    }

    if (!hasPending) {
        ui->cbPendingAppointments->addItem("Không có lịch chờ khám");
        ui->cbPendingAppointments->setEnabled(false);
        ui->btnCancelAppointment->setEnabled(false);
    } else {
        ui->cbPendingAppointments->setEnabled(true);
        ui->btnCancelAppointment->setEnabled(true);
    }
}

void ReceptionistMenu::on_btnCancelAppointment_clicked() {
    string apptId = ui->cbPendingAppointments->currentData().toString().toStdString();

    if (apptId.empty()) {
        QMessageBox::warning(this, "Lỗi", "Không có lịch hẹn nào được chọn!");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận Hủy", 
                                       "Bạn có chắc chắn muốn hủy lịch hẹn [" + QString::fromStdString(apptId) + "] không?", 
                                       QMessageBox::Yes | QMessageBox::No);
                                       
    if (reply == QMessageBox::Yes) {
        int result = db.cancelAppointmentFromQT(apptId);
        
        if (result == 1) {
            QMessageBox::information(this, "Thành công", "Đã hủy lịch hẹn thành công!");
            loadPendingAppointments(); 
        } 
        else {
            QMessageBox::critical(this, "Lỗi", "Không thể hủy lịch hẹn này!");
        }
    }
}

void ReceptionistMenu::on_cbSpecialty_currentIndexChanged(const QString &specialtyText) {
    updateAvailableDoctors();
}

void ReceptionistMenu::refreshPatientTable() {
    ui->tablePatients->setColumnCount(4);
    ui->tablePatients->setHorizontalHeaderLabels({"Mã BN", "Họ Tên", "SĐT", "Loại"});
    ui->tablePatients->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tablePatients->setRowCount(0);
    vector<Patient**> patList = db.getPatients().getAllPointers();

    int row = 0;
    for (auto p : patList) {
        Patient* pat = *p;
        ui->tablePatients->insertRow(row);
        ui->tablePatients->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(pat->getId())));
        ui->tablePatients->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(pat->getFullName())));
        ui->tablePatients->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(pat->getPhoneNumber())));
        ui->tablePatients->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(pat->getType())));
        row++;
    }
}