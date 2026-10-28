#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "services/Database.h"
#include "services/AuthService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QMainWindow
{
    Q_OBJECT

public:
    explicit Login(Database& db, AuthService& authService, QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_btnLogin_clicked();

private:
    Ui::Login *ui;
    Database& db;
    // Dùng chung phiên đăng nhập giữa form Login và các menu.
    AuthService& auth;
};

#endif // LOGIN_H