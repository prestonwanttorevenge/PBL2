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
    // Hàm khởi tạo chỉ nhận Database&, khớp với lệnh gọi từ lúc Đăng xuất
    explicit Login(Database& db, QWidget *parent = nullptr);
    ~Login();

private slots:
    void on_btnLogin_clicked();

private:
    Ui::Login *ui;
    Database& db;
    AuthService auth; // Tự động ôm AuthService bên trong Login
};

#endif // LOGIN_H