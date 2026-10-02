#ifndef ADMINMENU_H
#define ADMINMENU_H
#include <QMainWindow>
#include "../services/Database.h"
#include "../services/AuthService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AdminMenu; }
QT_END_NAMESPACE
class AdminMenu : public QMainWindow {
    Q_OBJECT 

public:
    explicit AdminMenu(Database& database, AuthService& authService, QWidget *parent = nullptr);
    ~AdminMenu();

private slots:
    void on_btnLogout_clicked();
    void on_btnManageDoctors_clicked();
    void on_btnManageUsers_clicked();
    void on_btnManageMedicines_clicked();
    void on_btnManageServices_clicked();

private:
    Ui::AdminMenu *ui; 
    Database& db;      
    AuthService& auth; 
    void refreshDoctorTable();
    void refreshUserTable();
};

#endif // ADMINMENU_H