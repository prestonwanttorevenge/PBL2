#include "Login.h"
#include "ui_Login.h"
// Bắt buộc include các Menu khác để mở form
#include "DoctorMenu.h"
#include "ReceptionistMenu.h"
#include "InventoryMenu.h" 
#include <QMessageBox>
Login::Login(QWidget *parent, Database& dbRef) : 
    QMainWindow(parent), ui(new Ui::Login), db(dbRef)
{
    ui->setupUi(this);
}
void Login::on_btnLogin_clicked() {
    // 1. Hút dữ liệu nhập vào
    string username = ui->txtUsername->text().toStdString();
    string password = ui->txtPassword->text().toStdString();

    if (username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đầy đủ tài khoản và mật khẩu!");
        return;
    }

    // 2. Giao việc cho Backend kiểm tra
    User* loggedInUser = db.loginFromQT(username, password);

    // 3. Xử lý kết quả trả về
    if (loggedInUser == nullptr) {
        QMessageBox::critical(this, "Đăng nhập thất bại", "Sai tài khoản hoặc mật khẩu!");
        return;
    }

    // 4. Kiểm tra trạng thái khóa tài khoản
    if (loggedInUser->getIsActive() == false) {
        QMessageBox::warning(this, "Tài khoản bị khóa", "Tài khoản của bạn đã bị khóa. Vui lòng liên hệ Admin!");
        return;
    }

    // 5. CHIA NGẢ ĐƯỜNG (ROUTING) DỰA VÀO QUYỀN
    string role = loggedInUser->getRole(); // Lấy quyền từ object User

    if (role == "Admin" || role == "QuanTri") {
    // Mở màn hình Admin (Quản lý nhân sự, thống kê doanh thu...)
    // AdminMenu *adminMenu = new AdminMenu();
    // adminMenu->show();
    QMessageBox::information(this, "Hệ thống", "Đăng nhập thành công với quyền Admin!");
    this->close(); 
}
// 5.2. Các quyền nghiệp vụ tiêu chuẩn
else if (role == "BacSi" || role == "Doctor") {
    DoctorMenu *docMenu = new DoctorMenu();
    docMenu->show();
    this->close();
} 
else if (role == "LeTan" || role == "Receptionist") {
    ReceptionistMenu *recMenu = new ReceptionistMenu();
    recMenu->show();
    this->close();
} 
else if (role == "QuanLyKho" || role == "DuocSi") {
    InventoryMenu *invMenu = new InventoryMenu();
    invMenu->show();
    this->close();
} 
// 5.3. CHỐT CHẶN AN TOÀN (Vô hiệu hóa mọi quyền lạ)
else {
    QMessageBox::critical(this, "Lỗi Phân Quyền", "Mã quyền [" + QString::fromStdString(role) + "] không hợp lệ!\nVui lòng liên hệ Quản trị viên để kiểm tra lại cơ sở dữ liệu.");
    // Không this->close(), bắt người dùng kẹt lại ở form Login
}

}