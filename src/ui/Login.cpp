#include "ui/Login.h"
#include "ui_Login.h"
#include "ui/AdminMenu.h"
#include "ui/DoctorMenu.h"
#include "ui/ReceptionistMenu.h"
#include "ui/InventoryMenu.h"
#include "ui/ClinicStyle.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>

using namespace std;

Login::Login(Database& database, AuthService& authService, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Login),
    db(database),
    auth(authService)
{
    ui->setupUi(this);
    applyClinicStyle(this);
    ui->txtUsername->setMaxLength(32);
    ui->txtUsername->setPlaceholderText(QString::fromUtf8("Tài khoản không dấu (3–32 ký tự)"));
    ui->txtUsername->setValidator(new QRegularExpressionValidator(
        QRegularExpression("^[A-Za-z0-9._-]{3,32}$"), this));
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

   // 1. GỌI AUTH.LOGIN ĐỂ HỆ THỐNG GHI NHẬN PHIÊN ĐĂNG NHẬP
    if (!auth.login(username, password)) {
        QMessageBox::critical(this, "Đăng nhập thất bại", "Sai tài khoản hoặc mật khẩu!");
        return;
    }

    // 2. Rút User hiện tại ra để kiểm tra quyền hạn
    User* loggedInUser = auth.getCurrentUser();

    if (loggedInUser == nullptr) {
        QMessageBox::critical(this, "Đăng nhập thất bại", "Sai tài khoản hoặc mật khẩu!");
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
        if (db.findDoctorById(loggedInUser->getId()) == nullptr) {
            QMessageBox::critical(this, "Lỗi phân quyền", "Tài khoản chưa được liên kết với hồ sơ bác sĩ.");
            auth.logout();
            return;
        }
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
    else if (role == "Inventory" || role == "QuanLyKho" || role == "DuocSi") {
        InventoryMenu *invMenu = new InventoryMenu(this->db, this->auth);
        invMenu->setAttribute(Qt::WA_DeleteOnClose);
        invMenu->show();
        this->close();
    }
    else {
        QMessageBox::critical(this, "Lỗi Phân Quyền", "Quyền không hợp lệ!");
        auth.logout();
    }
}