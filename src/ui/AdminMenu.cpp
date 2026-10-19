#include "ui/AdminMenu.h"
#include "ui_AdminMenu.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <string>
#include <vector>

using std::string;
using std::vector;

AdminMenu::AdminMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AdminMenu), db(database), auth(authService)
{
    ui->setupUi(this);

    // --- KIỂM SOÁT ĐẦU VÀO BẰNG REGEX ---
    QRegularExpression rxPhone("^[0-9]{10,11}$");
    ui->txtPhone->setValidator(new QRegularExpressionValidator(rxPhone, this));

    QRegularExpression rxPrice("^[0-9]*\\.?[0-9]+$");
    ui->txtMedPrice->setValidator(new QRegularExpressionValidator(rxPrice, this));
    ui->txtServicePrice->setValidator(new QRegularExpressionValidator(rxPrice, this));

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
    ui->cbDocSpecialty->addItems(specialties);

    // --- KẾT NỐI SỰ KIỆN CLICK-TO-FILL ---
    connect(ui->tableUsers, &QTableWidget::itemClicked, this, &AdminMenu::on_tableUsers_itemClicked);
    connect(ui->tableDoctors, &QTableWidget::itemClicked, this, &AdminMenu::on_tableDoctors_itemClicked);
    connect(ui->tableMedicines, &QTableWidget::itemClicked, this, &AdminMenu::on_tableMedicines_itemClicked);
    connect(ui->tableServices, &QTableWidget::itemClicked, this, &AdminMenu::on_tableServices_itemClicked);

    try {
        refreshUserTable();
        refreshDoctorTable();
        refreshMedicineTable();
        refreshServiceTable();
    } catch (...) {}
    
    ui->stackedWidget->setCurrentIndex(0);
}

AdminMenu::~AdminMenu() {
    delete ui;
}

// ==========================================
// CÁC HÀM CLICK-TO-FILL (DÙNG CHO VIỆC XÓA)
// ==========================================
void AdminMenu::on_tableUsers_itemClicked(QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0) ui->txtUserId->setText(ui->tableUsers->item(row, 0)->text());
}

void AdminMenu::on_tableDoctors_itemClicked(QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0) ui->txtDocId->setText(ui->tableDoctors->item(row, 0)->text());
}

void AdminMenu::on_tableMedicines_itemClicked(QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0) ui->txtMedId->setText(ui->tableMedicines->item(row, 0)->text());
}

void AdminMenu::on_tableServices_itemClicked(QTableWidgetItem *item) {
    int row = item->row();
    if (row >= 0) ui->txtServiceId->setText(ui->tableServices->item(row, 0)->text());
}


// ==========================================
// 1. QUẢN LÝ TÀI KHOẢN (TRANG SỐ 0)
// ==========================================

void AdminMenu::on_btnManageUsers_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    refreshUserTable();
}

void AdminMenu::on_btnAddUser_clicked() {
    string name = ui->txtUserFullName->text().toStdString();
    string username = ui->txtUsername->text().toStdString();
    string password = ui->txtPassword->text().toStdString();
    string role = ui->cbUserRole->currentText().toStdString();

    if (name.empty() || username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin tài khoản!");
        return;
    }
    
    // NÂNG CẤP: Chặn Admin tạo acc BS tay ngang
    if (role == "Doctor") {
        QMessageBox::warning(this, "Cảnh báo", "Tài khoản Bác sĩ sẽ được tự động cấp phát khi bạn thêm Bác sĩ mới bên thẻ [Bác Sĩ]!");
        return;
    }

    // NÂNG CẤP: Tự động cấp ID theo Role
    string generatedId = db.generateUserId(role);
    int result = db.addUserFromQT(generatedId, username, password, name, role);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Tên đăng nhập (Username) đã tồn tại!");
    } else {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã tạo tài khoản thành công với Mã: " + QString::fromStdString(generatedId));
        refreshUserTable();
        ui->txtUserId->clear();
        ui->txtUserFullName->clear();
        ui->txtUsername->clear();
        ui->txtPassword->clear();
    }
}

// ==========================================
// 2. QUẢN LÝ BÁC SĨ (TRANG SỐ 1)
// ==========================================

void AdminMenu::on_btnManageDoctors_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
    refreshDoctorTable();
}

void AdminMenu::on_btnAddDoctor_clicked() {
    string name = ui->txtDocName->text().toStdString();
    string dob = ui->txtDob->text().toStdString();
    string gender = ui->cbGender->currentText().toStdString();
    string phone = ui->txtPhone->text().toStdString();
    string spec = ui->cbDocSpecialty->currentText().toStdString(); 
    int expYears = ui->spinExpYears->value();

    string username = ui->txtDocUsername->text().toStdString();
    string password = ui->txtDocPassword->text().toStdString();

    if (name.empty() || dob.empty() || phone.empty() || spec.empty() || username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng điền đầy đủ tất cả các trường thông tin Bác sĩ!");
        return;
    }

    // NÂNG CẤP: Tự động cấp mã BS
    string generatedId = db.generateDoctorId();
    int result = db.addDoctorWithAccount(generatedId, name, dob, gender, phone, spec, expYears, username, password);

    if (result == -2) {
        QMessageBox::critical(this, "Lỗi", "Tên đăng nhập (Username) đã bị trùng!");
    }
    else if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã thêm Bác sĩ thành công!\nMã BS cấp phát: " + QString::fromStdString(generatedId));

        refreshDoctorTable();
        refreshUserTable();
        ui->txtDocId->clear();
        ui->txtDocName->clear();
        ui->txtDob->clear();
        ui->cbGender->setCurrentIndex(0);
        ui->txtPhone->clear();
        ui->cbDocSpecialty->setCurrentIndex(0);
        ui->spinExpYears->setValue(0);
        ui->txtDocUsername->clear();
        ui->txtDocPassword->clear();
    }
}

// ==========================================
// 3. QUẢN LÝ THUỐC (TRANG SỐ 2)
// ==========================================

void AdminMenu::on_btnManageMedicines_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    refreshMedicineTable();
}

void AdminMenu::on_btnAddMedicine_clicked() {
    // Rút 2 ký tự đầu tiên từ ComboBox (VD: "B1", "B2")
    string prefix = ui->cbMedPrefix->currentText().left(2).toStdString();
    
    string name = ui->txtMedName->text().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    string priceStr = ui->txtMedPrice->text().toStdString();
    int qty = ui->spinMedQty->value();

    if (name.empty() || dosage.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin thuốc!");
        return;
    }

    double price = 0.0;
    try { price = std::stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Lỗi", "Đơn giá thuốc không hợp lệ!");
        return;
    }

    // NÂNG CẤP: Dùng chuẩn 6 tham số của hệ thống mới
    string finalId = ""; // Hứng ID do Backend cấp phát trả về
    int result = db.addMedicineFromQT(prefix, name, price, dosage, qty, finalId);
    db.saveAllData(); 

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã khởi tạo THUỐC MỚI vào kho!\nMã cấp: " + QString::fromStdString(finalId));
    } else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Thuốc đã tồn tại (" + QString::fromStdString(finalId) + "). Đã CỘNG DỒN vào kho!");
    }

    refreshMedicineTable();
    ui->txtMedId->clear();
    ui->txtMedName->clear();
    ui->txtDosage->clear();
    ui->txtMedPrice->clear();
    ui->spinMedQty->setValue(1);
}

// ==========================================
// 4. QUẢN LÝ DỊCH VỤ Y TẾ (TRANG SỐ 3)
// ==========================================

void AdminMenu::on_btnManageServices_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
    refreshServiceTable();
}

void AdminMenu::on_btnAddService_clicked() {
    string name = ui->txtServiceName->text().toStdString();
    string priceStr = ui->txtServicePrice->text().toStdString();
    
    // Rút 2 ký tự đầu tiên từ ComboBox (Ví dụ: "S1", "S2")
    string prefix = ui->cbServicePrefix->currentText().left(2).toStdString();

    if (name.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin dịch vụ!");
        return;
    }

    double price = 0.0;
    try { price = std::stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Lỗi", "Phí dịch vụ không hợp lệ!");
        return;
    }
    string finalId = ""; // Biến chứa ID trả về
    
    int result = db.addServiceFromQT(prefix, name, price, finalId);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Dịch vụ này đã tồn tại trong hệ thống!\nMã dịch vụ trùng: " + QString::fromStdString(finalId));
    } else {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã phân loại và thêm Dịch vụ y tế mới!\nMã tự cấp phát: " + QString::fromStdString(finalId));
        
        refreshServiceTable();
        ui->txtServiceName->clear();
        ui->txtServicePrice->clear();
    }
}
// ==========================================
// CÁC HÀM XÓA (GIỮ NGUYÊN)
// ==========================================
void AdminMenu::on_btnDeleteUser_clicked() {
    string id = ui->txtUserId->text().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một tài khoản trên bảng để xóa!");
        return;
    }
    if (id == "U01" || id == "ADMIN" || id == "ADMIN01") {
        QMessageBox::critical(this, "Cảnh báo", "Không thể xóa tài khoản Quản trị viên gốc!");
        return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận xóa",
                                       "Xóa tài khoản có mã [" + QString::fromStdString(id) + "]?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (db.deleteUserFromQT(id)) {
            db.saveAllData(); refreshUserTable(); ui->txtUserId->clear();
        } else QMessageBox::critical(this, "Lỗi", "Không tìm thấy mã tài khoản!");
    }
}

void AdminMenu::on_btnDeleteDoctor_clicked() {
    string id = ui->txtDocId->text().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một Bác sĩ trên bảng để xóa!");
        return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận", "Xóa Bác sĩ [" + QString::fromStdString(id) + "] sẽ xóa luôn tài khoản liên kết?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int result = db.deleteDoctorFromQT(id);
        if (result == 1) {
            db.saveAllData(); refreshDoctorTable(); refreshUserTable(); ui->txtDocId->clear();
        } 
        else if (result == -2) QMessageBox::critical(this, "Lỗi Ràng Buộc", "TỪ CHỐI XÓA!\nBác sĩ đang có Lịch hẹn chờ khám.");
    }
}

void AdminMenu::on_btnDeleteMedicine_clicked() {
    string id = ui->txtMedId->text().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn thuốc trên bảng để xóa!"); return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận", "Xóa thuốc [" + QString::fromStdString(id) + "] khỏi kho?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int result = db.deleteMedicineFromQT(id);
        if (result == 1) {
            db.saveAllData(); refreshMedicineTable(); ui->txtMedId->clear();
        } 
        else if (result == -2) QMessageBox::critical(this, "Lỗi", "Thuốc đã được kê trong Bệnh án, không thể xóa!");
    }
}

void AdminMenu::on_btnDeleteService_clicked() {
    string id = ui->txtServiceId->text().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn dịch vụ trên bảng!"); return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận", "Xóa dịch vụ [" + QString::fromStdString(id) + "]?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (db.deleteServiceFromQT(id)) {
            db.saveAllData(); refreshServiceTable(); ui->txtServiceId->clear();
        }
    }
}

// ==========================================
// 5. ĐĂNG XUẤT HỆ THỐNG
// ==========================================

void AdminMenu::on_btnLogout_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Đăng xuất", "Bạn có chắc chắn muốn thoát quyền Admin?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        Login *loginForm = new Login(this->db);
        loginForm->show();
        this->close();
    }
}

// ==========================================
// 6. CÁC HÀM LÀM MỚI BẢNG (REFRESH TABLES)
// ==========================================

void AdminMenu::refreshUserTable() {
    ui->tableUsers->setColumnCount(5);
    ui->tableUsers->setHorizontalHeaderLabels({"User ID", "Username", "Họ và Tên", "Vai trò", "Trạng thái"});
    ui->tableUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableUsers->setRowCount(0);
    vector<User*> usersList = db.getUsers().getAllPointers();

    int row = 0;
    for (auto u : usersList) {
        ui->tableUsers->insertRow(row);
        ui->tableUsers->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(u->getId())));
        ui->tableUsers->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(u->getUsername())));
        ui->tableUsers->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(u->getFullName())));
        ui->tableUsers->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(u->getRole())));
        ui->tableUsers->setItem(row, 4, new QTableWidgetItem(u->getIsActive() ? "Hoạt động" : "Bị khóa"));
        row++;
    }
}

void AdminMenu::refreshDoctorTable() {
    ui->tableDoctors->setColumnCount(5);
    ui->tableDoctors->setHorizontalHeaderLabels({"Mã BS", "Họ Tên", "Chuyên Khoa", "SĐT", "Kinh Nghiệm"});
    ui->tableDoctors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableDoctors->setRowCount(0);
    vector<Doctor*> doctorsList = db.getDoctors().getAllPointers();

    int row = 0;
    for (auto doc : doctorsList) {
        ui->tableDoctors->insertRow(row);
        ui->tableDoctors->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(doc->getId())));
        ui->tableDoctors->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(doc->getFullName())));
        ui->tableDoctors->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(doc->getSpecialty())));
        ui->tableDoctors->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(doc->getPhoneNumber())));
        ui->tableDoctors->setItem(row, 4, new QTableWidgetItem(QString::number(doc->getYearsOfExperience())));
        row++;
    }
}

void AdminMenu::refreshMedicineTable() {
    ui->tableMedicines->setColumnCount(5);
    ui->tableMedicines->setHorizontalHeaderLabels({"Mã Thuốc", "Tên Thuốc", "Đơn Giá", "Liều Lượng", "Tồn Kho"});
    ui->tableMedicines->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableMedicines->setRowCount(0);
    vector<Medicine*> medList = db.getMedicines().getAllPointers();

    int row = 0;
    for (auto med : medList) {
        ui->tableMedicines->insertRow(row);
        ui->tableMedicines->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(med->getMedicineId())));
        ui->tableMedicines->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(med->getMedicineName())));
        ui->tableMedicines->setItem(row, 2, new QTableWidgetItem(QString::number(med->getUnitPrice(), 'f', 0)));
        ui->tableMedicines->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(med->getDosage())));
        ui->tableMedicines->setItem(row, 4, new QTableWidgetItem(QString::number(med->getStock())));
        row++;
    }
}

void AdminMenu::refreshServiceTable() {
    ui->tableServices->setColumnCount(3);
    ui->tableServices->setHorizontalHeaderLabels({"Mã Dịch Vụ", "Tên Dịch Vụ Y Tế", "Giá Tiền"});
    ui->tableServices->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableServices->setRowCount(0);
    vector<Service*> srvList = db.getServices().getAllPointers();

    int row = 0;
    for (auto srv : srvList) {
        ui->tableServices->insertRow(row);
        ui->tableServices->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(srv->getId())));
        ui->tableServices->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(srv->getServiceName())));
        ui->tableServices->setItem(row, 2, new QTableWidgetItem(QString::number(srv->getPrice(), 'f', 0)));
        row++;
    }
}