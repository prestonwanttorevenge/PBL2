#ifndef INVOICEDIALOG_H
#define INVOICEDIALOG_H

#include <QDialog>
#include <QString>
#include <string>
#include "services/Database.h"

class QLabel;
class QPushButton;
class QScrollArea;
class QWidget;

class InvoiceDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvoiceDialog(Database& database, const std::string& recordId, QWidget* parent = nullptr);
    ~InvoiceDialog() override = default;

    // Tra cứu hóa đơn hợp lệ từ Mã bệnh án (chỉ trả về khi bệnh nhân đã khám xong & đã lập hóa đơn).
    static std::string normalizeRecordId(const std::string& rawInput);
    static const Invoice* findInvoiceForRecord(
        Database& db,
        const std::string& recordId,
        const MedicalRecord** outRecord = nullptr,
        const Appointment** outAppt = nullptr);

private slots:
    void onPrintInvoice();
    void onExportPdf();

private:
    bool exportSheetToPdf(const QString& filePath);
    void buildUi(
        const Invoice* invoice,
        const MedicalRecord* record,
        const Appointment* appt,
        const Patient* patient,
        const Doctor* doctor);

    Database& db;
    std::string m_recordId;
    std::string m_invoiceId;

    QWidget* m_invoiceSheet = nullptr;
    QLabel* m_lblTitle = nullptr;
    QPushButton* m_btnPrint = nullptr;
    QPushButton* m_btnExportPdf = nullptr;
    QPushButton* m_btnClose = nullptr;
};

#endif // INVOICEDIALOG_H
