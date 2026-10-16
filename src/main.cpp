#include <QApplication>
#include "ui/Login.h"
#include "services/Database.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Ngăn chặn app tự tắt khi form chuyển đổi
    a.setQuitOnLastWindowClosed(false);

    Database db;
    db.loadAllData();

    // SỬA Ở ĐÂY: Dùng con trỏ 'new' thay vì khai báo biến stack cục bộ
    Login *w = new Login(db);
    w->show();
    int result = a.exec(); 
    db.saveAllData(); 

    return result;
}