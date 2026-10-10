#include "../../include/ui/ReceptionistMenu.h"
#include "ui_ReceptionistMenu.h"
#include <QMessageBox>
#include "../../include/services/Database.h"

ReceptionistMenu::ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ReceptionistMenu), db(database), auth(authService) {
    ui->setupUi(this);
    connect(ui->cbDay, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    connect(ui->cbShift, &QComboBox::currentTextChanged, this, &ReceptionistMenu::updateAvailableDoctors);
    updateAvailableDoctors();
    refreshPatientTable();
}

ReceptionistMenu::~ReceptionistMenu() {
    delete ui;
}

void ReceptionistMenu::on_btnLogout_clicked() {
    auth.logout();
    QMessageBox::information(this, "Đăng xuất", "Lễ tân đã đăng xuất hệ thống!");
    this->close();
}

void ReceptionistMenu::on_btnRegisterPatient_clicked() {
    QMessageBox::information(this, "Đăng Ký", "Mở form nhập thông tin bệnh nhân mới.");
    QString qId = ui->txtPatientId->text();
    QString qName = ui->txtPatientName->text();
    QString qPhone = ui->txtPatientPhone->text();
    if(qId.isEmpty() || qName.isEmpty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đầy đủ thông tin!");
        return;
    }
    // Chuyển đổi QString sang std::string 
    string id = qId.toStdString();
    string name = qName.toStdString();
    string phone = qPhone.toStdString();
    // làm cho t 1 cái RadioButton để chọn Nội trú/Ngoại trú để còn chia nhánh bệnh nhân
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
    ui->cbDoctor->clear(); // Xóa sạch danh sách cũ trong Hộp chọn Bác sĩ

    // Lấy Thứ và Ca hiện tại trên màn hình
    string day = ui->cbDay->currentText().toStdString();
    string shift = ui->cbShift->currentText().toStdString();

    // Gọi Backend lấy mảng Bác sĩ
    vector<Doctor*> availableDocs = db.getAvailableDoctors(day, shift);

    if (availableDocs.empty()) {
        ui->cbDoctor->addItem("Không có Bác sĩ rảnh");
        ui->cbDoctor->setEnabled(false); // Khóa hộp chọn lại vì không có ai
    } else {
        // Có bác sĩ rảnh -> Đổ dữ liệu vào Hộp chọn
        for (Doctor* doc : availableDocs) {
            // Hiển thị ra màn hình: "BS01 - Nguyen Van A"
            QString displayText = QString::fromStdString(doc->getId() + " - " + doc->getFullName());
            
            // Giấu Mã ID (doc->getId()) vào userData của item để lúc Đặt lịch sẽ lôi ra dùng
            ui->cbDoctor->addItem(displayText, QString::fromStdString(doc->getId()));
        }
        ui->cbDoctor->setEnabled(true); // Mở khóa hộp chọn
    }
}
void ReceptionistMenu::on_btnBookAppointment_clicked() {
    QMessageBox::information(this, "Đặt Lịch", "Xếp lịch hẹn với bác sĩ cho bệnh nhân.");
  string patientId = ui->txtPatientId->text().toStdString();
    string day = ui->cbDay->currentText().toStdString();
    string time = ui->cbShift->currentText().toStdString();
    
    // Lấy ID Bác sĩ đang được giấu (userData) ở Hộp chọn (QComboBox)
    string doctorId = ui->cbDoctor->currentData().toString().toStdString();
    
    string receptionistId = "LT01"; 

    // Kiểm tra UI có rỗng không (Nếu doctorId rỗng nghĩa là cbDoctor đang hiện "Không có Bác sĩ rảnh")
    if (patientId.empty() || doctorId.empty() || day.empty() || time.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng điền đủ thông tin và chọn Bác sĩ hợp lệ!");
        return;
    }

    // Đẩy xuống Backend (với hàm trả về int 1, -1, -2, -3 như đã bàn)
    int result = db.bookAppointmentFromQT(patientId, doctorId, receptionistId, day, time);

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã chốt lịch hẹn thành công!\nTrạng thái: Chờ Khám");
        ui->txtPatientId->clear();
    } 
    else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh Nhân không tồn tại!");
    } 
    // Các lỗi -2, -3 trên thực tế sẽ RẤT HIẾM KHI xảy ra vì giao diện đã khóa chặt danh sách đúng.
    // Nhưng nó vẫn nằm đó làm "chốt chặn an toàn" bảo vệ CSDL!
    else {
        QMessageBox::critical(this, "Lỗi Xếp Lịch", "Có lỗi xảy ra, Bác sĩ không hợp lệ!");
    }
}

void ReceptionistMenu::on_btnCreateInvoice_clicked() {
    QMessageBox::information(this, "Thanh Toán", "Lập hóa đơn và thanh toán chi phí khám chữa bệnh.");
    string patientId = ui->txtPatientId_Invoice->text().toStdString();
    string feeStr = ui->txtMedFee->text().toStdString();

    if (patientId.empty() || feeStr.empty()) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng nhập Mã Bệnh Nhân và Số tiền cần thanh toán!");
        return;
    }
    double medFee = 0.0;
    try {
        medFee = stod(feeStr);
        if (medFee < 0) throw invalid_argument("Tiền âm");
    } catch (...) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Số tiền không hợp lệ! Vui lòng nhập số.");
        return;
    }
    int result = db.createInvoiceFromQT(patientId, medFee);
    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã lập Hóa đơn thành công!\nDữ liệu đã được ghi nhận vào hệ thống.");
        ui->txtPatientId_Invoice->clear();
        ui->txtMedFee->clear();
    } 
    else if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bệnh Nhân không tồn tại!\nVui lòng kiểm tra lại ID.");
    }
}

void ReceptionistMenu::on_btnSearchPatient_clicked() {
    QMessageBox::information(this, "Tra Cứu", "Tìm kiếm thông tin bệnh nhân theo ID hoặc tên.");
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
        QString msg = "Đã tìm thấy " + QString::number(foundPatients.size()) + " bệnh nhân:\n";
        for (Patient* p : foundPatients) {
            msg += QString::fromStdString("- " + p->getId() + ": " + p->getFullName()) + "\n";
        }
        QMessageBox::information(this, "Kết quả", msg);
        
        /* 
        // SAU NÀY BẠN HIỂN THỊ LÊN BẢNG (QTableWidget) NHƯ SAU:
        ui->tablePatients->setRowCount(0); // Xóa trắng bảng cũ
        for (int i = 0; i < foundPatients.size(); i++) {
            ui->tablePatients->insertRow(i);
            ui->tablePatients->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getId())));
            ui->tablePatients->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(foundPatients[i]->getFullName())));
            // ... thêm các cột khác
        }
        */
    }
}

void ReceptionistMenu::refreshPatientTable() {
    ui->tablePatients->setRowCount(0);
    auto& patients = db.getPatients();
    
    int row = 0;
    for (auto pat : patients) {
        ui->tablePatients->insertRow(row);
        ui->tablePatients->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(pat->getId())));
        ui->tablePatients->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(pat->getFullName())));
        ui->tablePatients->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(pat->getPhoneNumber())));
        ui->tablePatients->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(pat->getType()))); // NoiTru hoặc NgoaiTru
        row++;
    }
}