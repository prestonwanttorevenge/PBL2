#include "ui/Login.h"
#include "ui_Login.h"
#include "ui/AdminMenu.h"
#include "ui/DoctorMenu.h"
#include "ui/ReceptionistMenu.h"
#include "ui/InventoryMenu.h"
#include <QMessageBox>

using namespace std;

Login::Login(Database& database, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Login),
    db(database),
    auth(database)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose); // Giải phóng RAM khi đóng form
}

Login::~Login() {
    delete ui;
    db.saveAllData();
}

void Login::on_btnLogin_clicked() {
    string username = ui->txtUsername->text().toStdString();
    string password = ui->txtPassword->text().toStdString();

    if (username.empty() || password.empty()) {
        QMessageBox::warning(this, "Lỗi", "Vui lòng nhập đầy đủ tài khoản và mật khẩu!");
        return;
    }

    User* loggedInUser = db.loginFromQT(username, password);

    if (loggedInUser == nullptr) {
        QMessageBox::critical(this, "Đăng nhập thất bại", "Sai tài khoản hoặc mật khẩu!");
        return;
    }

    if (!loggedInUser->getIsActive()) {
        QMessageBox::warning(this, "Tài khoản bị khóa", "Tài khoản của bạn đã bị khóa. Vui lòng liên hệ Admin!");
        return;
    }

    string role = loggedInUser->getRole();

    // Khởi tạo các Menu & Xóa sạch RAM khi Menu đóng
    if (role == "Admin" || role == "QuanTri") {
        AdminMenu *adminMenu = new AdminMenu(this->db, this->auth);
        adminMenu->setAttribute(Qt::WA_DeleteOnClose);
        adminMenu->show();
        this->close();
    }
    else if (role == "BacSi" || role == "Doctor") {
        DoctorMenu *docMenu = new DoctorMenu(this->db, this->auth);
        docMenu->setAttribute(Qt::WA_DeleteOnClose);
        docMenu->show();
        this->close();
    }
    else if (role == "LeTan" || role == "Receptionist") {
        ReceptionistMenu *recMenu = new ReceptionistMenu(this->db, this->auth);
        recMenu->setAttribute(Qt::WA_DeleteOnClose);
        recMenu->show();
        this->close();
    }
    else if (role == "QuanLyKho" || role == "DuocSi") {
        InventoryMenu *invMenu = new InventoryMenu(this->db);
        invMenu->setAttribute(Qt::WA_DeleteOnClose);
        invMenu->show();
        this->close();
    }
    else {
        QMessageBox::critical(this, "Lỗi Phân Quyền", "Quyền không hợp lệ!");
    }
}