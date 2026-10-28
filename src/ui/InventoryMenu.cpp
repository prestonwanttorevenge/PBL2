#include "ui/InventoryMenu.h"
#include "ui_InventoryMenu.h"
#include "ui/Login.h"
#include "ui/ClinicStyle.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <stdexcept>

using namespace std;

InventoryMenu::InventoryMenu(Database& database, AuthService& authService, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InventoryMenu),
    db(database),
    auth(authService)
{
    ui->setupUi(this);
    applyClinicStyle(this);
    setMinimumSize(820, 600);

    // --- NÂNG CẤP UX: KHÓA CHỈ CHO NHẬP SỐ DƯƠNG VÀO Ô GIÁ TIỀN ---
    QRegularExpression rxPrice("^[0-9]*\\.?[0-9]+$");
    ui->txtPrice->setValidator(new QRegularExpressionValidator(rxPrice, this));

    // --- KẾT NỐI TÍN HIỆU LIVE SEARCH ---
    // (Lưu ý: Nếu UI của ngài chưa có ô txtSearchMed, xin bỏ qua hàm connect này)
    connect(ui->txtSearchMed, &QLineEdit::textChanged, this, &InventoryMenu::on_txtSearchMed_textChanged);

    refreshMedicineTable();
    refreshPrescriptionQueue();
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

void InventoryMenu::refreshPrescriptionQueue() {
    ui->tablePendingPrescriptions->setColumnCount(5);
    ui->tablePendingPrescriptions->setHorizontalHeaderLabels(
        {"Mã bệnh án", "Mã bệnh nhân", "Thuốc", "Liều dùng", "Số lượng"});
    ui->tablePendingPrescriptions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tablePendingPrescriptions->setRowCount(0);

    int row = 0;
    for (MedicalRecord& record : db.getMedicalRecords()) {
        Appointment* appointment = db.findAppointmentById(record.getAppointmentId());
        if (appointment == nullptr || appointment->getStatus() != "Completed") continue;

        // Chỉ đưa đơn chưa cấp của lượt khám đã hoàn tất vào hàng đợi.
        for (const PrescriptionDetail& prescription : record.getPrescriptions()) {
            if (prescription.dispensed) continue;

            Medicine* medicine = db.findMedicineById(prescription.medicineId);
            const string medicineName = medicine == nullptr
                                            ? prescription.medicineId + " (đã ngừng kinh doanh)"
                                            : prescription.medicineId + " - " + medicine->getMedicineName();
            ui->tablePendingPrescriptions->insertRow(row);
            ui->tablePendingPrescriptions->setItem(row, 0,
                new QTableWidgetItem(QString::fromStdString(record.getRecordId())));
            ui->tablePendingPrescriptions->setItem(row, 1,
                new QTableWidgetItem(QString::fromStdString(appointment->getPatientId())));
            ui->tablePendingPrescriptions->setItem(row, 2,
                new QTableWidgetItem(QString::fromStdString(medicineName)));
            ui->tablePendingPrescriptions->setItem(row, 3,
                new QTableWidgetItem(QString::fromStdString(prescription.dosage)));
            ui->tablePendingPrescriptions->setItem(row, 4,
                new QTableWidgetItem(QString::number(prescription.quantity)));
            ui->tablePendingPrescriptions->item(row, 0)->setData(Qt::UserRole,
                QString::fromStdString(prescription.medicineId));
            ++row;
        }
    }

    ui->btnDispensePrescription->setEnabled(row > 0);
}

void InventoryMenu::on_btnDispensePrescription_clicked() {
    const int row = ui->tablePendingPrescriptions->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Chưa chọn đơn thuốc", "Chọn một dòng đơn thuốc cần cấp phát.");
        return;
    }

    const string recordId = ui->tablePendingPrescriptions->item(row, 0)->text().toStdString();
    const string medicineId =
        ui->tablePendingPrescriptions->item(row, 0)->data(Qt::UserRole).toString().toStdString();
    const auto reply = QMessageBox::question(
        this, "Xác nhận cấp phát",
        "Cấp phát " + ui->tablePendingPrescriptions->item(row, 2)->text() +
            " cho bệnh nhân [" + ui->tablePendingPrescriptions->item(row, 1)->text() +
            "] với số lượng " + ui->tablePendingPrescriptions->item(row, 4)->text() + "?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    const int result = db.dispensePrescriptionFromQT(recordId, medicineId);

    if (result == 1) {
        QMessageBox::information(this, "Đã cấp phát", "Thuốc đã được cấp phát và tồn kho đã cập nhật.");
        refreshMedicineTable();
        refreshPrescriptionQueue();
    } else if (result == -3) {
        QMessageBox::warning(this, "Không đủ tồn kho", "Tồn kho hiện tại không đủ để cấp phát đơn này.");
    } else if (result == -4) {
        QMessageBox::warning(this, "Đã cấp phát", "Đơn thuốc này đã được cấp phát trước đó.");
        refreshPrescriptionQueue();
    } else {
        QMessageBox::critical(this, "Không thể cấp phát", "Đơn thuốc hoặc hồ sơ khám không còn hợp lệ.");
        refreshPrescriptionQueue();
    }
}

void InventoryMenu::on_btnImportMedicine_clicked() {
    string prefix = ui->cbMedPrefix->currentText().left(2).toStdString();
    string medName = ui->txtMedName->text().toStdString();
    string dosage = ui->txtDosage->text().toStdString();
    string priceStr = ui->txtPrice->text().toStdString();
    int qty = ui->spinBoxQty->value();

    if (medName.empty() || dosage.empty() || priceStr.empty() || qty <= 0) {
        QMessageBox::warning(this, "Thiếu thông tin", "Nhập tên, dạng thuốc, giá và số lượng lớn hơn 0.");
        return;
    }

    double price = 0.0;
    try { price = stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Giá không hợp lệ", "Kiểm tra lại giá nhập thuốc.");
        return;
    }
    string finalId = ""; // Biến hứng ID từ Backend trả về
    
    int result = db.addMedicineFromQT(prefix, medName, price, dosage, qty, finalId);
    if (result < 0) {
        QMessageBox::warning(this, "Không thể nhập kho", "Dữ liệu thuốc hoặc số lượng nhập không hợp lệ.");
        return;
    }
    db.saveAllData(); 

    if (result == 1) {
        QMessageBox::information(this, "Thành công", "Đã phân loại và thêm THUỐC MỚI!\nMã ID: " + QString::fromStdString(finalId));
    }
    else if (result == 2) {
        QMessageBox::information(this, "Thành công", "Thuốc đã tồn tại (" + QString::fromStdString(finalId) + "). Đã CỘNG DỒN vào kho!");
    }

    refreshMedicineTable(); 
    refreshPrescriptionQueue();
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
        auth.logout();
        Login *loginForm = new Login(this->db, this->auth);
        loginForm->show();
        this->close();
    }
}