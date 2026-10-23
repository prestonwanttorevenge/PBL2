#include "ui/ReceptionistMenu.h"
#include "ui_ReceptionistMenu.h"
#include "services/Database.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QInputDialog>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include "services/AuthService.h"

using namespace std;

ReceptionistMenu::ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ReceptionistMenu), db(database), auth(authService) {
    ui->setupUi(this);

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

    QString qBhyt = ui->txtBHYT->text().trimmed(); 
    string bhyt = qBhyt.isEmpty() ? "Không" : qBhyt.toStdString();

    bool success = db.addPatientFromGUI(name, phone, bhyt, isNoiTru);
    
    if (success) {
        db.saveAllData(); 
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã thêm Bệnh nhân mới tự động!\nVui lòng xem danh sách bên phải để biết Mã BN."));
        
        ui->txtPatientName->clear();
        ui->txtPatientPhone->clear();
        ui->txtBHYT->clear(); 
        refreshPatientTable();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Hệ thống gặp sự cố khi tạo mã Bệnh nhân!"));
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
        ui->cbDoctor->addItem(QString::fromUtf8("Không có Bác sĩ rảnh"));
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

    // --- LỚP KHIÊN NGĂN CHẶN std::bad_alloc ---
    // Kiểm tra trực tiếp trạng thái của Combobox trước khi cố ép kiểu dữ liệu rỗng (Invalid QVariant)
    if (!ui->cbDoctor->isEnabled() || ui->cbDoctor->currentText() == QString::fromUtf8("Không có Bác sĩ rảnh")) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Ca khám này hiện không có Bác sĩ rảnh!\nVui lòng chọn ca khác."));
        return; // Dừng lại ngay lập tức, không cho mã độc chạy tiếp!
    }
    // ------------------------------------------

    // Nếu lọt qua được lớp khiên trên, nghĩa là chắc chắn có dữ liệu Bác sĩ hợp lệ
    string doctorId = ui->cbDoctor->currentData().toString().toStdString();
    string selectedSpecialty = ui->cbSpecialty->currentText().toStdString();
    User* currentUser = auth.getCurrentUser(); 
    
    if (currentUser == nullptr) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Phiên đăng nhập đã hết hạn hoặc không hợp lệ!"));
        return;
    }
    string receptionistId = currentUser->getId();

    if (patientId.empty() || doctorId.empty() || day.empty() || time.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng chọn Bệnh nhân trên bảng và Bác sĩ hợp lệ!"));
        return;
    }

    int result = db.bookAppointmentFromQT(patientId, doctorId, receptionistId, selectedSpecialty, day, time);

    if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã chốt lịch hẹn thành công!\nHệ thống tự động cấp Mã Lịch Hẹn (APP)."));
        ui->txtBookPatientId->clear();
        loadPendingAppointments(); 
    }
    else if (result == -1) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Mã Bệnh Nhân không tồn tại!"));
    }
    else {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi Xếp Lịch"), QString::fromUtf8("Có lỗi xảy ra, Bác sĩ không hợp lệ!"));
    }
}

void ReceptionistMenu::on_btnCreateInvoice_clicked() {
    string patientId = ui->txtPatientId_Invoice->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng chọn Mã Bệnh Nhân trên bảng để thanh toán!"));
        return;
    }

    double finalFeeToPay = 0.0;
    int result = db.createInvoiceFromQT(patientId, finalFeeToPay);

    if (result == 1) {
        QString msg = QString::fromUtf8("Lập Hóa đơn thành công!\n\nSố tiền Bệnh nhân cần thanh toán: ")
                      + QString::number(finalFeeToPay, 'f', 0) + " VNĐ";
        QMessageBox::information(this, QString::fromUtf8("Thanh Toán Thành Công"), msg);
        ui->txtPatientId_Invoice->clear();
    }
    else if (result == -1) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Mã Bệnh Nhân không tồn tại!"));
    }
    else if (result == -2) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Bệnh nhân này chưa có Lịch hẹn khám nào!"));
    }
    else if (result == -3) {
        QMessageBox::critical(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Bác sĩ chưa điền Hồ sơ Bệnh án. Chưa thể tính tiền!"));
    }
}

void ReceptionistMenu::on_btnSearchPatient_clicked() {
    string keyword = ui->txtSearchPatient->text().toStdString();

    if (keyword.empty()) {
        refreshPatientTable(); 
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
            ui->tablePatients->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getBHYT())));
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
        ui->cbPendingAppointments->addItem(QString::fromUtf8("Không có lịch chờ khám"));
        ui->cbPendingAppointments->setEnabled(false);
        ui->btnCancelAppointment->setEnabled(false);
    } else {
        ui->cbPendingAppointments->setEnabled(true);
        ui->btnCancelAppointment->setEnabled(true);
    }
}

void ReceptionistMenu::on_btnCancelAppointment_clicked() {
    // Thêm khiên bảo vệ cho nút Hủy luôn để an toàn tuyệt đối
    if (!ui->cbPendingAppointments->isEnabled()) return;

    string apptId = ui->cbPendingAppointments->currentData().toString().toStdString();

    if (apptId.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Không có lịch hẹn nào được chọn!"));
        return;
    }

    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận Hủy"), 
                                       QString::fromUtf8("Bạn có chắc chắn muốn hủy lịch hẹn [") + QString::fromStdString(apptId) + "] không?", 
                                       QMessageBox::Yes | QMessageBox::No);
                                       
    if (reply == QMessageBox::Yes) {
        int result = db.cancelAppointmentFromQT(apptId);
        
        if (result == 1) {
            QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã hủy lịch hẹn thành công!"));
            loadPendingAppointments(); 
        } 
        else {
            QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Không thể hủy lịch hẹn này!"));
        }
    }
}

void ReceptionistMenu::on_cbSpecialty_currentIndexChanged(const QString &specialtyText) {
    updateAvailableDoctors();
}

void ReceptionistMenu::on_btnAssignBed_clicked() {
    // 1. Kiểm tra xem Lễ tân đã click chọn bệnh nhân trên bảng chưa
    int row = ui->tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn 1 Bệnh nhân trên bảng!");
        return;
    }

    // 2. Chặn chặn nếu bệnh nhân là Ngoại trú
    QString type = ui->tablePatients->item(row, 3)->text();
    if (type != "NoiTru") {
        QMessageBox::warning(this, "Lỗi", "Chức năng này chỉ dành cho Bệnh nhân NỘI TRÚ!");
        return;
    }

    string patientId = ui->tablePatients->item(row, 0)->text().toStdString();

    // 3. Hiển thị Pop-up nhập Số Phòng
    bool ok1;
    QString room = QInputDialog::getText(this, "Xếp Phòng", 
                                         "Nhập SỐ PHÒNG bệnh:", 
                                         QLineEdit::Normal, "", &ok1);
    if (!ok1) return; // Lễ tân bấm Cancel

    // 4. Hiển thị Pop-up nhập Số Giường
    bool ok2;
    QString bed = QInputDialog::getText(this, "Xếp Giường", 
                                        "Nhập SỐ GIƯỜNG bệnh:", 
                                        QLineEdit::Normal, "", &ok2);
    if (!ok2) return;

    // 5. Cập nhật xuống Database và Lưu file
    bool success = db.updateInpatientBed(patientId, room.toStdString(), bed.toStdString());
    if (success) {
        db.saveAllData();
        QMessageBox::information(this, "Thành công", "Đã cập nhật Phòng/Giường cho bệnh nhân!");
        refreshPatientTable(); // Load lại bảng để thấy kết quả
    }
}
void ReceptionistMenu::refreshPatientTable() {
    // ĐỔI THÀNH 6 CỘT
    ui->tablePatients->setColumnCount(6); 
    ui->tablePatients->setHorizontalHeaderLabels({"Mã BN", "Họ Tên", "SĐT", "Loại", "Mã BHYT", "Phòng/Giường"});
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
        ui->tablePatients->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(pat->getBHYT())));
        
        // THÊM ĐOẠN NÀY LẤY THÔNG TIN GIƯỜNG NẾU LÀ NỘI TRÚ
        string note = "";
        if (Inpatient* inpat = dynamic_cast<Inpatient*>(pat)) {
            if (!inpat->getRoomNumber().empty() && inpat->getRoomNumber() != "Chua xep") {
                note = "P: " + inpat->getRoomNumber() + " - G: " + inpat->getBedNumber();
            } else {
                note = "Chưa xếp giường";
            }
        }
        ui->tablePatients->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(note)));
        
        row++;
    }
}