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

QString buildLotCode(const std::string& medId) {
    std::string cleaned;
    for (char ch : medId) {
        if (ch != '_') cleaned += static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    if (cleaned.empty()) cleaned = "001";
    return QString::fromStdString("LP-" + cleaned);
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

// Khung hóa đơn nền trắng có vẽ watermark BỆNH VIỆN LONG PHÁT và dòng bản quyền dọc mép phải.
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

        // Các vạch ngang đỏ nhạt + đường tròn cách điệu giống mẫu bệnh viện
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

        // Chữ chìm BỆNH VIỆN LONG PHÁT
        const QColor watermarkBlue(95, 138, 182, 54);
        painter.setPen(watermarkBlue);

        QFont subFont(QStringLiteral("Times New Roman"), 24, QFont::Bold);
        painter.setFont(subFont);
        painter.drawText(QRectF(center.x() - 150, center.y() - 62, 360, 40),
                         Qt::AlignCenter, QStringLiteral("BỆNH VIỆN"));

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
        painter.drawText(0, 0, QStringLiteral("Giải pháp: Hệ thống Quản lý Bệnh viện Long Phát - PBL2"));
        painter.restore();
    }
};

// Widget đóng dấu chữ ký số đỏ kèm dấu tick xanh lá giống mẫu hóa đơn điện tử.
class SignatureStampWidget : public QWidget {
public:
    explicit SignatureStampWidget(const QString& signDate, QWidget* parent = nullptr)
        : QWidget(parent), m_signDate(signDate) {
        setFixedSize(268, 78);
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::TextAntialiasing, true);

        const QRectF boxRect(1.0, 1.0, width() - 2.0, height() - 2.0);
        painter.fillRect(boxRect, QColor(255, 255, 255, 210));
        painter.setPen(QPen(QColor(QStringLiteral("#c9302c")), 1.4));
        painter.drawRect(boxRect);

        // Vẽ dấu tích xanh nghiêng đặc trưng ở góc trái hộp chữ ký
        painter.save();
        QPainterPath checkPath;
        checkPath.moveTo(14.0, 42.0);
        checkPath.lineTo(27.0, 56.0);
        checkPath.lineTo(54.0, 22.0);
        checkPath.lineTo(47.0, 18.0);
        checkPath.lineTo(26.0, 45.0);
        checkPath.lineTo(18.0, 36.0);
        checkPath.closeSubpath();
        painter.fillPath(checkPath, QColor(118, 168, 52, 225));
        painter.setPen(QPen(QColor(82, 122, 32, 230), 1.0));
        painter.drawPath(checkPath);
        painter.restore();

        // Văn bản chữ ký điện tử
        painter.setPen(QColor(QStringLiteral("#1f2937")));
        painter.setFont(QFont(QStringLiteral("Times New Roman"), 9, QFont::Bold));
        painter.drawText(QRectF(14.0, 6.0, width() - 24.0, 18.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Signature Valid"));

        painter.setPen(QColor(QStringLiteral("#c9302c")));
        painter.setFont(QFont(QStringLiteral("Times New Roman"), 9, QFont::Normal));
        painter.drawText(QRectF(14.0, 28.0, width() - 22.0, 18.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Ký bởi: BỆNH VIỆN LONG PHÁT"));
        painter.drawText(QRectF(14.0, 48.0, width() - 22.0, 18.0),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QStringLiteral("Ký ngày: ") + m_signDate);
    }

private:
    QString m_signDate;
};

struct InvoiceLineItem {
    QString description;
    QString unit;
    int quantity;
    long long unitPrice;
    long long lineTotal;
    QString expiryDate;
    QString lotNumber;
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
    setWindowTitle(QStringLiteral("Hóa đơn viện phí - BỆNH VIỆN LONG PHÁT (") + QString::fromStdString(m_recordId) + QStringLiteral(")"));
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
    const QString invDateText = invDate.toString(QStringLiteral("dd/MM/yyyy"));

    // === 1. PHẦN HEADER (3 CỘT) ===
    auto* headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(10);

    // Trái: Thông tin Bệnh viện Long Phát
    auto* leftCol = new QVBoxLayout();
    leftCol->setSpacing(2);
    auto* lblHospital = new QLabel(QStringLiteral("BỆNH VIỆN LONG PHÁT"));
    lblHospital->setStyleSheet(QStringLiteral("font-size: 13pt; font-weight: bold; color: #16365c;"));
    auto* lblMst = new QLabel(QStringLiteral("Mã số thuế: <b>0401234568</b>"));
    auto* lblAddress = new QLabel(QStringLiteral("54 Nguyễn Lương Bằng, Quận Liên Chiểu, TP. Đà Nẵng"));
    lblAddress->setWordWrap(true);
    auto* lblPhone = new QLabel(QStringLiteral("Điện thoại: <b>(0236) 3842308</b>"));
    leftCol->addWidget(lblHospital);
    leftCol->addWidget(lblMst);
    leftCol->addWidget(lblAddress);
    leftCol->addWidget(lblPhone);
    leftCol->addStretch();

    // Giữa: Tiêu đề Hóa đơn & Ngày tháng năm
    auto* centerCol = new QVBoxLayout();
    centerCol->setSpacing(3);
    centerCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_lblTitle = new QLabel(QStringLiteral("HÓA ĐƠN BÁN HÀNG"));
    m_lblTitle->setAlignment(Qt::AlignCenter);
    m_lblTitle->setStyleSheet(QStringLiteral("font-size: 16pt; font-weight: bold; color: #16365c;"));

    m_lblConversionNote = new QLabel(
        QStringLiteral("(Hóa đơn chuyển đổi từ hóa đơn điện tử - Ngày chuyển đổi: %1)")
            .arg(QDate::currentDate().toString(QStringLiteral("dd/MM/yyyy"))));
    m_lblConversionNote->setAlignment(Qt::AlignCenter);
    m_lblConversionNote->setStyleSheet(QStringLiteral("font-size: 9pt; font-style: italic; color: #334155;"));
    m_lblConversionNote->setVisible(false);

    auto* lblDate = new QLabel(
        QStringLiteral("<i>Ngày</i> <b>%1</b> &nbsp; <i>Tháng</i> <b>%2</b> &nbsp; <i>Năm</i> <b>%3</b>")
            .arg(invDate.day(), 2, 10, QChar('0'))
            .arg(invDate.month(), 2, 10, QChar('0'))
            .arg(invDate.year()));
    lblDate->setAlignment(Qt::AlignCenter);
    lblDate->setStyleSheet(QStringLiteral("font-size: 10.5pt; color: #0f172a;"));

    centerCol->addWidget(m_lblTitle);
    centerCol->addWidget(m_lblConversionNote);
    centerCol->addWidget(lblDate);
    centerCol->addStretch();

    // Phải: Mẫu số, Ký hiệu, Số hóa đơn
    auto* rightCol = new QVBoxLayout();
    rightCol->setSpacing(3);
    rightCol->setAlignment(Qt::AlignTop | Qt::AlignRight);
    auto* lblFormNo = new QLabel(QStringLiteral("Mẫu số: <b>02GTTT0/001</b>"));
    auto* lblSerial = new QLabel(QStringLiteral("Ký hiệu: <b>LP/26E</b>"));

    auto* invNumRow = new QHBoxLayout();
    invNumRow->setSpacing(4);
    invNumRow->setContentsMargins(0, 0, 0, 0);
    auto* lblSo = new QLabel(QStringLiteral("Số:"));
    const QString invoiceIdText = invoice ? QString::fromStdString(invoice->getId()) : QStringLiteral("N/A");
    auto* lblInvBox = makeBoxedField(
        invoiceIdText,
        QStringLiteral("font-weight: bold; color: #b91c1c; min-width: 76px; qproperty-alignment: AlignCenter;"));
    invNumRow->addWidget(lblSo);
    invNumRow->addWidget(lblInvBox);

    rightCol->addWidget(lblFormNo);
    rightCol->addWidget(lblSerial);
    rightCol->addLayout(invNumRow);
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

    // Dòng 4: Mã số thuế / BHYT & Điện thoại
    auto* rowTaxPhone = new QHBoxLayout();
    rowTaxPhone->setSpacing(6);
    rowTaxPhone->addWidget(new QLabel(QStringLiteral("Mã số thuế / Thẻ BHYT :")));
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
            QString itemName;
            if (srv != nullptr) {
                itemName = QString::fromStdString(srv->getServiceName());
            } else {
                itemName = QStringLiteral("Khám bệnh chuyên khoa & Dịch vụ lâm sàng");
            }
            const long long uPrice = std::llround(detail.getUnitPrice());
            const long long sub = std::llround(detail.getSubTotal());
            runningTotal += sub;
            const QString serviceCode = (detail.getServiceId().rfind("INV", 0) == 0)
                ? QStringLiteral("S1_001")
                : QString::fromStdString(detail.getServiceId());
            items.push_back({
                itemName,
                QStringLiteral("Lần"),
                detail.getQuantity(),
                uPrice,
                sub,
                QStringLiteral("—"),
                serviceCode
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

            double effectiveUnitPrice = rx.unitPrice;
            if (hasBhyt) {
                const std::string prefix = (rx.medicineId.length() >= 2) ? rx.medicineId.substr(0, 2) : "";
                if (prefix == "B1") {
                    effectiveUnitPrice = 0.0;
                    medTitle += QStringLiteral(" [BHYT 100%]");
                } else if (prefix == "B2") {
                    effectiveUnitPrice = rx.unitPrice * 0.5;
                    medTitle += QStringLiteral(" [BHYT 50%]");
                } else if (prefix == "B3") {
                    effectiveUnitPrice = rx.unitPrice * 0.2;
                    medTitle += QStringLiteral(" [BHYT 80%]");
                }
            }

            const long long uPrice = std::llround(effectiveUnitPrice);
            const long long lineTotal = std::llround(effectiveUnitPrice * rx.quantity);
            runningTotal += lineTotal;
            items.push_back({
                medTitle,
                QStringLiteral("Viên"),
                rx.quantity,
                uPrice,
                lineTotal,
                QStringLiteral("12/28"),
                buildLotCode(rx.medicineId)
            });
        }
    }

    const long long authoritativeTotal = invoice ? std::max(std::llround(invoice->getTotalAmount()), runningTotal) : runningTotal;
    const long long residual = authoritativeTotal - runningTotal;
    if (residual > 0) {
        items.push_back({
            QStringLiteral("Thuốc, vật tư y tế & chi phí điều trị theo hồ sơ"),
            QStringLiteral("Gói"),
            1,
            residual,
            residual,
            QStringLiteral("—"),
            QStringLiteral("LP-HS")
        });
    }

    // === 4. BẢNG KÊ CHI TIẾT (NỀN TRONG SUỐT HIỂN THỊ WATERMARK) ===
    auto* tableFrame = new QWidget(m_invoiceSheet);
    tableFrame->setStyleSheet(QStringLiteral("background: transparent;"));
    auto* grid = new QGridLayout(tableFrame);
    grid->setContentsMargins(0, 0, 0, 0);
    grid->setSpacing(0);

    // Tỉ lệ độ rộng các cột chuẩn theo mẫu hóa đơn
    const int colStretches[8] = {5, 36, 9, 8, 11, 13, 8, 10};
    for (int c = 0; c < 8; ++c) {
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

    const QString headers[8] = {
        QStringLiteral("STT"),
        QStringLiteral("Tên thuốc - Nội dung"),
        QStringLiteral("Đơn vị tính"),
        QStringLiteral("Số lượng"),
        QStringLiteral("Đơn giá"),
        QStringLiteral("Thành tiền"),
        QStringLiteral("Hạn SD"),
        QStringLiteral("Lô Sx")
    };

    // Hàng tiêu đề bảng
    for (int c = 0; c < 8; ++c) {
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

        const QString vals[8] = {
            QString::number(i + 1),
            item.description,
            item.unit,
            QString::number(item.quantity),
            formatVnd(item.unitPrice),
            formatVnd(item.lineTotal),
            item.expiryDate,
            item.lotNumber
        };
        const Qt::Alignment aligns[8] = {
            Qt::AlignCenter | Qt::AlignVCenter,
            Qt::AlignLeft | Qt::AlignVCenter,
            Qt::AlignCenter | Qt::AlignVCenter,
            Qt::AlignRight | Qt::AlignVCenter,
            Qt::AlignRight | Qt::AlignVCenter,
            Qt::AlignRight | Qt::AlignVCenter,
            Qt::AlignCenter | Qt::AlignVCenter,
            Qt::AlignCenter | Qt::AlignVCenter
        };

        for (int c = 0; c < 8; ++c) {
            QString border = QStringLiteral("border-right: 1px solid #1e293b;") + rowBottom;
            if (c == 0) border += QStringLiteral("border-left: 1px solid #1e293b;");
            grid->addWidget(makeCell(vals[c], aligns[c], border), currentRow, c);
        }
        ++currentRow;
    }

    // Khoảng trống giữ chiều cao giấy hóa đơn (chỉ kẻ sọc đứng giống ảnh mẫu)
    const int fillerHeight = qMax(36, 136 - static_cast<int>(items.size()) * 26);
    for (int c = 0; c < 8; ++c) {
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
            QStringLiteral("Cộng tiền hàng hóa, dịch vụ :"),
            Qt::AlignRight | Qt::AlignVCenter,
            QStringLiteral("border-left: 1px solid #1e293b; border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;")),
        currentRow, 0, 1, 5);
    grid->addWidget(
        makeCell(
            formatVnd(authoritativeTotal),
            Qt::AlignRight | Qt::AlignVCenter,
            QStringLiteral("border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;"),
            QStringLiteral("font-weight: bold; font-size: 11pt;")),
        currentRow, 5, 1, 1);
    grid->addWidget(
        makeCell(QString(), Qt::AlignCenter, QStringLiteral("border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;")),
        currentRow, 6, 1, 1);
    grid->addWidget(
        makeCell(QString(), Qt::AlignCenter, QStringLiteral("border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;")),
        currentRow, 7, 1, 1);
    ++currentRow;

    // Dòng số tiền viết bằng chữ
    const QString moneyWords = readVietnameseMoney(authoritativeTotal);
    auto* lblWords = makeCell(
        QStringLiteral("Số tiền viết bằng chữ : <b><u>%1</u></b>").arg(moneyWords),
        Qt::AlignLeft | Qt::AlignVCenter,
        QStringLiteral("border-left: 1px solid #1e293b; border-right: 1px solid #1e293b; border-bottom: 1px solid #1e293b;"));
    grid->addWidget(lblWords, currentRow, 0, 1, 8);

    sheetLayout->addWidget(tableFrame);

    // === 5. PHẦN CHỮ KÝ & DẤU ĐIỆN TỬ ===
    auto* sigLayout = new QHBoxLayout();
    sigLayout->setContentsMargins(10, 6, 10, 6);

    auto* buyerSigCol = new QVBoxLayout();
    buyerSigCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    auto* lblBuyerSig = new QLabel(QStringLiteral("Đơn vị mua hàng"));
    lblBuyerSig->setAlignment(Qt::AlignCenter);
    buyerSigCol->addWidget(lblBuyerSig);
    buyerSigCol->addStretch();

    auto* sellerSigCol = new QVBoxLayout();
    sellerSigCol->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    sellerSigCol->setSpacing(4);
    auto* lblSellerSig = new QLabel(QStringLiteral("Người bán hàng"));
    lblSellerSig->setAlignment(Qt::AlignCenter);
    sellerSigCol->addWidget(lblSellerSig);
    sellerSigCol->addWidget(new SignatureStampWidget(invDateText, m_invoiceSheet), 0, Qt::AlignCenter);

    sigLayout->addLayout(buyerSigCol, 1);
    sigLayout->addLayout(sellerSigCol, 1);
    sheetLayout->addLayout(sigLayout);

    // === 6. CHÂN TRANG TRA CỨU ===
    auto* lookupLayout = new QHBoxLayout();
    lookupLayout->setSpacing(6);
    auto* lblPortal = new QLabel(
        QStringLiteral("Trang tra cứu: <span style='color:#1d4ed8; text-decoration:underline;'>http://bvlongphat.easyinvoice.vn</span>"));
    auto* lblLookupCode = new QLabel(QStringLiteral("Mã tra cứu:"));
    auto* lblLookupBox = makeBoxedField(
        QString::fromStdString(m_recordId),
        QStringLiteral("min-width: 150px; font-weight: bold;"));

    lookupLayout->addWidget(lblPortal);
    lookupLayout->addSpacing(14);
    lookupLayout->addWidget(lblLookupCode);
    lookupLayout->addWidget(lblLookupBox);
    lookupLayout->addStretch();
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

    m_btnConvert = new QPushButton(QStringLiteral("Chuyển đổi"), this);
    m_btnConvert->setCursor(Qt::PointingHandCursor);
    m_btnConvert->setStyleSheet(QStringLiteral(
        "QPushButton { background: #2e6da4; color: #ffffff; border: 1px solid #265a88; "
        "border-radius: 5px; padding: 8px 16px; font-family: 'Segoe UI'; font-weight: 600; }"
        "QPushButton:hover { background: #245580; }"
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
    actionLayout->addWidget(m_btnConvert);
    actionLayout->addStretch();
    actionLayout->addWidget(m_btnClose);
    rootLayout->addLayout(actionLayout);

    connect(m_btnPrint, &QPushButton::clicked, this, &InvoiceDialog::onPrintInvoice);
    connect(m_btnExportPdf, &QPushButton::clicked, this, &InvoiceDialog::onExportPdf);
    connect(m_btnConvert, &QPushButton::clicked, this, &InvoiceDialog::onToggleConversion);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

bool InvoiceDialog::exportSheetToPdf(const QString& filePath) {
    if (m_invoiceSheet == nullptr || filePath.isEmpty()) return false;

    m_invoiceSheet->ensurePolished();
    m_invoiceSheet->adjustSize();

    QPdfWriter pdfWriter(filePath);
    pdfWriter.setTitle(QStringLiteral("Hóa đơn viện phí - BỆNH VIỆN LONG PHÁT (%1)")
                           .arg(QString::fromStdString(m_recordId)));
    pdfWriter.setCreator(QStringLiteral("BỆNH VIỆN LONG PHÁT - PBL2"));
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

void InvoiceDialog::onToggleConversion() {
    m_isConvertedView = !m_isConvertedView;
    if (m_lblTitle != nullptr) {
        m_lblTitle->setText(m_isConvertedView
            ? QStringLiteral("HÓA ĐƠN CHUYỂN ĐỔI")
            : QStringLiteral("HÓA ĐƠN BÁN HÀNG"));
    }
    if (m_lblConversionNote != nullptr) {
        m_lblConversionNote->setVisible(m_isConvertedView);
    }
    if (m_btnConvert != nullptr) {
        m_btnConvert->setText(m_isConvertedView
            ? QStringLiteral("Xem bản gốc")
            : QStringLiteral("Chuyển đổi"));
    }
}
