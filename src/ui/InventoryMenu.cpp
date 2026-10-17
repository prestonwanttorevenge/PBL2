#include "ui/InventoryMenu.h"
#include "ui_InventoryMenu.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <stdexcept>

using namespace std;

InventoryMenu::InventoryMenu(Database& database, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InventoryMenu),
    db(database)
{
    ui->setupUi(this);
    refreshMedicineTable(); // Nạp dữ liệu kho thuốc ngay khi mở giao diện
}

InventoryMenu::~InventoryMenu()
{
    delete ui;
}

void InventoryMenu::refreshMedicineTable() {
    ui->tableMedicines->setColumnCount(5);
    ui->tableMedicines->setHorizontalHeaderLabels({"Mã Thuốc", "Tên Thuốc", "Đơn Giá", "Liều Lượng", "Tồn Kho"});
    ui->tableMedicines->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableMedicines->setRowCount(0);
    vector<Medicine*> medList = db.getMedicines().getAllPointers();

    int row = 0;
    for (auto med : medList) {
        ui->tableMedicines->insertRow(row);
        ui->tableMedicines->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(med->getMedicineId())));
        ui->tableMedicines->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(med->getMedicineName())));
        ui->tableMedicines->setItem(row, 2, new QTableWidgetItem(QString::number(med->getUnitPrice(), 'f', 0)));
        ui->tableMedicines->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(med->getDosage())));
        ui->tableMedicines->setItem(row, 4, new QTableWidgetItem(QString::number(med->getStock())));
        row++;
    }
}

void InventoryMenu::on_btnImportMedicine_clicked()
{
    string medId = ui->txtMedId->text().toStdString();
    string medName = ui->txtMedName->text().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    string priceStr = ui->txtPrice->text().toStdString();
    int qty = ui->spinBoxQty->value();

    if (medId.empty() || medName.empty() || dosage.empty() || priceStr.empty() || qty <= 0) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng nhập đầy đủ thông tin và số lượng lớn hơn 0!");
        return;
    }

    double price = 0.0;
    try {
        price = stod(priceStr);
        if (price < 0) throw invalid_argument("Giá âm");
    } catch (...) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Giá thuốc không hợp lệ! Vui lòng chỉ nhập số.");
        return;
    }

    int result = db.addMedicineFromQT(medId, medName, price, dosage, qty);
    db.saveAllData(); // Lưu ngay xuống tệp CSV

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã thêm THUỐC MỚI vào danh mục!");
    }
    else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Thuốc đã tồn tại. Đã CỘNG DỒN số lượng vào kho!");
    }

    refreshMedicineTable(); // Cập nhật lại hiển thị trên bảng
    ui->txtMedId->clear();
    ui->txtMedName->clear();
    ui->txtDosage->clear();
    ui->txtPrice->clear();
    ui->spinBoxQty->setValue(1);
}

void InventoryMenu::on_btnLogout_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Đăng xuất",
                                                              "Bạn có chắc chắn muốn đăng xuất không?",
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        Login *loginForm = new Login(this->db);
        loginForm->show();
        this->close();
    }
}