#include "ui/InvoiceDialog.h"

#include <QApplication>
#include <QDate>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLocale>
#include <QMarginsF>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPainterPath>
#include <QPdfWriter>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <sstream>
#include <vector>

namespace {

QString formatVnd(long long amount) {
    if (amount == 0) return QStringLiteral("0");
    bool negative = amount < 0;
    unsigned long long val = negative
        ? static_cast<unsigned long long>(-(amount + 1)) + 1ULL
        : static_cast<unsigned long long>(amount);

    QString digits = QString::number(val);
    QString formatted;
    int count = 0;
    for (int i = digits.length() - 1; i >= 0; --i) {
        if (count > 0 && count % 3 == 0) {
            formatted.prepend('.');
        }
        formatted.prepend(digits.at(i));
        ++count;
    }
    if (negative) formatted.prepend('-');
    return formatted;
}

QString readThreeDigits(int num, bool isFirstBlock) {
    static const QString digits[10] = {
        QStringLiteral("không"), QStringLiteral("một"), QStringLiteral("hai"),
        QStringLiteral("ba"), QStringLiteral("bốn"), QStringLiteral("năm"),
        QStringLiteral("sáu"), QStringLiteral("bảy"), QStringLiteral("tám"),
        QStringLiteral("chín")
    };

    const int hundreds = num / 100;
    const int tens = (num % 100) / 10;
    const int units = num % 10;

    QStringList parts;
    if (hundreds > 0 || !isFirstBlock) {
        parts << digits[hundreds] << QStringLiteral("trăm");
    }

    if (tens == 0 && units > 0) {
        if (hundreds > 0 || !isFirstBlock) {
            parts << QStringLiteral("linh");
        }
        parts << digits[units];
    } else if (tens == 1) {
        parts << QStringLiteral("mười");
        if (units == 5) {
            parts << QStringLiteral("lăm");
        } else if (units > 0) {
            parts << digits[units];
        }
    } else if (tens > 1) {
        parts << digits[tens] << QStringLiteral("mươi");
        if (units == 1) {
            parts << QStringLiteral("mốt");
        } else if (units == 5) {
            parts << QStringLiteral("lăm");
        } else if (units > 0) {
            parts << digits[units];
        }
    }

    return parts.join(' ');
}

QString readVietnameseMoney(long long amount) {
    if (amount == 0) {
        return QStringLiteral("Không đồng chẵn");
    }

    bool negative = amount < 0;
    unsigned long long val = negative
        ? static_cast<unsigned long long>(-(amount + 1)) + 1ULL
        : static_cast<unsigned long long>(amount);

    static const QString scales[4] = {
        QStringLiteral(""),
        QStringLiteral("nghìn"),
        QStringLiteral("triệu"),
        QStringLiteral("tỷ")
    };

    std::vector<int> blocks;
    while (val > 0) {
        blocks.push_back(static_cast<int>(val % 1000ULL));
        val /= 1000ULL;
    }

    QStringList segments;
    for (int i = static_cast<int>(blocks.size()) - 1; i >= 0; --i) {
        const int block = blocks[static_cast<size_t>(i)];
        if (block == 0) {
            // Giữ hậu tố "tỷ" nếu có nhóm lớn hơn tỷ nhưng phần nghìn/triệu của chu kỳ tỷ = 0
            if (i > 0 && i % 3 == 0 && !segments.isEmpty()) {
                segments << QStringLiteral("tỷ");
            }
            continue;
        }
        const bool isFirstBlock = segments.isEmpty();
        QString text = readThreeDigits(block, isFirstBlock);
        const int scaleIndex = (i % 3 == 0 && i > 0) ? 3 : (i % 3);
        if (scaleIndex > 0) {
            text += QStringLiteral(" ") + scales[scaleIndex];
        }
        segments << text;
    }

    QString result = segments.join(' ').simplified();
    if (!result.isEmpty()) {
        result[0] = result.at(0).toUpper();
    }
    if (negative) {
        result.prepend(QStringLiteral("Âm "));
    }
    result += QStringLiteral(" đồng chẵn");
    return result;
}

QDate parseInvoiceDate(const Appointment* appt) {
    if (appt != nullptr) {
        const QString raw = QString::fromStdString(appt->getAppointmentDate()).trimmed();
        const QDate parsed = QDate::fromString(raw, QStringLiteral("dd/MM/yyyy"));
        if (parsed.isValid()) {
            return parsed;
        }
    }
    return QDate::currentDate();
}

// Khung hóa đơn nền trắng có vẽ watermark PHÒNG KHÁM ĐA KHOA LONG PHÁT và dòng bản quyền dọc mép phải.
class InvoicePaperWidget : public QWidget {
public:
    explicit InvoicePaperWidget(QWidget* parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_StyledBackground, false);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        // Nền giấy trắng + viền ngoài mảnh
        painter.fillRect(rect(), Qt::white);
        painter.setPen(QPen(QColor(QStringLiteral("#cbd5e1")), 1));
        painter.drawRect(rect().adjusted(0, 0, -1, -1));

        // === VẼ WATERMARK CHÌM Ở GIỮA TỜ HÓA ĐƠN ===
        painter.save();
        const QPointF center(width() * 0.49, height() * 0.49);

        // Các vạch ngang đỏ nhạt + đường tròn cách điệu giống mẫu phòng khám
        const QColor stripeRose(218, 155, 160, 62);
        const qreal emblemX = center.x() - 185.0;
        const qreal emblemY = center.y() - 75.0;
        for (int i = 0; i < 12; ++i) {
            const qreal y = emblemY + i * 13.0;
            const qreal w = (i < 3 || i > 9) ? 105.0 : 122.0;
            painter.fillRect(QRectF(emblemX, y, w, 6.0), stripeRose);
        }
        // Dải kẻ ngang phụ sau cụm chữ
        for (int i = 0; i < 3; ++i) {
            painter.fillRect(QRectF(center.x() - 95.0, center.y() - 96.0 + i * 10.0, 145.0, 5.0), stripeRose);
            painter.fillRect(QRectF(center.x() - 95.0, center.y() + 82.0 + i * 10.0, 145.0, 5.0), stripeRose);
        }

        // Chữ chìm PHÒNG KHÁM LONG PHÁT
        const QColor watermarkBlue(95, 138, 182, 54);
        painter.setPen(watermarkBlue);

        QFont subFont(QStringLiteral("Times New Roman"), 24, QFont::Bold);
        painter.setFont(subFont);
        painter.drawText(QRectF(center.x() - 150, center.y() - 62, 360, 40),
                         Qt::AlignCenter, QStringLiteral("PHÒNG KHÁM"));

        QFont mainFont(QStringLiteral("Times New Roman"), 34, QFont::Bold);
        painter.setFont(mainFont);
        painter.drawText(QRectF(center.x() - 165, center.y() - 22, 390, 56),
                         Qt::AlignCenter, QStringLiteral("LONG PHÁT"));

        QFont yearFont(QStringLiteral("Times New Roman"), 15, QFont::Bold);
        painter.setFont(yearFont);
        painter.drawText(QRectF(center.x() - 40, center.y() + 36, 130, 26),
                         Qt::AlignCenter, QStringLiteral("2026"));

        painter.fillRect(QRectF(center.x() - 60, center.y() + 44, 48, 3.0), stripeRose);
        painter.fillRect(QRectF(center.x() + 62, center.y() + 44, 48, 3.0), stripeRose);
        painter.restore();

        // === CHỮ DỌC MÉP PHẢI ===
        painter.save();
        painter.setPen(QColor(QStringLiteral("#334155")));
        QFont sideFont(QStringLiteral("Times New Roman"), 8);
        painter.setFont(sideFont);
        painter.translate(width() - 8, height() - 20);
        painter.rotate(-90.0);
        painter.drawText(0, 0, QStringLiteral("Giải pháp: Hệ thống Quản lý Phòng khám Long Phát - PBL2"));
        painter.restore();
    }
};

struct InvoiceLineItem {
    QString description;
    QString category;
    int quantity;
    long long unitPrice;
    long long lineTotal;
};

QWidget* makeBoxedField(const QString& text, const QString& extraStyle = QString()) {
    auto* lbl = new QLabel(text);
    lbl->setStyleSheet(
        QStringLiteral("border: 1px solid #334155; padding: 2px 8px; "
                       "background: rgba(255,255,255,0.75); color: #0f172a; ")
        + extraStyle);
    lbl->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    return lbl;
}

QFrame* makeDottedSpacer() {
    auto* line = new QFrame;
    line->setFrameShape(QFrame::NoFrame);
    line->setFixedHeight(14);
    line->setStyleSheet(QStringLiteral("background: transparent; border-bottom: 1px dotted #475569;"));
    line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    return line;
}

} // namespace

std::string InvoiceDialog::normalizeRecordId(const std::string& rawInput) {
    QString text = QString::fromStdString(rawInput).trimmed();
    if (text.isEmpty()) return "";

    // Hỗ trợ trường hợp QCompleter trả về chuỗi "BA00001 - Tên BN (...)"
    const int spacePos = text.indexOf(' ');
    const int dashPos = text.indexOf('-');
    int cutPos = -1;
    if (spacePos > 0 && dashPos > 0) {
        cutPos = qMin(spacePos, dashPos);
    } else if (spacePos > 0) {
        cutPos = spacePos;
    } else if (dashPos > 0) {
        cutPos = dashPos;
    }
    if (cutPos > 0) {
        text = text.left(cutPos).trimmed();
    }
    return text.toUpper().toStdString();
}

const Invoice* InvoiceDialog::findInvoiceForRecord(
    Database& db,
    const std::string& recordId,
    const MedicalRecord** outRecord,
    const Appointment** outAppt) {
    if (outRecord) *outRecord = nullptr;
    if (outAppt) *outAppt = nullptr;

    const std::string cleanId = normalizeRecordId(recordId);
    if (cleanId.empty()) return nullptr;

    MedicalRecord* record = db.findMedicalRecordById(cleanId);
    if (record == nullptr) return nullptr;
    if (outRecord) *outRecord = record;

    Appointment* appt = db.findAppointmentById(record->getAppointmentId());
    if (outAppt) *outAppt = appt;

    const auto& invoices = db.getInvoices();
    // Ưu tiên khớp chính xác theo appointmentId
    for (auto it = invoices.rbegin(); it != invoices.rend(); ++it) {
        if (!record->getAppointmentId().empty() &&
            it->getAppointmentId() == record->getAppointmentId()) {
            return &(*it);
        }
    }

    // Tương thích dữ liệu mẫu cũ khi hóa đơn có cột appointmentId rỗng nhưng lượt khám đã Billed
    if (appt != nullptr && appt->getStatus() == "Billed") {
        for (auto it = invoices.rbegin(); it != invoices.rend(); ++it) {
            if (it->getPatientId() == appt->getPatientId() &&
                it->getAppointmentId().empty()) {
                return &(*it);
            }
        }
    }

    return nullptr;
}

InvoiceDialog::InvoiceDialog(Database& database, const std::string& recordId, QWidget* parent)
    : QDialog(parent),
      db(database),
      m_recordId(normalizeRecordId(recordId)) {
    setObjectName(QStringLiteral("InvoiceDialog"));
    setWindowTitle(QStringLiteral("Hóa đơn viện phí - PHÒNG KHÁM ĐA KHOA LONG PHÁT (") + QString::fromStdString(m_recordId) + QStringLiteral(")"));
    setModal(true);
    resize(980, 720);
    setMinimumSize(860, 620);

    // Cô lập QSS khỏi ClinicStyle của ReceptionistMenu để giữ đúng màu trắng hóa đơn & nút thao tác
    setStyleSheet(QStringLiteral(
        "QDialog#InvoiceDialog { background: #eef2f6; color: #0f172a; }"
        "QScrollArea { background: transparent; border: none; }"
        "QLabel { background: transparent; color: #0f172a; }"
    ));

    const MedicalRecord* record = nullptr;
    const Appointment* appt = nullptr;
    const Invoice* invoice = findInvoiceForRecord(db, m_recordId, &record, &appt);
    const Patient* patient = (appt != nullptr)
        ? db.findPatientById(appt->getPatientId())
        : (invoice != nullptr ? db.findPatientById(invoice->getPatientId()) : nullptr);
    const Doctor* doctor = (appt != nullptr)
        ? db.findDoctorById(appt->getDoctorId())
        : nullptr;

    if (invoice != nullptr) {
        m_invoiceId = invoice->getId();
    }

    buildUi(invoice, record, appt, patient, doctor);
}

void InvoiceDialog::buildUi(
    const Invoice* invoice,
    const MedicalRecord* record,
    const Appointment* appt,
    const Patient* patient,
    const Doctor* doctor) {
    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(14, 14, 14, 12);
    rootLayout->setSpacing(10);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    auto* scrollContainer = new QWidget(scrollArea);
    auto* containerLayout = new QVBoxLayout(scrollContainer);
    containerLayout->setContentsMargins(4, 4, 4, 4);
    containerLayout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    m_invoiceSheet = new InvoicePaperWidget(scrollContainer);
    m_invoiceSheet->setMinimumWidth(890);
    m_invoiceSheet->setMaximumWidth(940);
    m_invoiceSheet->setStyleSheet(QStringLiteral(
        "font-family: 'Times New Roman', 'DejaVu Serif', serif; "
        "font-size: 10.5pt; color: #0f172a; background: transparent;"
    ));

    auto* sheetLayout = new QVBoxLayout(m_invoiceSheet);
    sheetLayout->setContentsMargins(24, 18, 28, 16);
    sheetLayout->setSpacing(8);

    const QDate invDate = parseInvoiceDate(appt);

    // === 1. PHẦN HEADER (3 CỘT) ===
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    // Trái: Thông tin cơ sở khám chữa bệnh
    auto* leftCol = new QVBoxLayout();
    leftCol->setSpacing(2);
    auto* lblHospital = new QLabel(QStringLiteral("PHÒNG KHÁM ĐA KHOA LONG PHÁT"));
    lblHospital->setStyleSheet(QStringLiteral("font-size: 12pt; font-weight: bold; color: #16365c;"));
    auto* lblAddress = new QLabel(QStringLiteral("54 Nguyễn Lương Bằng, Quận Liên Chiểu, TP. Đà Nẵng"));
    lblAddress->setWordWrap(true);
    auto* lblPhone = new QLabel(QStringLiteral("Điện thoại: <b>(0236) 3842308</b>"));
    leftCol->addWidget(lblHospital);
    leftCol->addWidget(lblAddress);
    leftCol->addWidget(lblPhone);
    leftCol->addStretch();

    // Giữa: Tiêu đề Hóa đơn & Ngày tháng năm
    auto* centerCol = new QVBoxLayout();
    centerCol->setSpacing(3);
    centerCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_lblTitle = new QLabel(QStringLiteral("HÓA ĐƠN VIỆN PHÍ"));
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setStyleSheet(QStringLiteral("font-size: 16pt; font-weight: bold; color: #16365c;"));

    auto* lblDate = new QLabel(
        QStringLiteral("<i>Ngày</i> <b>%1</b> &nbsp; <i>Tháng</i> <b>%2</b> &nbsp; <i>Năm</i> <b>%3</b>")
            .arg(invDate.day(), 2, 10, QChar('0'))
            .arg(invDate.month(), 2, 10, QChar('0'))
            .arg(invDate.year()));
    lblDate->setAlignment(Qt::AlignCenter);
    lblDate->setStyleSheet(QStringLiteral("font-size: 10.5pt; color: #0f172a;"));

    centerCol->addWidget(m_lblTitle);
    centerCol->addWidget(lblDate);
    centerCol->addStretch();

    // Phải: Số hóa đơn & Mã bệnh án
    auto* rightCol = new QVBoxLayout();
    rightCol->setSpacing(4);
    rightCol->setAlignment(Qt::AlignTop | Qt::AlignRight);
    auto* lblRecordCode = new QLabel(
        QStringLiteral("Mã bệnh án: <b>%1</b>").arg(QString::fromStdString(m_recordId)));

    auto* invNumRow = new QHBoxLayout();
    invNumRow->setSpacing(4);
    invNumRow->setContentsMargins(0, 0, 0, 0);
    auto* lblSo = new QLabel(QStringLiteral("Số hóa đơn:"));
    const QString invoiceIdText = invoice ? QString::fromStdString(invoice->getId()) : QStringLiteral("N/A");
    auto* lblInvBox = makeBoxedField(
        invoiceIdText,
        QStringLiteral("font-weight: bold; color: #b91c1c; min-width: 76px; qproperty-alignment: AlignCenter;"));
    invNumRow->addWidget(lblSo);
    invNumRow->addWidget(lblInvBox);

    rightCol->addLayout(invNumRow);
    rightCol->addWidget(lblRecordCode, 0, Qt::AlignRight);
    rightCol->addStretch();

    headerLayout->addLayout(leftCol, 40);
    headerLayout->addLayout(centerCol, 38);
    headerLayout->addLayout(rightCol, 22);
    sheetLayout->addLayout(headerLayout);

    // Đường kẻ ngang phân cách header
    auto* topRule = new QFrame();
    topRule->setFrameShape(QFrame::HLine);
    topRule->setFixedHeight(1);
    topRule->setStyleSheet(QStringLiteral("background: #334155; border: none;"));
    sheetLayout->addWidget(topRule);

    // === 2. THÔNG TIN NGƯỜI MUA / BỆNH NHÂN ===
    const QString patientName = patient
        ? QString::fromStdString(patient->getFullName() + " (" + patient->getId() + ")")
        : QString::fromStdString(invoice ? invoice->getPatientId() : "Khách lẻ");
    const QString patientType = patient
        ? (patient->getType() == "NoiTru" ? QStringLiteral("Nội trú") : QStringLiteral("Ngoại trú"))
        : QStringLiteral("Ngoại trú");
    const QString specialty = (appt && !appt->getSpecialty().empty())
        ? QString::fromStdString(appt->getSpecialty())
        : QStringLiteral("Khám đa khoa");
    const QString doctorName = doctor
        ? QString::fromStdString(doctor->getFullName())
        : (appt ? QString::fromStdString(appt->getDoctorId()) : QStringLiteral("—"));
    const QString diagnosis = (record && !record->getDiagnosis().empty())
        ? QString::fromStdString(record->getDiagnosis())
        : QStringLiteral("Hoàn tất khám bệnh");
    const QString bhytText = (patient && patient->hasBHYT())
        ? QStringLiteral("BHYT: %1").arg(QString::fromStdString(patient->getBHYT()))
        : QStringLiteral("Không BHYT (Viện phí dịch vụ)");
    const QString phoneText = patient && !patient->getPhoneNumber().empty()
        ? QString::fromStdString(patient->getPhoneNumber())
        : QStringLiteral("—");

    auto* infoLayout = new QVBoxLayout();
    infoLayout->setSpacing(4);

    // Dòng 1: Họ tên người mua hàng
    auto* rowBuyer = new QHBoxLayout();
    rowBuyer->setSpacing(6);
    rowBuyer->addWidget(new QLabel(QStringLiteral("Họ tên người mua hàng:")));
    rowBuyer->addWidget(makeBoxedField(patientName, QStringLiteral("font-weight: bold; min-width: 240px;")));
    rowBuyer->addWidget(makeDottedSpacer(), 1);
    infoLayout->addLayout(rowBuyer);

    // Dòng 2: Đơn vị / Khoa khám & Bác sĩ
    auto* rowUnit = new QHBoxLayout();
    rowUnit->setSpacing(6);
    rowUnit->addWidget(new QLabel(QStringLiteral("Đơn vị :")));
    auto* lblUnitVal = new QLabel(
        QStringLiteral("%1 — %2 — Bác sĩ: %3").arg(patientType, specialty, doctorName));
    lblUnitVal->setStyleSheet(QStringLiteral("border-bottom: 1px dotted #475569; padding-bottom: 1px;"));
    rowUnit->addWidget(lblUnitVal, 1);
    infoLayout->addLayout(rowUnit);

    // Dòng 3: Địa chỉ / Chẩn đoán theo Bệnh án
    auto* rowDiag = new QHBoxLayout();
    rowDiag->setSpacing(6);
    rowDiag->addWidget(new QLabel(QStringLiteral("Địa chỉ / Bệnh án :")));
    rowDiag->addWidget(makeBoxedField(
        QStringLiteral("Mã BA: %1  |  Chẩn đoán: %2").arg(QString::fromStdString(m_recordId), diagnosis),
        QStringLiteral("min-width: 340px;")));
    rowDiag->addWidget(makeDottedSpacer(), 1);
    infoLayout->addLayout(rowDiag);

    // Dòng 4: Thẻ BHYT & Điện thoại
    auto* rowTaxPhone = new QHBoxLayout();
    rowTaxPhone->setSpacing(6);
    rowTaxPhone->addWidget(new QLabel(QStringLiteral("Thẻ BHYT :")));
    auto* lblBhytVal = new QLabel(bhytText);
    lblBhytVal->setStyleSheet(QStringLiteral("font-weight: bold; color: #1e293b;"));
    rowTaxPhone->addWidget(lblBhytVal);
    rowTaxPhone->addWidget(makeDottedSpacer(), 1);
    rowTaxPhone->addWidget(new QLabel(QStringLiteral("Điện thoại:")));
    rowTaxPhone->addWidget(makeBoxedField(phoneText, QStringLiteral("min-width: 110px;")));
    infoLayout->addLayout(rowTaxPhone);

    // Dòng 5: Hình thức thanh toán
    auto* rowPayment = new QHBoxLayout();
    rowPayment->setSpacing(6);
    rowPayment->addWidget(new QLabel(QStringLiteral("Hình thức thanh toán : <b>TM</b>")));
    rowPayment->addStretch();
    infoLayout->addLayout(rowPayment);

    sheetLayout->addLayout(infoLayout);

    // === 3. GOM DỮ LIỆU CHI TIẾT DỊCH VỤ & THUỐC ĐÃ CẤP PHÁT ===
    std::vector<InvoiceLineItem> items;
    long long runningTotal = 0;

    if (invoice != nullptr) {
        for (const InvoiceDetail& detail : invoice->getDetails()) {
            const Service* srv = db.getServices().search(detail.getServiceId());
            QString itemName = (srv != nullptr)
                ? QString::fromStdString(srv->getServiceName())
                : QStringLiteral("Dịch vụ y tế (%1)").arg(QString::fromStdString(detail.getServiceId()));
            const long long uPrice = std::llround(detail.getUnitPrice());
            const long long sub = std::llround(detail.getSubTotal());
            runningTotal += sub;
            items.push_back({
                itemName,
                QStringLiteral("Dịch vụ"),
                detail.getQuantity(),
                uPrice,
                sub
            });
        }
    }

    const bool hasBhyt = (patient != nullptr && patient->hasBHYT());
    if (record != nullptr) {
        for (const PrescriptionDetail& rx : record->getPrescriptions()) {
            if (!rx.dispensed) continue;
            const Medicine* med = db.findMedicineById(rx.medicineId);
            QString medTitle = med
                ? QString::fromStdString(med->getMedicineName())
                : QString::fromStdString(rx.medicineId);
            if (!rx.dosage.empty()) {
                medTitle += QStringLiteral(" (") + QString::fromStdString(rx.dosage) + QStringLiteral(")");
            }

            const double coPayFactor = Database::calculateMedicineCoPayFactor(rx.medicineId, hasBhyt);
            const double effectiveUnitPrice = rx.unitPrice * coPayFactor;
            if (hasBhyt && coPayFactor < 1.0) {
                const int coveredPercent = static_cast<int>(std::llround((1.0 - coPayFactor) * 100.0));
                medTitle += QStringLiteral(" [BHYT %1%]").arg(coveredPercent);
            }

            const long long uPrice = std::llround(effectiveUnitPrice);
            const long long lineTotal = std::llround(effectiveUnitPrice * rx.quantity);
            runningTotal += lineTotal;
            items.push_back({
                medTitle,
                QStringLiteral("Thuốc"),
                rx.quantity,
                uPrice,
                lineTotal
            });
        }
    }

    const long long storedTotal = invoice ? std::llround(invoice->getTotalAmount()) : runningTotal;
    const long long displayTotal = invoice ? storedTotal : runningTotal;
    const bool hasTotalMismatch = (invoice != nullptr && runningTotal != storedTotal);

    // === 4. BẢNG KÊ CHI TIẾT (NỀN TRONG SUỐT HIỂN THỊ WATERMARK) ===
    auto* tableFrame = new QWidget(m_invoiceSheet);
    tableFrame->setStyleSheet(QStringLiteral("background: transparent;"));
    auto* grid = new QGridLayout(tableFrame);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    // Tỉ lệ độ rộng 6 cột trung thực theo dữ liệu lưu trữ
    const int colStretches[6] = {6, 46, 12, 10, 13, 13};
    for (int c = 0; c < 6; ++c) {
        grid->setColumnStretch(c, colStretches[c]);
    }

    auto makeCell = [](const QString& text, Qt::Alignment align, const QString& borderStyle, const QString& extraStyle = QString()) {
        auto* lbl = new QLabel(text);
        lbl->setAlignment(align);
        lbl->setWordWrap(true);
        lbl->setMargin(5);
        lbl->setStyleSheet(QStringLiteral("background: transparent; color: #0f172a; ") + borderStyle + extraStyle);
        return lbl;
    };

    const QString headers[6] = {
        QStringLiteral("STT"),
        QStringLiteral("Nội dung (Dịch vụ / Thuốc)"),
        QStringLiteral("Phân loại"),
        QStringLiteral("Số lượng"),
        QStringLiteral("Đơn giá (VNĐ)"),
        QStringLiteral("Thành tiền (VNĐ)")
    };

    // Hàng tiêu đề bảng
    for (int c = 0; c < 6; ++c) {
        QString border = QStringLiteral("border-top: 1px solid #1e293b; border-bottom: 1px solid #1e293b; border-right: 1px solid #1e293b;");
        if (c == 0) border += QStringLiteral("border-left: 1px solid #1e293b;");
        grid->addWidget(makeCell(headers[c], Qt::AlignCenter, border, QStringLiteral("font-weight: bold;")), 0, c);
    }

    // Các hàng dữ liệu
    int currentRow = 1;
    for (size_t i = 0; i < items.size(); ++i) {
        const auto& item = items[i];
        const QString rowBottom = (i + 1 < items.size())
            ? QStringLiteral("border-bottom: 1px dashed #cbd5e1;")
            : QStringLiteral("");

        const QString vals[6] = {
            QString::number(i + 1),
            item.description,
            item.category,
            QString::number(item.quantity),
            formatVnd(item.unitPrice),
            formatVnd(item.lineTotal)
        };
        const Qt::Alignment aligns[6] = {
            Qt::AlignCenter | Qt::AlignVCenter,
            Qt::AlignLeft | Qt::AlignVCenter,
            Qt::AlignCenter | Qt::AlignVCenter,
            Qt::AlignRight | Qt::AlignVCenter,
            Qt::AlignRight | Qt::AlignVCenter,
            Qt::AlignRight | Qt::AlignVCenter
        };

        for (int c = 0; c < 6; ++c) {
            QString border = QStringLiteral("border-right: 1px solid #1e293b;") + rowBottom;
            if (c == 0) border += QStringLiteral("border-left: 1px solid #1e293b;");
            grid->addWidget(makeCell(vals[c], aligns[c], border), currentRow, c);
        }
        ++currentRow;
    }

    // Khoảng trống giữ chiều cao giấy hóa đơn
    const int fillerHeight = qMax(36, 136 - static_cast<int>(items.size()) * 26);
    for (int c = 0; c < 6; ++c) {
        QString border = QStringLiteral("border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;");
        if (c == 0) border += QStringLiteral("border-left: 1px solid #1e293b;");
        auto* spacerCell = makeCell(QString(), Qt::AlignCenter, border);
        spacerCell->setMinimumHeight(fillerHeight);
        grid->addWidget(spacerCell, currentRow, c);
    }
    ++currentRow;

    // Dòng cộng tiền hàng hóa, dịch vụ
    grid->addWidget(
        makeCell(
            QStringLiteral("Tổng tiền thanh toán (VNĐ) :"),
            Qt::AlignRight | Qt::AlignVCenter,
            QStringLiteral("border-left: 1px solid #1e293b; border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;")),
        currentRow, 0, 1, 5);
    grid->addWidget(
        makeCell(
            formatVnd(displayTotal),
            Qt::AlignRight | Qt::AlignVCenter,
            QStringLiteral("border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;"),
            QStringLiteral("font-weight: bold; font-size: 11pt;")),
        currentRow, 5, 1, 1);
    ++currentRow;

    // Dòng số tiền viết bằng chữ
    const QString moneyWords = readVietnameseMoney(displayTotal);
    auto* lblWords = makeCell(
        QStringLiteral("Số tiền viết bằng chữ : <b><u>%1</u></b>").arg(moneyWords),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("border-left: 1px solid #1e293b; border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;"));
    grid->addWidget(lblWords, currentRow, 0, 1, 6);

    sheetLayout->addWidget(tableFrame);

    if (hasTotalMismatch) {
        auto* lblMismatch = new QLabel(
            QStringLiteral("Cảnh báo đối soát: Tổng các dòng chi tiết (%1 VNĐ) chênh lệch với tổng hóa đơn đã lưu (%2 VNĐ).")
                .arg(formatVnd(runningTotal), formatVnd(storedTotal)));
        lblMismatch->setWordWrap(true);
        lblMismatch->setStyleSheet(QStringLiteral("color: #b91c1c; font-weight: bold; background: #fef2f2; border: 1px solid #fca5a5; padding: 6px;"));
        sheetLayout->addWidget(lblMismatch);
    }

    // === 5. PHẦN XÁC NHẬN KÝ TÊN ===
    auto* sigLayout = new QHBoxLayout();
    sigLayout->setContentsMargins(10, 10, 10, 18);

    auto* buyerSigCol = new QVBoxLayout();
    buyerSigCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    auto* lblBuyerSig = new QLabel(QStringLiteral("<b>Bệnh nhân / Người thanh toán</b>"));
    lblBuyerSig->setAlignment(Qt::AlignCenter);
    auto* lblBuyerNote = new QLabel(QStringLiteral("<i>(Ký, ghi rõ họ tên)</i>"));
    lblBuyerNote->setAlignment(Qt::AlignCenter);
    buyerSigCol->addWidget(lblBuyerSig);
    buyerSigCol->addWidget(lblBuyerNote);
    buyerSigCol->addSpacing(34);

    auto* sellerSigCol = new QVBoxLayout();
    sellerSigCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    auto* lblSellerSig = new QLabel(QStringLiteral("<b>Nhân viên thu ngân</b>"));
    lblSellerSig->setAlignment(Qt::AlignCenter);
    auto* lblSellerNote = new QLabel(QStringLiteral("<i>(Ký, ghi rõ họ tên)</i>"));
    lblSellerNote->setAlignment(Qt::AlignCenter);
    sellerSigCol->addWidget(lblSellerSig);
    sellerSigCol->addWidget(lblSellerNote);
    sellerSigCol->addSpacing(34);

    sigLayout->addLayout(buyerSigCol, 1);
    sigLayout->addLayout(sellerSigCol, 1);
    sheetLayout->addLayout(sigLayout);

    // === 6. CHÂN TRANG THAM CHIẾU NỘI BỘ ===
    auto* lookupLayout = new QHBoxLayout();
    lookupLayout->setSpacing(6);
    auto* lblPortal = new QLabel(
        QStringLiteral("Phiếu thanh toán viện phí nội bộ — Hệ thống Quản lý Phòng khám (PBL2)"));
    lblPortal->setStyleSheet(QStringLiteral("color: #475569; font-style: italic;"));
    auto* lblLookupCode = new QLabel(QStringLiteral("Mã bệnh án:"));
    auto* lblLookupBox = makeBoxedField(
        QString::fromStdString(m_recordId),
        QStringLiteral("min-width: 120px; font-weight: bold;"));

    lookupLayout->addWidget(lblPortal);
    lookupLayout->addStretch();
    lookupLayout->addWidget(lblLookupCode);
    lookupLayout->addWidget(lblLookupBox);
    sheetLayout->addLayout(lookupLayout);

    containerLayout->addWidget(m_invoiceSheet);
    containerLayout->addStretch();
    scrollArea->setWidget(scrollContainer);
    rootLayout->addWidget(scrollArea, 1);

    // === 7. THANH NÚT THAO TÁC DƯỚI CÙNG ===
    auto* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(10);

    m_btnPrint = new QPushButton(QStringLiteral("In hóa đơn"), this);
    m_btnPrint->setCursor(Qt::PointingHandCursor);
    m_btnPrint->setStyleSheet(QStringLiteral(
        "QPushButton { background: #38b2ce; color: #ffffff; border: 1px solid #2b9eb8; "
        "border-radius: 5px; padding: 8px 16px; font-family: 'Segoe UI'; font-weight: 600; }"
        "QPushButton:hover { background: #2b9eb8; }"
    ));

    m_btnExportPdf = new QPushButton(QStringLiteral("Tải PDF & đính kèm"), this);
    m_btnExportPdf->setCursor(Qt::PointingHandCursor);
    m_btnExportPdf->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f0ad4e; color: #ffffff; border: 1px solid #eea236; "
        "border-radius: 5px; padding: 8px 16px; font-family: 'Segoe UI'; font-weight: 600; }"
        "QPushButton:hover { background: #ec971f; }"
    ));

    m_btnClose = new QPushButton(QStringLiteral("× Close"), this);
    m_btnClose->setCursor(Qt::PointingHandCursor);
    m_btnClose->setStyleSheet(QStringLiteral(
        "QPushButton { background: #f8fafc; color: #64748b; border: 1px solid #cbd5e1; "
        "border-radius: 5px; padding: 8px 16px; font-family: 'Segoe UI'; font-size: 10.5pt; font-weight: 600; }"
        "QPushButton:hover { background: #e2e8f0; color: #334155; }"
    ));

    actionLayout->addWidget(m_btnPrint);
    actionLayout->addWidget(m_btnExportPdf);
    actionLayout->addStretch();
    actionLayout->addWidget(m_btnClose);
    rootLayout->addLayout(actionLayout);

    connect(m_btnPrint, &QPushButton::clicked, this, &InvoiceDialog::onPrintInvoice);
    connect(m_btnExportPdf, &QPushButton::clicked, this, &InvoiceDialog::onExportPdf);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

bool InvoiceDialog::exportSheetToPdf(const QString& filePath) {
    if (m_invoiceSheet == nullptr || filePath.isEmpty()) return false;

    m_invoiceSheet->ensurePolished();
    m_invoiceSheet->adjustSize();

    QPdfWriter pdfWriter(filePath);
    pdfWriter.setTitle(QStringLiteral("Hóa đơn viện phí - PHÒNG KHÁM ĐA KHOA LONG PHÁT (%1)")
                           .arg(QString::fromStdString(m_recordId)));
    pdfWriter.setCreator(QStringLiteral("PHÒNG KHÁM ĐA KHOA LONG PHÁT - PBL2"));
    pdfWriter.setResolution(300);
    pdfWriter.setPageLayout(QPageLayout(
        QPageSize(QPageSize::A4),
        QPageLayout::Landscape,
        QMarginsF(10.0, 10.0, 10.0, 10.0),
        QPageLayout::Millimeter));

    QPainter painter(&pdfWriter);
    if (!painter.isActive()) {
        return false;
    }

    const QRectF paintRect = pdfWriter.pageLayout().paintRectPixels(pdfWriter.resolution());
    const QSize sheetSize = m_invoiceSheet->size();
    if (sheetSize.width() <= 0 || sheetSize.height() <= 0) {
        return false;
    }

    const double scaleX = paintRect.width() / static_cast<double>(sheetSize.width());
    const double scaleY = paintRect.height() / static_cast<double>(sheetSize.height());
    const double scale = qMin(scaleX, scaleY);
    const double offsetX = (paintRect.width() - sheetSize.width() * scale) / 2.0;
    const double offsetY = (paintRect.height() - sheetSize.height() * scale) / 2.0;

    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);
    m_invoiceSheet->render(&painter);
    painter.end();
    return true;
}

void InvoiceDialog::onPrintInvoice() {
    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    const QString safeDir = tempDir.isEmpty() ? QDir::tempPath() : tempDir;
    const QString tempPdfPath = QDir(safeDir).filePath(
        QStringLiteral("HoaDon_BVLongPhat_%1.pdf").arg(QString::fromStdString(m_recordId)));

    if (!exportSheetToPdf(tempPdfPath)) {
        QMessageBox::critical(this, QStringLiteral("Lỗi in hóa đơn"),
                              QStringLiteral("Không thể khởi tạo bản in PDF cho hóa đơn này."));
        return;
    }

    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(tempPdfPath))) {
        QMessageBox::information(
            this,
            QStringLiteral("Bản in sẵn sàng"),
            QStringLiteral("Đã tạo bản in hóa đơn tại:\n%1").arg(tempPdfPath));
    }
}

void InvoiceDialog::onExportPdf() {
    const QString defaultName = QStringLiteral("HoaDon_BVLongPhat_%1_%2.pdf")
        .arg(QString::fromStdString(m_recordId))
        .arg(QString::fromStdString(m_invoiceId.empty() ? "INV" : m_invoiceId));

    const QString savePath = QFileDialog::getSaveFileName(
        this,
        QStringLiteral("Lưu hóa đơn viện phí PDF"),
        QDir::homePath() + QDir::separator() + defaultName,
        QStringLiteral("PDF Files (*.pdf)"));

    if (savePath.isEmpty()) return;

    if (exportSheetToPdf(savePath)) {
        QMessageBox::information(
            this,
            QStringLiteral("Xuất PDF thành công"),
            QStringLiteral("Đã lưu hóa đơn ra tệp:\n%1").arg(savePath));
    } else {
        QMessageBox::critical(
            this,
            QStringLiteral("Lỗi xuất PDF"),
            QStringLiteral("Không thể ghi tệp PDF. Vui lòng kiểm tra quyền truy cập hoặc đóng tệp đang mở."));
    }
}
