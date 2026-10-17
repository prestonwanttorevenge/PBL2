#ifndef INVENTORYMENU_H
#define INVENTORYMENU_H

#include <QMainWindow>
#include "services/Database.h"

QT_BEGIN_NAMESPACE
namespace Ui { class InventoryMenu; }
QT_END_NAMESPACE

class InventoryMenu : public QMainWindow
{
    Q_OBJECT

public:
    explicit InventoryMenu(Database& database, QWidget *parent = nullptr);
    ~InventoryMenu();

private slots:
    void on_btnImportMedicine_clicked();
    void on_btnLogout_clicked();

private:
    Ui::InventoryMenu *ui;
    Database& db;
    
    void refreshMedicineTable(); // Bổ sung hàm cập nhật bảng kho thuốc
};

#endif // INVENTORYMENU_H