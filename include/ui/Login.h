#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "Database.h"  // Nhớ include Database để có thể gọi hàm loginFromQT

// Dòng này báo cho C++ biết có một giao diện UI tên là Login được vẽ bên file .ui
QT_BEGIN_NAMESPACE
namespace Ui { class Login; }
QT_END_NAMESPACE

class Login : public QMainWindow
{
    // Q_OBJECT là "linh hồn" của Qt, bắt buộc phải có để các nút bấm (signal/slot) hoạt động
    Q_OBJECT

public:
    // Hàm dựng (Constructor) và Hàm hủy (Destructor)
    explicit Login(QWidget *parent = nullptr);
    ~Login();

private slots:
    // BẠN CẦN THÊM DÒNG NÀY: Khai báo hàm bắt sự kiện click nút Đăng nhập
    void on_btnLogin_clicked();

private:
    Ui::Login *ui; // Con trỏ ui dùng để móc nối tới các ô txtUsername, txtPassword trên màn hình
    Database &db; 
};

#endif // LOGIN_H