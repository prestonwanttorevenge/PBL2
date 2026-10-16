#include "ui/ReceptionistMenu.h"
#include "ui_ReceptionistMenu.h"
#include "services/Database.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem> // Bắt buộc phải có để thao tác với bảng
#include <QComboBox>        // Bắt buộc phải có để thao tác với hộp chọn (cbDay, cbShift)

using namespace std;

ReceptionistMenu::ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ReceptionistMenu), db(database), auth(authService) {
    ui->setupUi(this);

    // Kết nối tín hiệu khi đổi ngày/ca khám thì tự cập nhật danh sách Bác sĩ
    connect(ui->cbDay, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    connect(ui->cbShift, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);

    updateAvailableDoctors();
    refreshPatientTable();
}

ReceptionistMenu::~ReceptionistMenu() {
    delete ui;
}

void ReceptionistMenu::on_btnLogout_clicked() {
     QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Đăng xuất", "Bạn có chắc chắn muốn đăng xuất?",
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply == QMessageBox::Yes) {
        // Mở form Login mới
        Login *loginForm = new Login(this->db);
        loginForm->show();
        this->close(); 
    }
}
void ReceptionistMenu::on_btnRegisterPatient_clicked() {
    QString qId = ui->txtPatientId->text();
    QString qName = ui->txtPatientName->text();
    QString qPhone = ui->txtPatientPhone->text();

    if(qId.isEmpty() || qName.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đầy đủ thông tin!");
        return;
    }

    string id = qId.toStdString();
    string name = qName.toStdString();
    string phone = qPhone.toStdString();
    bool isNoiTru = ui->radioNoiTru->isChecked();

    bool success = db.addPatientFromGUI(id, name, phone, isNoiTru);
    if (success) {
        QMessageBox::information(this, "Thành công", "Đã thêm bệnh nhân mới!");
        ui->txtPatientId->clear();
        ui->txtPatientName->clear();
        ui->txtPatientPhone->clear();
        refreshPatientTable();
        db.saveAllData();
    } else {
        QMessageBox::critical(this, "Lỗi", "ID Bệnh nhân đã tồn tại!");
    }
}

void ReceptionistMenu::updateAvailableDoctors() {
    ui->cbDoctor->clear();

    string day = ui->cbDay->currentText().toStdString();
    string shift = ui->cbShift->currentText().toStdString();

    vector<Doctor*> availableDocs = db.getAvailableDoctors(day, shift);

    if (availableDocs.empty()) {
        ui->cbDoctor->addItem("Không có Bác sĩ rảnh");
        ui->cbDoctor->setEnabled(false);
    } else {
        for (Doctor* doc : availableDocs) {
            QString displayText = QString::fromStdString(doc->getId() + " - " + doc->getFullName());
            ui->cbDoctor->addItem(displayText, QString::fromStdString(doc->getId()));
        }
        ui->cbDoctor->setEnabled(true);
    }
}

void ReceptionistMenu::on_btnBookAppointment_clicked() {
    string patientId = ui->txtPatientId->text().toStdString();
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
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng điền đủ thông tin và chọn Bác sĩ hợp lệ!");
        return;
    }

    int result = db.bookAppointmentFromQT(patientId, doctorId, receptionistId, selectedSpecialty, day, time);

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã chốt lịch hẹn thành công!\nTrạng thái: Chờ Khám");
        ui->txtPatientId->clear();
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
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng nhập Mã Bệnh Nhân để thanh toán!");
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

void ReceptionistMenu::on_btnSearchPatient_clicked() {
    string keyword = ui->txtSearchPatient->text().toStdString();

    if (keyword.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập ID hoặc Tên bệnh nhân để tìm!");
        return;
    }
    vector<Patient*> foundPatients = db.searchPatients(keyword);

    if (foundPatients.empty()) {
        QMessageBox::information(this, "Kết quả", "Không tìm thấy bệnh nhân nào phù hợp!");
    }
    else {
        ui->tablePatients->setRowCount(0);
        for (size_t i = 0; i < foundPatients.size(); i++) {
            ui->tablePatients->insertRow(i);
            ui->tablePatients->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getId())));
            ui->tablePatients->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getFullName())));
            ui->tablePatients->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getPhoneNumber())));
            ui->tablePatients->setItem(i, 3, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getType())));
        }
    }
}
// Bắt sự kiện khi Lễ tân đổi Chuyên khoa
void ReceptionistMenu::on_cbSpecialty_currentIndexChanged(const QString &specialtyText) {
    ui->cbDoctor->clear(); // Xóa danh sách bác sĩ cũ
    string targetSpecialty = specialtyText.toStdString();
    
    // Quét toàn bộ bác sĩ trong Database
    vector<Doctor*> doctors = db.getDoctors().getAllPointers();
    for (auto doc : doctors) {
        // Chỉ thêm những bác sĩ khớp chuyên khoa vào ComboBox
        if (doc->getSpecialty() == targetSpecialty) {
            ui->cbDoctor->addItem(QString::fromStdString(doc->getFullName()), 
                                  QString::fromStdString(doc->getId()));
        }
    }
}

void ReceptionistMenu::refreshPatientTable() {
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