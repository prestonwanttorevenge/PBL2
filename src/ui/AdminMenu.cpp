#include "ui/AdminMenu.h"
#include "ui_AdminMenu.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <string>
#include <vector>

using std::string;
using std::vector;

AdminMenu::AdminMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AdminMenu), db(database), auth(authService)
{
    ui->setupUi(this);
    try {
        refreshUserTable();
        refreshDoctorTable();
        refreshMedicineTable();
        refreshServiceTable();
    } catch (...) {
    }
    
    ui->stackedWidget->setCurrentIndex(0);
}

AdminMenu::~AdminMenu() {
    delete ui;
}

// ==========================================
// 1. QUẢN LÝ TÀI KHOẢN (TRANG SỐ 0)
// ==========================================

void AdminMenu::on_btnManageUsers_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    refreshUserTable();
}

void AdminMenu::on_btnAddUser_clicked() {
    string id = ui->txtUserId->text().toStdString();
    string name = ui->txtUserFullName->text().toStdString();
    string username = ui->txtUsername->text().toStdString();
    string password = ui->txtPassword->text().toStdString();
    string role = ui->cbUserRole->currentText().toStdString();

    if (id.empty() || name.empty() || username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin tài khoản!");
        return;
    }

    int result = db.addUserFromQT(id, username, password, name, role);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã User ID hoặc Tên đăng nhập (Username) đã tồn tại!");
    } else {
        QMessageBox::information(this, "Thành công", "Đã tạo tài khoản mới thành công!");
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
    string id = ui->txtDocId->text().toStdString();
    string name = ui->txtDocName->text().toStdString();
    string dob = ui->txtDob->text().toStdString();
    string gender = ui->cbGender->currentText().toStdString();
    string phone = ui->txtPhone->text().toStdString();
    string spec = ui->txtDocSpecialty->text().toStdString();
    int expYears = ui->spinExpYears->value();

    string username = ui->txtDocUsername->text().toStdString();
    string password = ui->txtDocPassword->text().toStdString();

    if (id.empty() || name.empty() || dob.empty() || phone.empty() ||
        spec.empty() || username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng điền đầy đủ tất cả các trường thông tin Bác sĩ!");
        return;
    }

    int result = db.addDoctorWithAccount(id, name, dob, gender, phone, spec, expYears, username, password);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bác sĩ (ID) đã tồn tại trong hệ thống!");
    }
    else if (result == -2) {
        QMessageBox::critical(this, "Lỗi", "Tên đăng nhập (Username) đã bị trùng!");
    }
    else if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã thêm Bác sĩ mới và cấp tài khoản đăng nhập!");

        refreshDoctorTable();
        refreshUserTable();
        ui->txtDocId->clear();
        ui->txtDocName->clear();
        ui->txtDob->clear();
        ui->cbGender->setCurrentIndex(0);
        ui->txtPhone->clear();
        ui->txtDocSpecialty->clear();
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
    string id = ui->txtMedId->text().toStdString();
    string name = ui->txtMedName->text().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    string priceStr = ui->txtMedPrice->text().toStdString();
    int qty = ui->spinMedQty->value();

    if (id.empty() || name.empty() || dosage.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin thuốc!");
        return;
    }

    double price = 0.0;
    try {
        price = std::stod(priceStr);
    } catch (...) {
        QMessageBox::warning(this, "Lỗi kiểu dữ liệu", "Đơn giá thuốc phải là số!");
        return;
    }

    int result = db.addMedicineFromQT(id, name, price, dosage, qty);

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã thêm loại THUỐC MỚI vào kho!");
    } else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Đã CỘNG DỒN số lượng thuốc vào kho!");
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
    string id = ui->txtServiceId->text().toStdString();
    string name = ui->txtServiceName->text().toStdString();
    string priceStr = ui->txtServicePrice->text().toStdString();

    if (id.empty() || name.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin dịch vụ!");
        return;
    }

    double price = 0.0;
    try {
        price = std::stod(priceStr);
    } catch (...) {
        QMessageBox::warning(this, "Lỗi kiểu dữ liệu", "Phí dịch vụ phải là số!");
        return;
    }

    int result = db.addServiceFromQT(id, name, price);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Dịch vụ đã tồn tại!");
    } else {
        QMessageBox::information(this, "Thành công", "Đã thêm Dịch vụ y tế mới!");
        refreshServiceTable();
        ui->txtServiceId->clear();
        ui->txtServiceName->clear();
        ui->txtServicePrice->clear();
    }
}
// ==========================================
// XÓA TÀI KHOẢN (TRANG SỐ 0)
// ==========================================
void AdminMenu::on_btnDeleteUser_clicked() {
    string id = "";
    int row = ui->tableUsers->currentRow();

    // Ưu tiên lấy ID từ dòng đang click chọn trên bảng, nếu không chọn thì lấy từ ô txtUserId
    if (row >= 0) {
        id = ui->tableUsers->item(row, 0)->text().toStdString();
    } else {
        id = ui->txtUserId->text().toStdString();
    }

    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một tài khoản trên bảng (hoặc nhập User ID) để xóa!");
        return;
    }

    // Không cho phép xóa tài khoản đang đăng nhập hoặc tài khoản gốc (tuỳ chọn)
    if (id == "U01" || id == "ADMIN") {
        QMessageBox::critical(this, "Cảnh báo", "Không thể xóa tài khoản Quản trị viên gốc!");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận xóa",
                                       "Bạn có chắc chắn muốn xóa tài khoản có mã [" + QString::fromStdString(id) + "] không?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (db.deleteUserFromQT(id)) {
            db.saveAllData(); // Lưu xuống file CSV ngay lập tức
            QMessageBox::information(this, "Thành công", "Đã xóa tài khoản thành công!");
            refreshUserTable();
            ui->txtUserId->clear();
        } else {
            QMessageBox::critical(this, "Lỗi", "Không tìm thấy mã tài khoản này trong hệ thống!");
        }
    }
}

// ==========================================
// XÓA BÁC SĨ (TRANG SỐ 1)
// ==========================================
void AdminMenu::on_btnDeleteDoctor_clicked() {
    string id = "";
    int row = ui->tableDoctors->currentRow();
    if (row >= 0) {
        id = ui->tableDoctors->item(row, 0)->text().toStdString();
    } else {
        id = ui->txtDocId->text().toStdString();
    }

    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một Bác sĩ trên bảng để xóa!");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận xóa",
                                       "Xóa Bác sĩ [" + QString::fromStdString(id) + "] sẽ xóa luôn tài khoản đăng nhập của họ.\nBạn có chắc chắn muốn xóa?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (db.deleteDoctorFromQT(id)) {
            db.saveAllData();
            QMessageBox::information(this, "Thành công", "Đã xóa Bác sĩ và tài khoản liên kết!");
            refreshDoctorTable();
            refreshUserTable(); // Làm mới cả bảng User vì tài khoản Bác sĩ cũng bị xóa
            ui->txtDocId->clear();
        } else {
            QMessageBox::critical(this, "Lỗi", "Không tìm thấy Mã Bác sĩ này!");
        }
    }
}

// ==========================================
// XÓA THUỐC (TRANG SỐ 2)
// ==========================================
void AdminMenu::on_btnDeleteMedicine_clicked() {
    string id = "";
    int row = ui->tableMedicines->currentRow();
    if (row >= 0) {
        id = ui->tableMedicines->item(row, 0)->text().toStdString();
    } else {
        id = ui->txtMedId->text().toStdString();
    }

    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một loại thuốc trên bảng để xóa!");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận xóa",
                                       "Bạn có chắc chắn muốn xóa thuốc [" + QString::fromStdString(id) + "] khỏi kho?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (db.deleteMedicineFromQT(id)) {
            db.saveAllData();
            QMessageBox::information(this, "Thành công", "Đã xóa thuốc khỏi kho!");
            refreshMedicineTable();
            ui->txtMedId->clear();
        } else {
            QMessageBox::critical(this, "Lỗi", "Không tìm thấy Mã thuốc này!");
        }
    }
}

// ==========================================
// XÓA DỊCH VỤ Y TẾ (TRANG SỐ 3)
// ==========================================
void AdminMenu::on_btnDeleteService_clicked() {
    string id = "";
    int row = ui->tableServices->currentRow();
    if (row >= 0) {
        id = ui->tableServices->item(row, 0)->text().toStdString();
    } else {
        id = ui->txtServiceId->text().toStdString();
    }

    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một dịch vụ trên bảng để xóa!");
        return;
    }

    auto reply = QMessageBox::question(this, "Xác nhận xóa",
                                       "Bạn có chắc chắn muốn xóa dịch vụ [" + QString::fromStdString(id) + "] không?",
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (db.deleteServiceFromQT(id)) {
            db.saveAllData();
            QMessageBox::information(this, "Thành công", "Đã xóa dịch vụ y tế!");
            refreshServiceTable();
            ui->txtServiceId->clear();
        } else {
            QMessageBox::critical(this, "Lỗi", "Không tìm thấy Mã dịch vụ này!");
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
        auth.logout();
        // SỬA Ở ĐÂY: Hàm Login chỉ nhận tham số db
        Login *loginForm = new Login(this->db);
        loginForm->show();
        this->close();
    }
}

// ==========================================
// 6. CÁC HÀM LÀM MỚI BẢNG (REFRESH TABLES)
// ==========================================

void AdminMenu::refreshUserTable() {
    // THÊM 2 DÒNG NÀY ĐỂ BẢNG HIỆN CỘT:
    ui->tableUsers->setColumnCount(5);
    ui->tableUsers->setHorizontalHeaderLabels({"User ID", "Username", "Họ và Tên", "Vai trò", "Trạng thái"});

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
    // THÊM 2 DÒNG NÀY ĐỂ BẢNG HIỆN CỘT:
    ui->tableDoctors->setColumnCount(5);
    ui->tableDoctors->setHorizontalHeaderLabels({"Mã BS", "Họ Tên", "Chuyên Khoa", "SĐT", "Kinh Nghiệm"});

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
    // THÊM 2 DÒNG NÀY ĐỂ BẢNG HIỆN CỘT:
    ui->tableMedicines->setColumnCount(5);
    ui->tableMedicines->setHorizontalHeaderLabels({"Mã Thuốc", "Tên Thuốc", "Đơn Giá", "Liều Lượng", "Tồn Kho"});

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
    // THÊM 2 DÒNG NÀY ĐỂ BẢNG HIỆN CỘT:
    ui->tableServices->setColumnCount(3);
    ui->tableServices->setHorizontalHeaderLabels({"Mã Dịch Vụ", "Tên Dịch Vụ Y Tế", "Giá Tiền"});

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