#ifndef DOCTORMENU_H
#define DOCTORMENU_H
#include <QMainWindow>
#include "../services/Database.h"
#include "../services/AuthService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class DoctorMenu; }
QT_END_NAMESPACE

class DoctorMenu : public QMainWindow {
    Q_OBJECT

public:
    explicit DoctorMenu(Database& database, AuthService& authService, QWidget *parent = nullptr);
    ~DoctorMenu();

private slots:
    void on_btnLogout_clicked();
    void on_btnViewSchedule_clicked();
    void on_btnCreateMedicalRecord_clicked();
    void on_btnPrescribeMedicine_clicked();
    void on_btnViewPatientHistory_clicked();

private:
    Ui::DoctorMenu *ui;
    Database& db;
    AuthService& auth;
    
    void loadPatientQueue();
};

#endif // DOCTORMENU_H