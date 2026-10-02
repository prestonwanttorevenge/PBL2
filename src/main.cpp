#include <QApplication>
#include "../include/services/Database.h"
#include "../include/services/AuthService.h"
#include "../include/ui/AdminMenu.h"
// #include "../include/ui/DoctorMenu.h"
// #include "../include/ui/ReceptionistMenu.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Khởi tạo Database và tải toàn bộ dữ liệu từ CSV
    Database db;
    db.loadAllData();

    // Khởi tạo dịch vụ xác thực (truyền db vào)
    AuthService auth(db);

    // KHỞI CHẠY GIAO DIỆN (Test thử form Admin)
    AdminMenu adminWindow(db, auth);
    adminWindow.show();

    // Chạy vòng lặp sự kiện của Qt
    int result = a.exec();

    // Tự động lưu toàn bộ dữ liệu xuống CSV trước khi tắt app
    db.saveAllData();

    return result;
}