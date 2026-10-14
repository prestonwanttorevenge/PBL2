#ifndef INVENTORYMENU_H
#define INVENTORYMENU_H

#include <QMainWindow>
#include "services/Database.h" // Chuẩn hóa đường dẫn

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryMenu; }
QT_END_NAMESPACE

class InventoryMenu : public QMainWindow
{
    Q_OBJECT // Bắt buộc phải có để các nút bấm hoạt động

public:
    // Đã thêm tham số Database& để khởi tạo biến db
    explicit InventoryMenu(Database& database, QWidget *parent = nullptr);
    ~InventoryMenu();

private slots:
    // Nơi khai báo các nút bấm trên giao diện
    void on_btnImportMedicine_clicked(); // Nút Nhập thuốc / Thêm thuốc mới
    void on_btnLogout_clicked();         // Nút Đăng xuất (Tùy chọn)

private:
    Ui::InventoryMenu *ui;
    Database& db; // Biến tham chiếu phải được khởi tạo ở constructor
};

#endif // INVENTORYMENU_H