#ifndef INVENTORYMENU_H
#define INVENTORYMENU_H

#include <QMainWindow>
#include "services/Database.h"
#include "services/AuthService.h"

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryMenu; }
QT_END_NAMESPACE

class InventoryMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit InventoryMenu(Database& database, AuthService& authService, QWidget *parent = nullptr);
    ~InventoryMenu();

private slots:
    void on_btnImportMedicine_clicked();
    // Xác nhận cấp thuốc từ hàng đợi đơn chưa cấp.
    void on_btnDispensePrescription_clicked();
    void on_btnLogout_clicked();
    void on_txtSearchMed_textChanged(const QString &arg1); // MỚI: Tự tìm kiếm khi gõ

private:
    Ui::InventoryMenu *ui;
    Database& db;
    AuthService& auth;
    
    void refreshMedicineTable(); 
    void refreshPrescriptionQueue();
};

#endif // INVENTORYMENU_H