#include "../../include/ui/DoctorMenu.h"
#include "ui_DoctorMenu.h"
#include <QMessageBox>

DoctorMenu::DoctorMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::DoctorMenu), db(database), auth(authService) {
    ui->setupUi(this);
    loadPatientQueue();
}

DoctorMenu::~DoctorMenu() {
    delete ui;
}

void DoctorMenu::on_btnLogout_clicked() {
    auth.logout();
    QMessageBox::information(this, "Đăng xuất", "Bác sĩ đã đăng xuất an toàn!");
    this->close();
}

void DoctorMenu::on_btnViewSchedule_clicked() {
    QMessageBox::information(this, "Lịch Trực", "Hiển thị lịch trực của bác sĩ trong tuần...");
}

void DoctorMenu::on_btnCreateMedicalRecord_clicked() {
    QMessageBox::information(this, "Hồ Sơ Bệnh Án", "Tạo và cập nhật bệnh án mới cho bệnh nhân.");
}

void DoctorMenu::on_btnPrescribeMedicine_clicked() {
    QMessageBox::information(this, "Kê Đơn Thuốc", "Mở danh sách thuốc để kê đơn.");
}

void DoctorMenu::on_btnViewPatientHistory_clicked() {
    QMessageBox::information(this, "Lịch Sử Khám", "Tra cứu lịch sử khám bệnh cũ của bệnh nhân.");
}

void DoctorMenu::loadPatientQueue() {
    ui->tableAppointments->setRowCount(0);
    auto& appts = db.getAppointments();
    
    int row = 0;
    for (const auto& appt : appts) {
        if (appt.getStatus() == "Pending") {
            ui->tableAppointments->insertRow(row);
            ui->tableAppointments->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(appt.getAppointmentId())));
            ui->tableAppointments->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(appt.getPatientId())));
            ui->tableAppointments->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(appt.getAppointmentDate())));
            row++;
        }
    }
}