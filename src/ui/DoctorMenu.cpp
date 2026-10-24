#include "ui/DoctorMenu.h"
#include "ui_DoctorMenu.h"
#include "ui/Login.h"
#include "services/Database.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QColor>
#include <QDate> 

using namespace std;

DoctorMenu::DoctorMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DoctorMenu), db(database), auth(authService) {
    ui->setupUi(this);
    
    connect(ui->tableAppointments, &QTableWidget::itemClicked, this, &DoctorMenu::on_tableAppointments_itemClicked);
    connect(ui->cbMedPrefixFilter, &QComboBox::currentTextChanged, this, &DoctorMenu::on_cbMedPrefixFilter_currentTextChanged);
    
    loadPatientQueue();
    loadMedicineComboBox("Tất cả"); 
    
    // Tải danh sách Dịch vụ Cận lâm sàng
    ui->cbService->clear();
    vector<Service*> srvs = db.getServices().getAllPointers();
    for (auto srv : srvs) {
        QString displayText = QString::fromStdString(srv->getId() + " - " + srv->getServiceName());
        ui->cbService->addItem(displayText, QString::fromStdString(srv->getId()));
    }
}

DoctorMenu::~DoctorMenu() {
    delete ui;
}

void DoctorMenu::on_tableAppointments_itemClicked(QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0) {
        QString apptId = ui->tableAppointments->item(row, 0)->text();
        QString patientId = ui->tableAppointments->item(row, 1)->text(); // Cột 1 là Mã Bệnh Nhân
        
        ui->txtAppointmentId->setText(apptId);
        ui->txtPatientId_History->setText(patientId);

        // TỰ ĐỘNG LẤY MÃ BỆNH ÁN TỪ LỄ TÂN
        ui->txtRecordId->clear();
        vector<MedicalRecord>& records = db.getMedicalRecords();
        for (auto& rec : records) {
            if (rec.getAppointmentId() == apptId.toStdString()) {
                ui->txtRecordId->setText(QString::fromStdString(rec.getRecordId()));
                break;
            }
        }
    }
}

void DoctorMenu::on_cbMedPrefixFilter_currentTextChanged(const QString &text) {
    QString prefix = "Tất cả";
    if(text.startsWith("B1")) prefix = "B1";
    else if(text.startsWith("B2")) prefix = "B2";
    else if(text.startsWith("B3")) prefix = "B3";
    else if(text.startsWith("DV")) prefix = "DV";
    
    loadMedicineComboBox(prefix);
}

void DoctorMenu::loadMedicineComboBox(const QString& filterPrefix) {
    ui->cbMedicine->clear();
    vector<Medicine*> meds = db.getMedicines().getAllPointers();
    bool foundAny = false;

    if (meds.empty()) {
        ui->cbMedicine->addItem("Không có thuốc trong kho");
        ui->cbMedicine->setEnabled(false);
        return;
    } 

    for (auto med : meds) {
        QString medId = QString::fromStdString(med->getMedicineId());
        
        if (filterPrefix != "Tất cả" && !medId.startsWith(filterPrefix)) {
            continue; 
        }
        
        foundAny = true;
        QString displayText = medId + " - " + QString::fromStdString(med->getMedicineName()) + " (Tồn: " + QString::number(med->getStock()) + ")";
        ui->cbMedicine->addItem(displayText, medId);
    }

    if (!foundAny) {
        ui->cbMedicine->addItem("Không có thuốc thuộc nhóm này");
        ui->cbMedicine->setEnabled(false);
    } else {
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
        QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng click chọn Lịch hẹn trên bảng và nhập Chẩn đoán!");
        return;
    }

    string updatedRecordId = db.updateMedicalRecordFromQT(apptId, symp, diag, notes);

    if (!updatedRecordId.empty()) {
        ui->txtRecordId->setText(QString::fromStdString(updatedRecordId));
        QMessageBox::information(this, "Thành công", 
                                "Đã cập nhật Bệnh án thành công!\nMã Bệnh Án: " + QString::fromStdString(updatedRecordId));
        
        ui->txtAppointmentId->clear();
        ui->txtSymptoms->clear();
        ui->txtDiagnosis->clear();
        ui->txtNotes->clear();
        loadPatientQueue();
    } else {
        QMessageBox::critical(this, "Lỗi", "Không tìm thấy Lịch hẹn hoặc Bệnh án bị lỗi hệ thống!");
    }
}

void DoctorMenu::on_btnPrescribeMedicine_clicked() {
    string recordId = ui->txtRecordId->text().toStdString();
    string medId = ui->cbMedicine->currentData().toString().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    int qty = ui->spinBoxQuantity->value();

    if (recordId.empty() || medId.empty() || qty <= 0) {
        QMessageBox::warning(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Vui lòng nhập đủ thông tin và số lượng lớn hơn 0."));
        return;
    }

    // --- LỚP KHIÊN BẢO VỆ UX: ĐÃ ĐƯỢC KHÔI PHỤC NGUYÊN TRẠNG ---
    MedicalRecord* record = db.findMedicalRecordById(recordId);
    if (record != nullptr && record->getDiagnosis().empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Quy trình sai"), 
                             QString::fromUtf8("Bác sĩ vui lòng nhập [Chẩn đoán] và bấm [Lưu Bệnh Án] ở phía trên để chốt hồ sơ trước khi kê đơn thuốc!"));
        return;
    }

    // === LỚP KHIÊN ÉP BUỘC CHỜ KẾT QUẢ CLS: ĐÃ ĐƯỢC KHÔI PHỤC ===
    string apptId = ui->txtAppointmentId->text().toStdString();
    Appointment* appt = db.findAppointmentById(apptId);

    if (appt != nullptr && record != nullptr) {
        if (!record->getAssignedServices().empty() && appt->getStatus() == "Pending") {
            QMessageBox::critical(this, QString::fromUtf8("Cảnh Báo Vi Phạm Quy Trình"), 
                QString::fromUtf8("Bệnh nhân đã được chỉ định Dịch vụ Cận lâm sàng!\n\n"
                                  "Vui lòng bấm nút [Lưu & Chờ Kết Quả CLS] cho bệnh nhân đi làm xét nghiệm.\n"
                                  "Khi bệnh nhân mang kết quả về, bạn click lại vào tên bệnh nhân trên bảng mới được phép kê đơn thuốc!"));
            return; 
        }
    }
    
    int result = db.prescribeMedicineFromQT(recordId, medId, dosage, qty);

    if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, QString::fromUtf8("Kê đơn"), QString::fromUtf8("Đã thêm thuốc vào Bệnh án [") + QString::fromStdString(recordId) + QString::fromUtf8("] thành công!"));
        ui->txtDosage->clear();
        ui->spinBoxQuantity->setValue(1);
        
        QString currentFilter = QString::fromUtf8("Tất cả");
        QString filterText = ui->cbMedPrefixFilter->currentText();
        if(filterText.startsWith("B1")) currentFilter = "B1";
        else if(filterText.startsWith("B2")) currentFilter = "B2";
        else if(filterText.startsWith("B3")) currentFilter = "B3";
        else if(filterText.startsWith("DV")) currentFilter = "DV";
        
        loadMedicineComboBox(currentFilter); 
    } 
    else if (result == -1) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Mã Bệnh án không tồn tại!"));
    } 
    else if (result == -2) {
        QMessageBox::critical(this, QString::fromUtf8("Lỗi"), QString::fromUtf8("Thuốc không tồn tại trên hệ thống!"));
    } 
    else if (result == -3) {
        QMessageBox::critical(this, QString::fromUtf8("Hết hàng"), QString::fromUtf8("Số lượng thuốc trong kho không đủ để kê đơn này!"));
    }
}

void DoctorMenu::on_btnViewPatientHistory_clicked() {
    string patientId = ui->txtPatientId_History->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng chọn hoặc nhập Mã Bệnh Nhân để tra cứu!");
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

    // ĐÃ KHÔI PHỤC NGUYÊN DẠNG GHI CHÚ
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
        
        // ĐÃ KHÔI PHỤC KIỂM TRA LỖI -2
        if (result == 1) {
            QMessageBox::information(this, "Thành công", "Đã hủy lịch hẹn thành công!");
            loadPatientQueue(); 
            ui->txtAppointmentId->clear();
            ui->txtPatientId_History->clear();
        } 
        else if (result == -2) {
            QMessageBox::warning(this, "Lỗi", "Lịch hẹn này đã khám xong, không thể hủy!");
        }
    }
}

void DoctorMenu::on_btnAssignService_clicked() {
    string recordId = ui->txtRecordId->text().toStdString();
    string serviceId = ui->cbService->currentData().toString().toStdString();

    if (recordId.empty() || serviceId.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập Chẩn đoán và Lưu bệnh án trước!");
        return;
    }

    int result = db.assignServiceFromQT(recordId, serviceId);

    if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã chỉ định Dịch vụ thành công!");
    } else if (result == -3) {
        QMessageBox::warning(this, "Cảnh báo", "Dịch vụ này đã được chỉ định rồi!");
    }
}

void DoctorMenu::on_btnWaitResult_clicked() {
    string apptId = ui->txtAppointmentId->text().toStdString();
    string symp = ui->txtSymptoms->text().toStdString();
    string diag = ui->txtDiagnosis->text().toStdString();
    string notes = ui->txtNotes->text().toStdString();

    if (apptId.empty() || diag.empty()) {
        QMessageBox::warning(this, "Thiếu thông tin", "Vui lòng click chọn Lịch hẹn và nhập Chẩn đoán sơ bộ!");
        return;
    }

    string updatedRecordId = db.updateMedicalRecordFromQT(apptId, symp, diag, notes, "Waiting");

    if (!updatedRecordId.empty()) {
        QMessageBox::information(this, "Thành công", "Đã lưu hồ sơ. Bệnh nhân đang đi làm xét nghiệm Cận lâm sàng!");
        loadPatientQueue(); 
    }
}

void DoctorMenu::loadPatientQueue() {
    ui->tableAppointments->setColumnCount(5);
    ui->tableAppointments->setHorizontalHeaderLabels({"Mã Lịch Hẹn", "Mã Bệnh Nhân", "Loại", "Thời Gian", "Trạng Thái"});
    ui->tableAppointments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableAppointments->setRowCount(0);
    auto& appts = db.getAppointments();
    
    int row = 0;
    for (const auto& appt : appts) {
        if (appt.getStatus() == "Pending" || appt.getStatus() == "Waiting") {
            ui->tableAppointments->insertRow(row);
            
            QTableWidgetItem *item0 = new QTableWidgetItem(QString::fromStdString(appt.getAppointmentId()));
            QTableWidgetItem *item1 = new QTableWidgetItem(QString::fromStdString(appt.getPatientId()));
            
            // CỘT LOẠI BỆNH NHÂN
            Patient* pat = db.findPatientById(appt.getPatientId());
            QString typeStr = "Chưa rõ";
            if (pat != nullptr) {
                if (pat->getType() == "NoiTru") typeStr = QString::fromUtf8("Nội Trú");
                else if (pat->getType() == "NgoaiTru") typeStr = QString::fromUtf8("Ngoại Trú");
            }
            QTableWidgetItem *item2 = new QTableWidgetItem(typeStr);

            QString timeStr = QString::fromStdString(appt.getDayOfWeek() + " - " + appt.getTimeSlot());
            QTableWidgetItem *item3 = new QTableWidgetItem(timeStr);
            
            QString statusText = (appt.getStatus() == "Pending") ? QString::fromUtf8("Chờ Khám") : QString::fromUtf8("Chờ Kết Quả CLS");
            QTableWidgetItem *item4 = new QTableWidgetItem(statusText);
            
            // ĐÃ KHÔI PHỤC NGUYÊN TRẠNG BẢNG MÀU UI
            QColor bg = (appt.getStatus() == "Pending") ? QColor(255, 243, 205) : QColor(217, 237, 247); 
            QColor fg = (appt.getStatus() == "Pending") ? QColor(133, 100, 4) : QColor(12, 84, 96);
            
            item0->setBackground(bg); item1->setBackground(bg); item2->setBackground(bg);
            item3->setBackground(bg); item4->setBackground(bg);
            item4->setForeground(fg);

            ui->tableAppointments->setItem(row, 0, item0);
            ui->tableAppointments->setItem(row, 1, item1);
            ui->tableAppointments->setItem(row, 2, item2);
            ui->tableAppointments->setItem(row, 3, item3);
            ui->tableAppointments->setItem(row, 4, item4);
            row++;
        }
    }
}

void DoctorMenu::on_btnDischargePatient_clicked() {
    string patientId = ui->txtPatientId_History->text().toStdString();

    if (patientId.empty()) {
        QMessageBox::warning(this, QString::fromUtf8("Cảnh báo"), QString::fromUtf8("Vui lòng truy xuất Hồ sơ Bệnh nhân cần duyệt xuất viện tại ô Mã Bệnh Nhân."));
        return;
    }

    auto reply = QMessageBox::question(this, QString::fromUtf8("Xác nhận Phê duyệt"), 
                                       QString::fromUtf8("Xác nhận phê duyệt xuất viện cho Bệnh nhân [") + QString::fromStdString(patientId) + 
                                       QString::fromUtf8("]?\nHành động này sẽ tự động thu hồi không gian lưu trú (Trả giường)."), 
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QString currentDate = QDate::currentDate().toString("dd/MM/yyyy");
        int result = db.dischargeInpatientFromQT(patientId, currentDate.toStdString());

        if (result == 1) {
            db.saveAllData(); 
            QMessageBox::information(this, QString::fromUtf8("Thành công"), 
                                     QString::fromUtf8("Đã phê duyệt xuất viện thành công!\nKhông gian lưu trú đã được thu hồi."));
            ui->txtPatientId_History->clear();
        } 
        else if (result == -2) {
            QMessageBox::warning(this, QString::fromUtf8("Từ chối"), QString::fromUtf8("Đây là bệnh nhân Ngoại trú. Quy trình giải phóng giường không áp dụng."));
        } 
        else if (result == -3) {
            QMessageBox::warning(this, QString::fromUtf8("Lỗi Trạng thái"), QString::fromUtf8("Hồ sơ này chưa được xếp giường hoặc đã xuất viện từ trước."));
        } 
    }
}