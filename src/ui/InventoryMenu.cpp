#include "InventoryMenu.h"
#include "ui_InventoryMenu.h"
#include <QMessageBox>
#include <stdexcept>
#include <Login.h>
using namespace std;
InventoryMenu::InventoryMenu(QWidget *parent, Database *dbPointer) :
    QMainWindow(parent), ui(new Ui::InventoryMenu), db(dbPointer) {
    ui->setupUi(this);
}

// --- HÀM DỰNG (Khởi tạo giao diện) ---
InventoryMenu::InventoryMenu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InventoryMenu)
{
    ui->setupUi(this);
}

// --- HÀM HỦY (Giải phóng bộ nhớ) ---
InventoryMenu::~InventoryMenu()
{
    delete ui;
}

// --- BƯỚC 1: XỬ LÝ NÚT NHẬP THUỐC ---
void InventoryMenu::on_btnImportMedicine_clicked()
{
    // 1. Rút trích dữ liệu Quản lý kho vừa nhập
    string medId = ui->txtMedId->text().toStdString();
    string medName = ui->txtMedName->text().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    string priceStr = ui->txtPrice->text().toStdString();
    int qty = ui->spinBoxQty->value(); // QSpinBox chuyên dùng để chọn số nguyên

    // 2. Chốt chặn kiểm tra rỗng
    if (medId.empty() || medName.empty() || dosage.empty() || priceStr.empty() || qty <= 0) {
        QMessageBox::warning(this, "Cảnh báo", "Vui lòng nhập đầy đủ thông tin và số lượng lớn hơn 0!");
        return;
    }

    // 3. Xử lý ép kiểu dữ liệu tiền tệ (Double) một cách an toàn
    double price = 0.0;
    try {
        price = stod(priceStr);
        if (price < 0) throw invalid_argument("Giá âm");
    } catch (...) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Giá thuốc không hợp lệ! Vui lòng chỉ nhập số.");
        return;
    }

    // 4. Giao việc cho Backend: Thêm thuốc hoặc Cộng dồn số lượng

    int result = db.addMedicineFromQT(medId, medName, price, dosage, qty);

    // 5. Trả kết quả lên màn hình
    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã thêm THUỐC MỚI vào danh mục!");
    } 
    else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Thuốc đã tồn tại. Đã CỘNG DỒN số lượng vào kho!");
    }

    // 6. Xóa trắng form để tiện tay nhập lô thuốc tiếp theo
    ui->txtMedId->clear();
    ui->txtMedName->clear();
    ui->txtDosage->clear();
    ui->txtPrice->clear();
    ui->spinBoxQty->setValue(1);
}

// --- BƯỚC 2 (Tùy chọn): XỬ LÝ NÚT ĐĂNG XUẤT ---
void InventoryMenu::on_btnLogout_clicked()
{
    // Hiển thị hộp thoại xác nhận
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Đăng xuất", 
                                "Bạn có chắc chắn muốn đăng xuất không?", 
                                QMessageBox::Yes | QMessageBox::No);
                                
    if (reply == QMessageBox::Yes) {
        // Mở lại màn hình Login
        Login *loginForm = new Login();
        loginForm->show();
        this->close(); // Đóng giao diện Quản lý kho
    }
}
