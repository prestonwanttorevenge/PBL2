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

    // KẾT NỐI TÍN HIỆU (SIGNALS & SLOTS)
    connect(ui->cbSpecialty, &QComboBox::currentTextChanged, this, &ReceptionistMenu::on_cbSpecialty_currentIndexChanged);
    connect(ui->cbDay, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    connect(ui->cbShift, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    
    connect(ui->tablePatients, &QTableWidget::itemClicked, this, &ReceptionistMenu::on_tablePatients_itemClicked);
    connect(ui->txtSearchPatient, &QLineEdit::textChanged, this, &ReceptionistMenu::on_btnSearchPatient_clicked);
    
    // Bắt sự kiện Lễ tân chuyển đổi phòng thì danh sách giường đổi theo
    connect(ui->cbRoom, &QComboBox::currentTextChanged, this, &ReceptionistMenu::on_cbRoom_currentIndexChanged);

    updateAvailableDoctors();
    refreshPatientTable();
    loadPendingAppointments(); 
    loadAvailableRooms(); // Kích hoạt quét không gian ngay khi mở Form
}

ReceptionistMenu::~ReceptionistMenu() {
    delete ui;
}

// ==========================================
// THUẬT TOÁN ĐỒNG BỘ PHÒNG/GIƯỜNG TỰ ĐỘNG
// ==========================================
void ReceptionistMenu::loadAvailableRooms() {
    ui->cbRoom->clear();
    currentRoomMap = db.getAvailableRoomMap();
    
    if (currentRoomMap.empty()) {
        ui->cbRoom->addItem(QString::fromUtf8("Hết phòng trống"));
        ui->cbRoom->setEnabled(false);
        ui->cbBed->setEnabled(false);
        ui->btnAssignBed->setEnabled(false);
    } else {
        for (auto const& pair : currentRoomMap) {
            QString room = QString::fromStdString(pair.first);
            int bedCount = pair.second.size();
            QString displayText = room + QString::fromUtf8(" (Còn ") + QString::number(bedCount) + QString::fromUtf8(" giường)");
            
            // Tham số thứ 2 lưu Data ngầm định (Data thực chất là mã phòng)
            ui->cbRoom->addItem(displayText, room); 
        }
        ui->cbRoom->setEnabled(true);
        ui->cbBed->setEnabled(true);
        ui->btnAssignBed->setEnabled(true);
    }
}

void ReceptionistMenu::on_cbRoom_currentIndexChanged(const QString &roomText) {
    ui->cbBed->clear();
    if (roomText.isEmpty() || currentRoomMap.empty()) return;

    // Rút mã phòng thực tế từ Data ngầm định
    string room = ui->cbRoom->currentData().toString().toStdString();
    
    if (currentRoomMap.find(room) != currentRoomMap.end()) {
        for (const string& bed : currentRoomMap[room]) {
            ui->cbBed->addItem(QString::fromUtf8("Giường ") + QString::fromStdString(bed), QString::fromStdString(bed));
        }
    }
}

// ==========================================
// CÁC HÀM XỬ LÝ SỰ KIỆN KHÁC
// ==========================================
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
        this, QString::fromUtf8("Đăng xuất"), QString::fromUtf8("Xác nhận đăng xuất khỏi hệ thống Lễ tân?"),
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
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng nhập đầy đủ thông tin hành chính. Số điện thoại (10-11 số)."));
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
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã đăng ký Hồ sơ Bệnh nhân mới thành công."));
        
        ui->txtPatientName->clear();
        ui->txtPatientPhone->clear();
        ui->txtBHYT->clear(); 
        refreshPatientTable();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Hệ thống gián đoạn khi khởi tạo Hồ sơ!"));
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
        ui->cbDoctor->addItem(QString::fromUtf8("Không có Bác sĩ nhận bệnh"));
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

    if (!ui->cbDoctor->isEnabled() || ui->cbDoctor->currentText() == QString::fromUtf8("Không có Bác sĩ nhận bệnh")) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Ca khám này hiện không có Bác sĩ tiếp nhận. Vui lòng chuyển ca khác."));
        return; 
    }
    
    string doctorId = ui->cbDoctor->currentData().toString().toStdString();
    string selectedSpecialty = ui->cbSpecialty->currentText().toStdString();
    User* currentUser = auth.getCurrentUser(); 
    
    if (currentUser == nullptr) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Phiên đăng nhập đã hết hạn!"));
        return;
    }
    string receptionistId = currentUser->getId();

    if (patientId.empty() || doctorId.empty() || day.empty() || time.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng chọn Hồ sơ Bệnh nhân và Chỉ định Bác sĩ hợp lệ."));
        return;
    }

    int result = db.bookAppointmentFromQT(patientId, doctorId, receptionistId, selectedSpecialty, day, time);

    if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Chỉ định lịch khám thành công. Mã Lịch Hẹn đã được khởi tạo tự động."));
        ui->txtBookPatientId->clear();
        loadPendingAppointments(); 
    }
    else if (result == -1) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Hồ sơ Bệnh nhân không tồn tại."));
    }
    else {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Trạng thái Bác sĩ không hợp lệ."));
    }
}

void ReceptionistMenu::on_btnCreateInvoice_clicked() {
    string patientId = ui->txtPatientId_Invoice->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng truy xuất Bệnh nhân cần thanh toán."));
        return;
    }

    double finalFeeToPay = 0.0;
    int result = db.createInvoiceFromQT(patientId, finalFeeToPay);

    if (result == 1) {
        QString msg = QString::fromUtf8("Lập Hóa đơn viện phí thành công.\n\nTổng tiền Bệnh nhân cần thanh toán: ")
                      + QString::number(finalFeeToPay, 'f', 0) + " VNĐ";
        QMessageBox::information(this, QString::fromUtf8("Kế Toán Viện Phí"), msg);
        ui->txtPatientId_Invoice->clear();
    }
    else if (result == -1) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Hồ sơ Bệnh nhân không tồn tại."));
    }
    else if (result == -2) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Bệnh nhân chưa có lịch sử tiếp nhận khám chữa bệnh."));
    }
    else if (result == -3) {
        QMessageBox::critical(this, QString::fromUtf8("Quy trình"), QString::fromUtf8("Hồ sơ Bệnh án chưa được Bác sĩ hoàn tất chẩn đoán. Không thể lập hóa đơn."));
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
    if (!ui->cbPendingAppointments->isEnabled()) return;

    string apptId = ui->cbPendingAppointments->currentData().toString().toStdString();

    if (apptId.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Không có Lịch hẹn nào được chỉ định."));
        return;
    }

    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận"), 
                                       QString::fromUtf8("Xác nhận hủy lịch khám [") + QString::fromStdString(apptId) + QString::fromUtf8("] do vắng mặt?"), 
                                       QMessageBox::Yes | QMessageBox::No);
                                       
    if (reply == QMessageBox::Yes) {
        int result = db.cancelAppointmentFromQT(apptId);
        
        if (result == 1) {
            QMessageBox::information(this, QString::fromUtf8("Thành công"), QString::fromUtf8("Đã vô hiệu hóa Lịch hẹn thành công."));
            loadPendingAppointments(); 
        } 
        else {
            QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Gián đoạn hệ thống. Không thể hủy lịch hẹn này."));
        }
    }
}

void ReceptionistMenu::on_cbSpecialty_currentIndexChanged(const QString &specialtyText) {
    updateAvailableDoctors();
}

// === CƠ CHẾ XẾP GIƯỜNG HOÀN TOÀN MỚI BẰNG COMBOBOX ===
void ReceptionistMenu::on_btnAssignBed_clicked() {
    int row = ui->tablePatients->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, QString::fromUtf8("Chưa chọn"), QString::fromUtf8("Vui lòng truy xuất 1 Hồ sơ Nội trú trên hệ thống."));
        return;
    }

    QString type = ui->tablePatients->item(row, 3)->text();
    if (type != "NoiTru") {
        QMessageBox::warning(this, QString::fromUtf8("Phân loại sai"), QString::fromUtf8("Nghiệp vụ lưu trú chỉ áp dụng cho Bệnh nhân Nội trú."));
        return;
    }

    string patientId = ui->tablePatients->item(row, 0)->text().toStdString();

    if (!ui->cbRoom->isEnabled()) {
        QMessageBox::critical(this, QString::fromUtf8("Cảnh báo quá tải"), QString::fromUtf8("Toàn bộ 250 giường bệnh đã hoạt động hết công suất!"));
        return;
    }

    // Rút xuất Data ngầm định để lấy thông tin Room và Bed chuẩn xác
    string room = ui->cbRoom->currentData().toString().toStdString();
    string bed = ui->cbBed->currentData().toString().toStdString();

    // Điều hướng xuống DB để thực hiện giao dịch mờ (ACID)
    int result = db.updateInpatientBed(patientId, room, bed);
    
    if (result == 1) {
        db.saveAllData();
        QMessageBox::information(this, QString::fromUtf8("Lưu trú"), QString::fromUtf8("Cấp phát Giường bệnh thành công. Bệnh nhân có thể bắt đầu quá trình điều trị."));
        refreshPatientTable();
        loadAvailableRooms(); // Cập nhật lại Mắt thần để loại bỏ giường vừa bị chiếm
    } else if (result == -3) {
        QMessageBox::warning(this, QString::fromUtf8("Xung đột dữ liệu"), QString::fromUtf8("Giường này vừa được một Lễ tân khác tiếp nhận. Vui lòng chuyển sang giường khác."));
        loadAvailableRooms();
    } else if (result == -4) {
        QMessageBox::warning(this, QString::fromUtf8("Đầy phòng"), QString::fromUtf8("Căn phòng này vừa đạt sức chứa tối đa. Hệ thống đang đồng bộ lại."));
        loadAvailableRooms();
    } else {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi Hệ thống"), QString::fromUtf8("Hồ sơ Bệnh nhân bị gián đoạn."));
    }
}

void ReceptionistMenu::refreshPatientTable() {
    ui->tablePatients->setColumnCount(6); 
    ui->tablePatients->setHorizontalHeaderLabels({QString::fromUtf8("Mã BN"), QString::fromUtf8("Họ Tên"), QString::fromUtf8("SĐT"), QString::fromUtf8("Loại"), QString::fromUtf8("Mã BHYT"), QString::fromUtf8("Lưu Trú")});
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
        
        string note = "";
        if (Inpatient* inpat = dynamic_cast<Inpatient*>(pat)) {
            if (inpat->getRoomNumber() == "Da xuat vien") {
                note = "Đã xuất viện";
            }
            else if (!inpat->getRoomNumber().empty() && inpat->getRoomNumber() != "Chua xep") {
                note = "Phòng: " + inpat->getRoomNumber() + " - Giường: " + inpat->getBedNumber();
            } else {
                note = "Chờ xếp giường";
            }
        }
        ui->tablePatients->setItem(row, 5, new QTableWidgetItem(QString::fromUtf8(note.c_str())));
        
        row++;
    }
}