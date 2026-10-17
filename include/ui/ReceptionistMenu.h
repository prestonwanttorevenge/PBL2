#ifndef RECEPTIONISTMENU_H
#define RECEPTIONISTMENU_H
#include <QMainWindow>
#include "../services/Database.h"
#include "../services/AuthService.h"

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
    void loadPendingAppointments();
private:
    Ui::ReceptionistMenu *ui;
    Database& db;
    AuthService& auth;

    void refreshPatientTable();
};

#endif // RECEPTIONISTMENU_H