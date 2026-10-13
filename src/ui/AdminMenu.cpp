#include "../../include/ui/AdminMenu.h"
#include "ui_AdminMenu.h"
#include <QMessageBox>
#include "../../include/services/Database.h"
#include "InventoryMenu.h"
#include "DoctorMenu.h"
#include "ReceptionistMenu.h"
#include "Login.h"
AdminMenu::AdminMenu(QWidget *parent, Database& dbRef) :
    QMainWindow(parent),
    ui(new Ui::AdminMenu),
    db(dbRef)
{
    ui->setupUi(this);
    refreshDoctorTable();
    refreshUserTable();
    ui->stackedWidget->setCurrentIndex(0);
}
AdminMenu::AdminMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AdminMenu), db(database), auth(authService) {
    ui->setupUi(this);
    
}

AdminMenu::~AdminMenu() {
    delete ui;
}

// 1. Quản lý Tài khoản (Nằm ở Trang số 0)
void AdminMenu::on_btnManageUsers_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}
void AdminMenu::on_btnAddUser_clicked() {
    // 1. Hút 5 dữ liệu
    string id = ui->txtUserId->text().toStdString();
    string name = ui->txtUserFullName->text().toStdString();
    string username = ui->txtUsername->text().toStdString();
    string password = ui->txtPassword->text().toStdString();
    string role = ui->cbUserRole->currentText().toStdString(); // Combobox: Admin, Receptionist, Inventory

    // 2. Chặn lỗi
    if (id.empty() || name.empty() || username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đủ thông tin tài khoản!");
        return;
    }

    // 3. Gọi Backend (Tái sử dụng hàm addUserFromQT mà chúng ta đã bàn)
    int result = db.addUserFromQT(id, username, password, name, role);

    // 4. Phản hồi
    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "ID hoặc Username đã tồn tại!");
    } else {
        QMessageBox::information(this, "Thành công", "Đã tạo tài khoản mới!");
        refreshUserTable();
        
        // Dọn dẹp
        ui->txtUserId->clear();
        ui->txtUserFullName->clear();
        ui->txtUsername->clear();
        ui->txtPassword->clear();
    }
}
// 2. Quản lý Bác sĩ (Nằm ở Trang số 1)
void AdminMenu::on_btnManageDoctors_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
}
void AdminMenu::on_btnAddDoctor_clicked() {
    // --- BƯỚC 1: HÚT DỮ LIỆU TỪ GIAO DIỆN ---
    // 1.1 Dữ liệu chuyên môn Bác sĩ
    string id = ui->txtDocId->text().toStdString();
    string name = ui->txtDocName->text().toStdString();
    string dob = ui->txtDob->text().toStdString();
    string gender = ui->cbGender->currentText().toStdString(); // Lấy chữ "Nam" hoặc "Nữ"
    string phone = ui->txtPhone->text().toStdString();
    string spec = ui->txtDocSpecialty->text().toStdString();
    int expYears = ui->spinExpYears->value();

    // 1.2 Dữ liệu Tài khoản đăng nhập
    string username = ui->txtDocUsername->text().toStdString();
    string password = ui->txtDocPassword->text().toStdString();

    // --- BƯỚC 2: KIỂM TRA LỖI LOGIC BỀ MẶT (UI VALIDATION) ---
    // Ngăn chặn Admin bấm "Thêm" khi chưa nhập đủ thông tin
    if (id.empty() || name.empty() || dob.empty() || phone.empty() || 
        spec.empty() || username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng điền đầy đủ tất cả các trường thông tin!");
        return; // Chặn đứng tại đây, không cho code chạy tiếp xuống Database
    }

    // --- BƯỚC 3: GIAO VIỆC CHO BỘ NÃO BACKEND ---
    // Truyền đủ 9 tham số vào hàm 2-trong-1 đã viết ở Database.cpp
    int result = db.addDoctorWithAccount(id, name, dob, gender, phone, spec, expYears, username, password);

    // --- BƯỚC 4: XỬ LÝ PHẢN HỒI VÀ DỌN DẸP UI ---
    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Bác sĩ (ID) đã tồn tại trong hệ thống!");
    } 
    else if (result == -2) {
        QMessageBox::critical(this, "Lỗi", "Tên đăng nhập (Username) đã bị trùng, vui lòng chọn tên khác!");
    } 
    else if (result == 1) {
        // 4.1 Báo cáo thành công
        QMessageBox::information(this, "Thành công", "Đã thêm Bác sĩ mới và cấp tài khoản đăng nhập!");
        
        // 4.2 Cập nhật lại 2 bảng hiển thị (Bảng Bác sĩ ở trang 1 và Bảng User ở trang 0)
        refreshDoctorTable(); 
        refreshUserTable();   
        
        // 4.3 Xóa trắng các ô nhập liệu để chuẩn bị nhập người tiếp theo
        ui->txtDocId->clear();
        ui->txtDocName->clear();
        ui->txtDob->clear();
        ui->cbGender->setCurrentIndex(0); // Trả ComboBox về dòng đầu tiên (VD: "Nam")
        ui->txtPhone->clear();
        ui->txtDocSpecialty->clear();
        ui->spinExpYears->setValue(0);    // Trả số năm kinh nghiệm về 0
        
        ui->txtDocUsername->clear();
        ui->txtDocPassword->clear();
    }
}

// 3. Quản lý Thuốc (Nằm ở Trang số 2)
void AdminMenu::on_btnManageMedicines_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    refreshMedicineTable(); 
}
void AdminMenu::on_btnAddMedicine_clicked() {
    // 1. Hút 5 dữ liệu
    string id = ui->txtMedId->text().toStdString();
    string name = ui->txtMedName->text().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    string priceStr = ui->txtMedPrice->text().toStdString();
    int qty = ui->spinMedQty->value();

    if (id.empty() || name.empty() || dosage.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đủ thông tin thuốc!");
        return;
    }

    double price = 0.0;
    try { price = stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Lỗi", "Giá thuốc phải là số!"); return;
    }

    // 3. Gọi Backend
    int result = db.addMedicineFromQT(id, name, price, dosage, qty);

    // 4. Phản hồi
    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã thêm loại THUỐC MỚI!");
    } else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Đã CỘNG DỒN số lượng thuốc vào kho!");
    }
    
    refreshMedicineTable(); // viết hàm ni đi phát
    
    // Dọn dẹp
    ui->txtMedId->clear();
    ui->txtMedName->clear();
    ui->txtDosage->clear();
    ui->txtMedPrice->clear();
    ui->spinMedQty->setValue(1);
}

// 4. Quản lý Dịch vụ (Nằm ở Trang số 3)
void AdminMenu::on_btnManageServices_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
}
void AdminMenu::on_btnAddService_clicked() {
    // 1. Hút 3 dữ liệu
    string id = ui->txtServiceId->text().toStdString();
    string name = ui->txtServiceName->text().toStdString();
    string priceStr = ui->txtServicePrice->text().toStdString();

    if (id.empty() || name.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đủ thông tin dịch vụ!");
        return;
    }

    double price = 0.0;
    try { price = stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Lỗi", "Phí dịch vụ phải là số!"); return;
    }

    // 3. Gọi Backend (Giả sử bạn có hàm addServiceFromQT trong Database)
    int result = db.addServiceFromQT(id, name, price);

    // 4. Phản hồi
    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Mã Dịch vụ đã tồn tại!");
    } else {
        QMessageBox::information(this, "Thành công", "Đã thêm Dịch vụ y tế mới!");
        refreshServiceTable(); // Tải lại bảng dịch vụ
        
        // Dọn dẹp
        ui->txtServiceId->clear();
        ui->txtServiceName->clear();
        ui->txtServicePrice->clear();
    }
}

// 5. Nút Đăng xuất
void AdminMenu::on_btnLogout_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Đăng xuất", "Bạn có muốn thoát quyền Admin?");
    if (reply == QMessageBox::Yes) {
        // Trở về màn hình Login ban đầu
        Login *loginForm = new Login(nullptr, this->db);
        loginForm->show();
        this->close(); // Đóng Dashboard Admin
    }
}
void AdminMenu::refreshDoctorTable() {
    ui->tableDoctors->setRowCount(0);
    
    // 1. Rút danh bạ con trỏ từ Cây ra thành một mảng Vector
    vector<Doctor*> doctorsList = db.getDoctors().getAllPointers(); 
    
    int row = 0;
    // 2. Chạy vòng lặp trên mảng con trỏ (dùng auto cho gọn)
    for(auto doc : doctorsList) {
        ui->tableDoctors->insertRow(row);
        
        // 3. Đổi dấu chấm (.) thành mũi tên (->)
        ui->tableDoctors->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(doc->getId())));
        ui->tableDoctors->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(doc->getFullName())));
        ui->tableDoctors->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(doc->getSpecialty())));
        ui->tableDoctors->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(doc->getPhoneNumber())));
        row++;
    }
}

void AdminMenu::refreshUserTable() {
    ui->tableUsers->setRowCount(0);
    
    // 1. Rút danh bạ con trỏ User từ Cây ra
    vector<User*> usersList = db.getUsers().getAllPointers();
    
    int row = 0;
    // 2. Duyệt qua mảng con trỏ User*
    for(auto u : usersList) {
        ui->tableUsers->insertRow(row);
        
        // 3. Đổi dấu chấm (.) thành mũi tên (->)
        ui->tableUsers->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(u->getId())));
        ui->tableUsers->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(u->getUsername())));
        ui->tableUsers->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(u->getRole())));
        ui->tableUsers->setItem(row, 3, new QTableWidgetItem(u->getIsActive() ? "Hoat dong" : "Bi khoa"));
        row++;
    }
}