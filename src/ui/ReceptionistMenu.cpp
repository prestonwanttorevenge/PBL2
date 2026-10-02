#include "../../include/ui/ReceptionistMenu.h"
#include "ui_ReceptionistMenu.h"
#include <QMessageBox>

ReceptionistMenu::ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ReceptionistMenu), db(database), auth(authService) {
    ui->setupUi(this);
    
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
}

void ReceptionistMenu::on_btnBookAppointment_clicked() {
    QMessageBox::information(this, "Đặt Lịch", "Xếp lịch hẹn với bác sĩ cho bệnh nhân.");
}

void ReceptionistMenu::on_btnCreateInvoice_clicked() {
    QMessageBox::information(this, "Thanh Toán", "Lập hóa đơn và thanh toán chi phí khám chữa bệnh.");
}

void ReceptionistMenu::on_btnSearchPatient_clicked() {
    QMessageBox::information(this, "Tra Cứu", "Tìm kiếm thông tin bệnh nhân theo ID hoặc tên.");
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