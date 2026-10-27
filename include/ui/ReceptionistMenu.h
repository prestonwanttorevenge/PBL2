#ifndef RECEPTIONISTMENU_H
#define RECEPTIONISTMENU_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include "services/Database.h"
#include "services/AuthService.h"
#include <map>
#include <vector>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui { class ReceptionistMenu; }
QT_END_NAMESPACE

class ReceptionistMenu : public QMainWindow {
    Q_OBJECT

public:
    explicit ReceptionistMenu(Database& database, AuthService& authService, QWidget *parent = nullptr);
    ~ReceptionistMenu();

private slots:
    void updateAvailableDoctors();
    void on_btnLogout_clicked();
    void on_btnRegisterPatient_clicked();   
    void on_btnBookAppointment_clicked();  
    void on_btnCreateInvoice_clicked();  
    void on_btnSearchPatient_clicked();
    void on_cbSpecialty_currentIndexChanged(const QString &specialtyText);
    void on_btnCancelAppointment_clicked();
    void on_btnCheckInAppointment_clicked();
    void on_cbPendingAppointments_currentIndexChanged(int index);
    void loadPendingAppointments();
    void on_btnAssignBed_clicked();
    void on_tablePatients_itemClicked(QTableWidgetItem *item);
    void loadAvailableRooms();
    void on_cbRoom_currentIndexChanged(const QString &roomName);

private:
    Ui::ReceptionistMenu *ui;
    Database& db;
    AuthService& auth;
    std::map<std::string, std::vector<std::string>> currentRoomMap;

    void refreshPatientTable();
};

#endif // RECEPTIONISTMENU_H