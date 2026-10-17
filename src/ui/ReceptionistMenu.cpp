#include "ui/ReceptionistMenu.h"
#include "ui_ReceptionistMenu.h"
#include "services/Database.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem> // Bắt buộc phải có để thao tác với bảng
#include <QComboBox>        // Bắt buộc phải có để thao tác với hộp chọn (cbDay, cbShift)
#include "services/AuthService.h"
using namespace std;

ReceptionistMenu::ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ReceptionistMenu), db(database), auth(authService) {
    ui->setupUi(this);

    // --- NẠP 14 CHUYÊN KHOA VÀO COMBOBOX CHUYÊN KHOA ---
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

    // Kết nối tín hiệu khi đổi chuyên khoa/ngày/ca khám thì tự cập nhật danh sách Bác sĩ
    connect(ui->cbSpecialty, &QComboBox::currentTextChanged, this, &ReceptionistMenu::on_cbSpecialty_currentIndexChanged);
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

    string targetSpecialty = ui->cbSpecialty->currentText().toStdString();
    string day = ui->cbDay->currentText().toStdString();
    string shift = ui->cbShift->currentText().toStdString();

    vector<Doctor*> availableDocs = db.getAvailableDoctors(day, shift);
    
    // Lọc thêm theo chuyên khoa
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
void ReceptionistMenu::loadPendingAppointments() {
    ui->cbPendingAppointments->clear(); // Xóa sạch dữ liệu cũ
    
    // Lấy toàn bộ danh sách lịch hẹn từ Database
    auto& appts = db.getAppointments(); 
    bool hasPending = false;

    for (const auto& appt : appts) {
        if (appt.getStatus() == "Pending") {
            hasPending = true;
            
            // Tìm tên bệnh nhân để hiển thị cho thân thiện
            string patientName = "Không rõ";
            Patient* pat = db.findPatientById(appt.getPatientId());
            if (pat != nullptr) {
                patientName = pat->getFullName();
            }

            // Text hiển thị trên màn hình: "APP1 - Nguyen Van A (Thứ 2 - Sáng)"
            QString displayText = QString::fromStdString(appt.getAppointmentId() + " - " + patientName 
                                  + " (" + appt.getDay() + ", " + appt.getTime() + ")");
            
            // Data ngầm lưu trữ: "APP1" (chỉ lưu mã ID để lát nữa gọi hàm Hủy)
            ui->cbPendingAppointments->addItem(displayText, QString::fromStdString(appt.getAppointmentId()));
        }
    }

    if (!hasPending) {
        ui->cbPendingAppointments->addItem("Không có lịch chờ khám");
        ui->cbPendingAppointments->setEnabled(false);
        ui->btnCancelAppointment->setEnabled(false); // Khóa nút Hủy
    } else {
        ui->cbPendingAppointments->setEnabled(true);
        ui->btnCancelAppointment->setEnabled(true);  // Mở nút Hủy
    }
}
void ReceptionistMenu::on_btnCancelAppointment_clicked() {
    // 1. Rút mã Lịch Hẹn (ID ngầm) từ mục đang được chọn trên ComboBox
    string apptId = ui->cbPendingAppointments->currentData().toString().toStdString();

    if (apptId.empty()) {
        QMessageBox::warning(this, "Lỗi", "Không có lịch hẹn nào được chọn!");
        return;
    }

    // 2. Hỏi xác nhận
    auto reply = QMessageBox::question(this, "Xác nhận Hủy", 
                                       "Bạn có chắc chắn muốn hủy lịch hẹn [" + QString::fromStdString(apptId) + "] không?", 
                                       QMessageBox::Yes | QMessageBox::No);
                                       
    if (reply == QMessageBox::Yes) {
        // 3. Gọi Backend để hủy
        int result = db.cancelAppointmentFromQT(apptId);
        
        if (result == 1) {
            QMessageBox::information(this, "Thành công", "Đã hủy lịch hẹn thành công!");
            
            // 4. QUAN TRỌNG: Load lại cái ComboBox để nó đá cái lịch vừa Hủy biến mất!
            loadPendingAppointments(); 
        } 
        else {
            QMessageBox::critical(this, "Lỗi", "Không thể hủy lịch hẹn này!");
        }
    }
}
// Bắt sự kiện khi Lễ tân đổi Chuyên khoa
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