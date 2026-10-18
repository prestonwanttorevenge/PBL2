#include "ui/InventoryMenu.h"
#include "ui_InventoryMenu.h"
#include "ui/Login.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <stdexcept>

using namespace std;

InventoryMenu::InventoryMenu(Database& database, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InventoryMenu),
    db(database)
{
    ui->setupUi(this);

    // --- NÂNG CẤP UX: KHÓA CHỈ CHO NHẬP SỐ DƯƠNG VÀO Ô GIÁ TIỀN ---
    QRegularExpression rxPrice("^[0-9]*\\.?[0-9]+$");
    ui->txtPrice->setValidator(new QRegularExpressionValidator(rxPrice, this));

    // --- KẾT NỐI TÍN HIỆU LIVE SEARCH ---
    // (Lưu ý: Nếu UI của ngài chưa có ô txtSearchMed, xin bỏ qua hàm connect này)
    connect(ui->txtSearchMed, &QLineEdit::textChanged, this, &InventoryMenu::on_txtSearchMed_textChanged);

    refreshMedicineTable(); 
}

InventoryMenu::~InventoryMenu()
{
    delete ui;
}

// ==========================================
// NÂNG CẤP UX: LIVE SEARCH TRONG KHO THUỐC
// ==========================================
void InventoryMenu::on_txtSearchMed_textChanged(const QString &keyword) {
    ui->tableMedicines->setRowCount(0);
    vector<Medicine*> medList = db.getMedicines().getAllPointers();

    int row = 0;
    string searchStr = keyword.toStdString();
    
    // Biến đổi chuỗi tìm kiếm thành chữ thường để so khớp (Case-insensitive)
    for (char& c : searchStr) c = tolower(c);

    for (auto med : medList) {
        string medName = med->getMedicineName();
        string medId = med->getMedicineId();
        
        string lowerName = medName;
        for (char& c : lowerName) c = tolower(c);
        
        string lowerId = medId;
        for (char& c : lowerId) c = tolower(c);

        // Lọc kết quả: Nếu tìm thấy mã ID HOẶC Tên thuốc khớp với từ khóa
        if (searchStr.empty() || lowerId.find(searchStr) != string::npos || lowerName.find(searchStr) != string::npos) {
            ui->tableMedicines->insertRow(row);
            ui->tableMedicines->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(medId)));
            ui->tableMedicines->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(medName)));
            ui->tableMedicines->setItem(row, 2, new QTableWidgetItem(QString::number(med->getUnitPrice(), 'f', 0)));
            ui->tableMedicines->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(med->getDosage())));
            ui->tableMedicines->setItem(row, 4, new QTableWidgetItem(QString::number(med->getStock())));
            row++;
        }
    }
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
    db.saveAllData(); 

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã thêm THUỐC MỚI vào danh mục!");
    }
    else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Thuốc đã tồn tại. Đã CỘNG DỒN số lượng vào kho!");
    }

    refreshMedicineTable(); 
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