#include "../../include/ui/AdminMenu.h"
#include "ui_AdminMenu.h"
#include <QMessageBox>

AdminMenu::AdminMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AdminMenu), db(database), auth(authService) {
    ui->setupUi(this);
    refreshDoctorTable();
    refreshUserTable();
}

AdminMenu::~AdminMenu() {
    delete ui;
}

void AdminMenu::on_btnLogout_clicked() {
    auth.logout();
    QMessageBox::information(this, "Đăng xuất", "Bạn đã thoát quyền Admin!");
    this->close();
}

void AdminMenu::on_btnManageDoctors_clicked() {
    QMessageBox::information(this, "Quản lý Bác sĩ", "Đang mở form Thêm/Sửa/Xóa Bác sĩ...");
}

void AdminMenu::on_btnManageUsers_clicked() {
    QMessageBox::information(this, "Quản lý Tài khoản", "Đang mở chức năng quản lý người dùng...");
}

void AdminMenu::on_btnManageMedicines_clicked() {
    QMessageBox::information(this, "Quản lý Thuốc", "Đang mở chức năng quản lý kho thuốc...");
}

void AdminMenu::on_btnManageServices_clicked() {
    QMessageBox::information(this, "Quản lý Dịch vụ", "Đang mở chức năng cấu hình dịch vụ y tế...");
}

void AdminMenu::refreshDoctorTable() {
    ui->tableDoctors->setRowCount(0);
    auto& doctors = db.getDoctors(); 
    int row = 0;
    for(const auto& doc : doctors) {
        ui->tableDoctors->insertRow(row);
        ui->tableDoctors->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(doc.getId())));
        ui->tableDoctors->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(doc.getFullName())));
        ui->tableDoctors->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(doc.getSpecialty())));
        ui->tableDoctors->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(doc.getPhoneNumber())));
        row++;
    }
}
void AdminMenu::refreshUserTable() {
    ui->tableUsers->setRowCount(0);
    auto& users = db.getUsers();
    int row = 0;
    for(const auto& u : users) {
        ui->tableUsers->insertRow(row);
        ui->tableUsers->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(u.getId())));
        ui->tableUsers->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(u.getUsername())));
        ui->tableUsers->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(u.getRole())));
        ui->tableUsers->setItem(row, 3, new QTableWidgetItem(u.getIsActive() ? "Hoat dong" : "Bi khoa"));
        row++;
    }
}