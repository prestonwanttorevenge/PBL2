#include "ui/AdminMenu.h"
#include "ui_AdminMenu.h"
#include "ui/Login.h"
#include "ui/ClinicStyle.h"
#include "utils/Constants.h"
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QLinearGradient>
#include <QFontMetrics>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QLocale>
#include <QDate>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using std::string;
using std::vector;

class RevenueChart : public QWidget {
public:
    static constexpr size_t kMaxRecentInvoices = 8;

    explicit RevenueChart(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(330, 280);
    }

    void setData(const vector<double>& recentValues,
                 const vector<QString>& recentLabels,
                 const vector<double>& allValues) {
        values_ = recentValues;
        labels_ = recentLabels;
        allValues_ = allValues;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#ffffff"));

        const QRect content = rect().adjusted(12, 8, -12, -8);
        if (allValues_.empty() || content.width() <= 0 || content.height() <= 0) {
            painter.setPen(QColor("#64748b"));
            painter.drawText(rect(), Qt::AlignCenter,
                             QString::fromUtf8("Chưa có hóa đơn để thống kê"));
            return;
        }

        const QLocale vietnamese(QLocale::Vietnamese, QLocale::Vietnam);
        const int topHeight = std::clamp(static_cast<int>(content.height() * 0.44),
                                         96, std::max(96, content.height() - 120));
        const QRect topRect(content.left(), content.top(), content.width(), topHeight - 6);
        const int dividerY = content.top() + topHeight;
        const QRect bottomRect(content.left(), dividerY + 6, content.width(),
                               std::max(0, content.bottom() - (dividerY + 6)));

        drawDonutSection(painter, topRect, vietnamese);

        painter.setPen(QPen(QColor("#e2e8f0"), 1));
        painter.drawLine(content.left(), dividerY, content.right(), dividerY);

        drawTrendSection(painter, bottomRect, vietnamese);
    }

private:
    static constexpr double kTierLowThreshold = 500000.0;
    static constexpr double kTierHighThreshold = 1000000.0;

    static QString formatShortVnd(double value, const QLocale& locale) {
        const long long roundedK = std::llround(value / 1000.0);
        if (value >= kTierHighThreshold || roundedK >= 1000) {
            return locale.toString(value / 1000000.0, 'f', 1) + QString::fromUtf8(" tr");
        }
        if (value >= 1000.0) {
            return locale.toString(static_cast<double>(roundedK), 'f', 0) + QStringLiteral(" k");
        }
        return locale.toString(value, 'f', 0);
    }

    void drawDonutSection(QPainter& painter, const QRect& rect, const QLocale& locale) const {
        if (rect.height() < 40) return;

        painter.setFont(QFont(QStringLiteral("Segoe UI"), 9, QFont::DemiBold));
        painter.setPen(QColor("#1e293b"));
        painter.drawText(QRect(rect.left(), rect.top(), rect.width(), 20),
                         Qt::AlignLeft | Qt::AlignVCenter,
                         QString::fromUtf8("Cơ cấu hóa đơn theo mức thu"));

        const QRect area = rect.adjusted(0, 24, 0, 0);
        if (area.width() <= 0 || area.height() <= 0) return;

        int counts[3] = {0, 0, 0};
        double totalSum = 0.0;
        for (double val : allValues_) {
            totalSum += val;
            if (val < kTierLowThreshold) {
                ++counts[0];
            } else if (val <= kTierHighThreshold) {
                ++counts[1];
            } else {
                ++counts[2];
            }
        }
        const int totalCount = static_cast<int>(allValues_.size());
        const double avgAll = totalCount > 0 ? totalSum / totalCount : 0.0;

        const QColor colors[3] = {
            QColor("#3b82f6"),
            QColor("#10b981"),
            QColor("#f59e0b")
        };
        const QString tierLabels[3] = {
            QString::fromUtf8("Dưới 500k"),
            QString::fromUtf8("500k - 1 triệu"),
            QString::fromUtf8("Trên 1 triệu")
        };

        const int ringSize = std::clamp(
            std::min(area.height() - 6, static_cast<int>(area.width() * 0.38)),
            76, 164);
        const int penWidth = std::clamp(ringSize / 7, 10, 20);
        const int ringLeft = area.left() + 6;
        const int ringTop = area.top() + (area.height() - ringSize) / 2;
        const QRectF arcRect(ringLeft + penWidth / 2.0, ringTop + penWidth / 2.0,
                             ringSize - penWidth, ringSize - penWidth);

        painter.setPen(QPen(QColor("#f1f5f9"), penWidth, Qt::SolidLine, Qt::FlatCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawArc(arcRect, 0, 360 * 16);

        int lastNonZero = -1;
        for (int i = 0; i < 3; ++i) {
            if (counts[i] > 0) lastNonZero = i;
        }
        int startAngle16 = 90 * 16;
        int remainingSpan16 = 360 * 16;
        for (int i = 0; i < 3; ++i) {
            if (counts[i] <= 0) continue;
            const int span16 = (i == lastNonZero)
                ? remainingSpan16
                : static_cast<int>(std::llround(360.0 * 16.0 * counts[i] / totalCount));
            painter.setPen(QPen(colors[i], penWidth, Qt::SolidLine, Qt::FlatCap));
            painter.drawArc(arcRect, startAngle16, -span16);
            startAngle16 -= span16;
            remainingSpan16 -= span16;
        }

        const int innerWidth = std::max(10, ringSize - 2 * penWidth - 4);
        const QRectF centerRect(ringLeft + penWidth + 2, ringTop + penWidth + 2,
                                innerWidth, innerWidth);
        const int countFontPt = ringSize >= 120 ? 11 : 8;
        const int avgFontPt = (innerWidth < 68 || ringSize < 120) ? 7 : 8;
        const QString avgCenterText = (innerWidth < 68)
            ? formatShortVnd(avgAll, locale)
            : (QString::fromUtf8("TB: ") + formatShortVnd(avgAll, locale));

        painter.setFont(QFont(QStringLiteral("Segoe UI"), countFontPt, QFont::Bold));
        painter.setPen(QColor("#0f172a"));
        painter.drawText(QRectF(centerRect.left(), centerRect.top(),
                                centerRect.width(), centerRect.height() * 0.52),
                         Qt::AlignHCenter | Qt::AlignBottom,
                         QString::number(totalCount) + QString::fromUtf8(" HĐ"));

        painter.setFont(QFont(QStringLiteral("Segoe UI"), avgFontPt));
        painter.setPen(QColor("#64748b"));
        painter.drawText(QRectF(centerRect.left(), centerRect.top() + centerRect.height() * 0.52,
                                centerRect.width(), centerRect.height() * 0.48),
                         Qt::AlignHCenter | Qt::AlignTop,
                         avgCenterText);

        const int legendGap = area.width() < 340 ? 12 : 20;
        const int legendLeft = ringLeft + ringSize + legendGap;
        const int legendWidth = area.right() - legendLeft - 4;
        if (legendWidth < 80) return;

        const int rowHeight = std::clamp(area.height() / 3, 20, 36);
        const int legendStartY = area.top() + (area.height() - rowHeight * 3) / 2;
        const int legendFontPt = legendWidth < 210 ? 8 : 9;
        const int metricWidth = std::min(legendWidth < 210 ? 82 : 108, legendWidth / 2);

        for (int i = 0; i < 3; ++i) {
            const QRect rowRect(legendLeft, legendStartY + i * rowHeight, legendWidth, rowHeight);
            const QRect badgeRect(rowRect.left(), rowRect.center().y() - 5, 10, 10);
            painter.setPen(Qt::NoPen);
            painter.setBrush(colors[i]);
            painter.drawRoundedRect(badgeRect, 3, 3);

            const int labelLeft = rowRect.left() + 16;
            const int labelWidth = std::max(20, rowRect.width() - 16 - metricWidth - 4);
            painter.setFont(QFont(QStringLiteral("Segoe UI"), legendFontPt, QFont::Medium));
            painter.setPen(QColor("#334155"));
            painter.drawText(QRect(labelLeft, rowRect.top(), labelWidth, rowRect.height()),
                             Qt::AlignLeft | Qt::AlignVCenter, tierLabels[i]);

            const double pct = totalCount > 0 ? (100.0 * counts[i] / totalCount) : 0.0;
            const QString metric = QString::number(counts[i]) + QString::fromUtf8(" HĐ (")
                + QString::number(pct, 'f', 0) + QStringLiteral("%)");
            painter.setFont(QFont(QStringLiteral("Segoe UI"), legendFontPt, QFont::DemiBold));
            painter.setPen(QColor("#0f172a"));
            painter.drawText(QRect(rowRect.right() - metricWidth, rowRect.top(),
                                   metricWidth, rowRect.height()),
                             Qt::AlignRight | Qt::AlignVCenter, metric);
        }
    }

    void drawTrendSection(QPainter& painter, const QRect& rect, const QLocale& locale) const {
        if (rect.height() < 60 || values_.empty()) return;

        double maximum = 0.0;
        double sumRecent = 0.0;
        for (double value : values_) {
            sumRecent += value;
            if (value > maximum) maximum = value;
        }
        if (maximum <= 0.0) maximum = 1.0;
        maximum *= 1.22;
        const double avgRecent = sumRecent / static_cast<double>(values_.size());

        const QRect headerRect(rect.left(), rect.top(), rect.width(), 22);
        const QFont headerFont(QStringLiteral("Segoe UI"), rect.width() < 340 ? 8 : 9, QFont::DemiBold);
        const QString trendTitle = rect.width() < 370
            ? QString::fromUtf8("Xu hướng %1 HĐ gần nhất (VNĐ)").arg(kMaxRecentInvoices)
            : QString::fromUtf8("Xu hướng %1 hóa đơn gần nhất (VNĐ)").arg(kMaxRecentInvoices);
        painter.setFont(headerFont);
        painter.setPen(QColor("#1e293b"));
        painter.drawText(headerRect, Qt::AlignLeft | Qt::AlignVCenter, trendTitle);

        const QString badgeText = QString::fromUtf8("TB gần nhất: ") + formatShortVnd(avgRecent, locale);
        const QFont badgeFont(QStringLiteral("Segoe UI"), rect.width() < 340 ? 7 : 8, QFont::DemiBold);
        painter.setFont(badgeFont);
        const int titleWidth = QFontMetrics(headerFont).horizontalAdvance(trendTitle);
        const int badgeTextWidth = QFontMetrics(badgeFont).horizontalAdvance(badgeText);
        const int sampleEndX = headerRect.right() - badgeTextWidth - 6;
        const int sampleStartX = headerRect.right() - badgeTextWidth - 24;
        if (sampleStartX >= headerRect.left() + titleWidth + 8) {
            painter.setPen(QPen(QColor("#f59e0b"), 1.8, Qt::DashLine));
            painter.drawLine(sampleStartX, headerRect.center().y(),
                             sampleEndX, headerRect.center().y());
        }
        painter.setPen(QColor("#b45309"));
        painter.drawText(headerRect, Qt::AlignRight | Qt::AlignVCenter, badgeText);

        const QRect plot = rect.adjusted(54, 30, -14, -24);
        if (plot.width() <= 28 || plot.height() <= 24) return;

        painter.setFont(QFont(QStringLiteral("Segoe UI"), 8));
        for (int line = 0; line <= 3; ++line) {
            const int y = plot.bottom() - (plot.height() * line / 3);
            const double amount = maximum * line / 3.0;
            painter.setPen(QColor("#e6edf5"));
            painter.drawLine(plot.left(), y, plot.right(), y);
            painter.setPen(QColor("#64748b"));
            painter.drawText(QRect(rect.left(), y - 8, plot.left() - rect.left() - 6, 16),
                             Qt::AlignRight | Qt::AlignVCenter, formatShortVnd(amount, locale));
        }

        const double avgY = plot.bottom() - (avgRecent / maximum) * plot.height();
        painter.setPen(QPen(QColor("#f59e0b"), 1.3, Qt::DashLine));
        painter.drawLine(QPointF(plot.left(), avgY), QPointF(plot.right(), avgY));

        vector<QPointF> points;
        points.reserve(values_.size());
        const double leftX = plot.left() + 14.0;
        const double rightX = plot.right() - 14.0;
        for (size_t i = 0; i < values_.size(); ++i) {
            const double px = (values_.size() == 1)
                ? plot.center().x()
                : leftX + (rightX - leftX) * static_cast<double>(i) / static_cast<double>(values_.size() - 1);
            const double py = plot.bottom() - (values_[i] / maximum) * plot.height();
            points.emplace_back(px, py);
        }

        if (points.size() >= 2) {
            QPainterPath areaPath;
            areaPath.moveTo(points.front().x(), plot.bottom());
            for (const QPointF& pt : points) {
                areaPath.lineTo(pt);
            }
            areaPath.lineTo(points.back().x(), plot.bottom());
            areaPath.closeSubpath();

            QLinearGradient gradient(0, plot.top(), 0, plot.bottom());
            gradient.setColorAt(0.0, QColor(16, 185, 129, 75));
            gradient.setColorAt(1.0, QColor(16, 185, 129, 8));
            painter.fillPath(areaPath, gradient);

            QPainterPath linePath;
            linePath.moveTo(points.front());
            for (size_t i = 1; i < points.size(); ++i) {
                linePath.lineTo(points[i]);
            }
            painter.setPen(QPen(QColor("#059669"), 2.4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(linePath);
        }

        const double slotWidth = points.size() > 1
            ? (rightX - leftX) / static_cast<double>(points.size() - 1)
            : static_cast<double>(plot.width());
        const double halfBox = std::max(22.0, slotWidth * 0.5);
        const int pointFontPt = plot.width() < 280 ? 7 : 8;
        const QFont valFont(QStringLiteral("Segoe UI"), pointFontPt, QFont::DemiBold);
        const QFontMetrics valFm(valFont);
        QRectF prevValueRect;

        for (size_t i = 0; i < points.size(); ++i) {
            painter.setPen(QPen(QColor("#ffffff"), 2.0));
            painter.setBrush(QColor("#059669"));
            painter.drawEllipse(points[i], 4.5, 4.5);

            const QString valText = formatShortVnd(values_[i], locale);
            const double textW = static_cast<double>(valFm.horizontalAdvance(valText)) + 6.0;
            const double textH = 14.0;
            QRectF valRect(points[i].x() - textW / 2.0,
                           std::max<double>(plot.top() + 1.0, points[i].y() - 20.0),
                           textW, textH);
            if (prevValueRect.isValid() && valRect.adjusted(-2.0, 0.0, 2.0, 0.0).intersects(prevValueRect)) {
                const QRectF belowRect(points[i].x() - textW / 2.0, points[i].y() + 6.0, textW, textH);
                if (belowRect.bottom() <= plot.bottom() - 2.0
                    && !belowRect.adjusted(-2.0, 0.0, 2.0, 0.0).intersects(prevValueRect)) {
                    valRect = belowRect;
                }
            }

            painter.setFont(valFont);
            painter.setPen(QColor("#0f172a"));
            painter.drawText(valRect, Qt::AlignHCenter | Qt::AlignVCenter, valText);
            prevValueRect = valRect;

            QString xLabel = i < labels_.size() ? labels_[i] : QString::number(i + 1);
            if (slotWidth < 42.0 && xLabel.startsWith(QStringLiteral("INV"), Qt::CaseInsensitive)) {
                xLabel = QStringLiteral("#") + xLabel.mid(3);
            }
            painter.setFont(QFont(QStringLiteral("Segoe UI"), pointFontPt));
            painter.setPen(QColor("#64748b"));
            painter.drawText(QRectF(points[i].x() - halfBox, plot.bottom() + 3.0, halfBox * 2.0, 18.0),
                             Qt::AlignHCenter | Qt::AlignTop, xLabel);
        }
    }

    vector<double> values_;
    vector<QString> labels_;
    vector<double> allValues_;
};

AdminMenu::AdminMenu(Database& database, AuthService& authService, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AdminMenu), db(database), auth(authService)
{
    ui->setupUi(this);
    applyClinicStyle(this);
    setMinimumSize(960, 640);
    setAttribute(Qt::WA_DeleteOnClose);
    revenueChart = new RevenueChart(ui->revenueChartHost);
    QVBoxLayout* chartLayout = new QVBoxLayout(ui->revenueChartHost);
    chartLayout->setContentsMargins(0, 0, 0, 0);
    chartLayout->addWidget(revenueChart);
    ui->revenueSplitter->setOrientation(Qt::Horizontal);
    ui->revenueSplitter->setChildrenCollapsible(false);
    ui->revenueSplitter->setStretchFactor(0, 5);
    ui->revenueSplitter->setStretchFactor(1, 6);
    ui->revenueSplitter->setSizes({520, 620});

    // --- KIỂM SOÁT ĐẦU VÀO BẰNG REGEX ---
    QRegularExpression rxPhone("^[0-9]{10,11}$");
    ui->txtPhone->setValidator(new QRegularExpressionValidator(rxPhone, this));
    ui->txtDob->setPlaceholderText("dd/MM/yyyy");

    QRegularExpression rxPrice("^[0-9]*\\.?[0-9]+$");
    ui->txtMedPrice->setValidator(new QRegularExpressionValidator(rxPrice, this));
    ui->txtServicePrice->setValidator(new QRegularExpressionValidator(rxPrice, this));
    const QRegularExpression usernamePattern("^[A-Za-z0-9._-]{3,32}$");
    ui->txtUsername->setMaxLength(32);
    ui->txtDocUsername->setMaxLength(32);
    ui->txtUsername->setValidator(new QRegularExpressionValidator(usernamePattern, this));
    ui->txtDocUsername->setValidator(new QRegularExpressionValidator(usernamePattern, this));
    ui->txtUsername->setPlaceholderText(QString::fromUtf8("Tài khoản không dấu"));
    ui->txtDocUsername->setPlaceholderText(QString::fromUtf8("Tài khoản không dấu"));

    QStringList specialties;
    for (const string& specialty : Constants::medicalSpecialties()) {
        specialties.append(QString::fromUtf8(specialty.c_str()));
    }
    ui->cbDocSpecialty->addItems(specialties);
    ui->cbDocWorkDay->addItems({
        QString::fromUtf8("Thứ 2"), QString::fromUtf8("Thứ 3"),
        QString::fromUtf8("Thứ 4"), QString::fromUtf8("Thứ 5"),
        QString::fromUtf8("Thứ 6"), QString::fromUtf8("Thứ 7"),
        QString::fromUtf8("Chủ nhật")
    });
    ui->cbDocWorkShift->addItems({
        QString::fromUtf8("Sáng"), QString::fromUtf8("Chiều"), QString::fromUtf8("Tối")
    });

    // --- KẾT NỐI SỰ KIỆN CLICK-TO-FILL ---
    connect(ui->tableUsers, &QTableWidget::itemClicked, this, &AdminMenu::on_tableUsers_itemClicked);
    connect(ui->tableDoctors, &QTableWidget::itemClicked, this, &AdminMenu::on_tableDoctors_itemClicked);
    connect(ui->tableMedicines, &QTableWidget::itemClicked, this, &AdminMenu::on_tableMedicines_itemClicked);
    connect(ui->tableServices, &QTableWidget::itemClicked, this, &AdminMenu::on_tableServices_itemClicked);

    refreshUserTable();
    refreshDoctorTable();
    refreshMedicineTable();
    refreshServiceTable();
    
    ui->stackedWidget->setCurrentIndex(0);
}

AdminMenu::~AdminMenu() {
    delete ui;
}

// ==========================================
// CÁC HÀM CLICK-TO-FILL (DÙNG CHO VIỆC XÓA)
// ==========================================
void AdminMenu::on_tableUsers_itemClicked(QTableWidgetItem *item) {
    if (!item) return;
    int row = item->row();
    QTableWidgetItem *idItem = (row >= 0) ? ui->tableUsers->item(row, 0) : nullptr;
    if (idItem) ui->txtUserId->setText(idItem->text());
}

void AdminMenu::on_tableDoctors_itemClicked(QTableWidgetItem *item) {
    if (!item) return;
    int row = item->row();
    QTableWidgetItem *idItem = (row >= 0) ? ui->tableDoctors->item(row, 0) : nullptr;
    if (idItem) ui->txtDocId->setText(idItem->text());
}

void AdminMenu::on_tableMedicines_itemClicked(QTableWidgetItem *item) {
    if (!item) return;
    int row = item->row();
    QTableWidgetItem *idItem = (row >= 0) ? ui->tableMedicines->item(row, 0) : nullptr;
    if (idItem) ui->txtMedId->setText(idItem->text());
}

void AdminMenu::on_tableServices_itemClicked(QTableWidgetItem *item) {
    if (!item) return;
    int row = item->row();
    QTableWidgetItem *idItem = (row >= 0) ? ui->tableServices->item(row, 0) : nullptr;
    if (idItem) ui->txtServiceId->setText(idItem->text());
}


// ==========================================
// 1. QUẢN LÝ TÀI KHOẢN (TRANG SỐ 0)
// ==========================================

void AdminMenu::on_btnManageUsers_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
    refreshUserTable();
}

void AdminMenu::on_btnAddUser_clicked() {
    string name = ui->txtUserFullName->text().trimmed().toStdString();
    string username = ui->txtUsername->text().trimmed().toStdString();
    string password = ui->txtPassword->text().toStdString();
    string role = ui->cbUserRole->currentText().trimmed().toStdString();

    if (name.empty() || username.empty() || ui->txtPassword->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin tài khoản!");
        return;
    }
    
    // NÂNG CẤP: Chặn Admin tạo acc BS tay ngang
    if (role == "Doctor") {
        QMessageBox::warning(this, "Cảnh báo", "Tài khoản Bác sĩ sẽ được tự động cấp phát khi bạn thêm Bác sĩ mới bên thẻ [Bác Sĩ]!");
        return;
    }

    // NÂNG CẤP: Tự động cấp ID theo Role
    string generatedId = db.generateUserId(role);
    int result = db.addUserFromQT(generatedId, username, password, name, role);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Tên đăng nhập (Username) đã tồn tại!");
    } else if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã tạo tài khoản thành công với Mã: " + QString::fromStdString(generatedId));
        refreshUserTable();
        ui->txtUserId->clear();
        ui->txtUserFullName->clear();
        ui->txtUsername->clear();
        ui->txtPassword->clear();
    } else {
        QMessageBox::critical(this, "Không thể tạo tài khoản",
                              "Thông tin chưa hợp lệ, mã tài khoản đã tồn tại hoặc tên đăng nhập phải dùng 3–32 ký tự Latin không dấu.");
    }
}

// ==========================================
// 2. QUẢN LÝ BÁC SĨ (TRANG SỐ 1)
// ==========================================

void AdminMenu::on_btnManageDoctors_clicked() {
    ui->stackedWidget->setCurrentIndex(1);
    refreshDoctorTable();
}

void AdminMenu::on_btnAddDoctor_clicked() {
    string name = ui->txtDocName->text().trimmed().toStdString();
    string dob = ui->txtDob->text().trimmed().toStdString();
    string gender = ui->cbGender->currentText().trimmed().toStdString();
    string phone = ui->txtPhone->text().trimmed().toStdString();
    string spec = ui->cbDocSpecialty->currentText().trimmed().toStdString(); 
    int expYears = ui->spinExpYears->value();

    string username = ui->txtDocUsername->text().trimmed().toStdString();
    string password = ui->txtDocPassword->text().toStdString();

    if (name.empty() || dob.empty() || phone.empty() || spec.empty() || username.empty() ||
        ui->txtDocPassword->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng điền đầy đủ tất cả các trường thông tin Bác sĩ!");
        return;
    }

    const QDate parsedDob = QDate::fromString(QString::fromStdString(dob), "dd/MM/yyyy");
    if (!parsedDob.isValid() || parsedDob > QDate::currentDate()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Ngày sinh không hợp lệ (định dạng dd/MM/yyyy và không vượt quá ngày hiện tại)!");
        return;
    }

    if (!ui->txtPhone->hasAcceptableInput()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Số điện thoại phải gồm 10 đến 11 chữ số!");
        return;
    }

    // NÂNG CẤP: Tự động cấp mã BS
    string generatedId = db.generateDoctorId();
    int result = db.addDoctorWithAccount(generatedId, name, dob, gender, phone, spec, expYears,
                                         username, password,
                                         ui->cbDocWorkDay->currentText().toStdString(),
                                         ui->cbDocWorkShift->currentText().toStdString());

    if (result == -2) {
        QMessageBox::critical(this, "Lỗi", "Tên đăng nhập (Username) đã bị trùng!");
    }
    else if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã thêm Bác sĩ thành công!\nMã BS cấp phát: " + QString::fromStdString(generatedId));

        refreshDoctorTable();
        refreshUserTable();
        ui->txtDocId->clear();
        ui->txtDocName->clear();
        ui->txtDob->clear();
        ui->cbGender->setCurrentIndex(0);
        ui->txtPhone->clear();
        ui->cbDocSpecialty->setCurrentIndex(0);
        ui->cbDocWorkDay->setCurrentIndex(0);
        ui->cbDocWorkShift->setCurrentIndex(0);
        ui->spinExpYears->setValue(0);
        ui->txtDocUsername->clear();
        ui->txtDocPassword->clear();
    } else {
        QMessageBox::critical(this, "Không thể thêm bác sĩ",
                              "Thông tin bác sĩ hoặc lịch làm việc chưa hợp lệ. Tên đăng nhập phải gồm 3–32 ký tự Latin không dấu.");
    }
}

// ==========================================
// 3. QUẢN LÝ THUỐC (TRANG SỐ 2)
// ==========================================

void AdminMenu::on_btnManageMedicines_clicked() {
    ui->stackedWidget->setCurrentIndex(2);
    refreshMedicineTable();
}

void AdminMenu::on_btnAddMedicine_clicked() {
    // Rút 2 ký tự đầu tiên từ ComboBox (VD: "B1", "B2")
    string prefix = ui->cbMedPrefix->currentText().left(2).trimmed().toStdString();
    
    string name = ui->txtMedName->text().trimmed().toStdString();
    string dosage = ui->txtDosage->text().trimmed().toStdString();
    string priceStr = ui->txtMedPrice->text().trimmed().toStdString();
    int qty = ui->spinMedQty->value();

    if (name.empty() || dosage.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin thuốc!");
        return;
    }

    double price = 0.0;
    try { price = std::stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Lỗi", "Đơn giá thuốc không hợp lệ!");
        return;
    }

    // NÂNG CẤP: Dùng chuẩn 6 tham số của hệ thống mới
    string finalId = ""; // Hứng ID do Backend cấp phát trả về
    int result = db.addMedicineFromQT(prefix, name, price, dosage, qty, finalId);

    if (result == 1) {
        db.saveAllData();
        QMessageBox::information(this, "Thành công", "Đã khởi tạo THUỐC MỚI vào kho!\nMã cấp: " + QString::fromStdString(finalId));
    } else if (result == 2) {
        db.saveAllData();
        QMessageBox::information(this, "Thành công", "Thuốc đã tồn tại (" + QString::fromStdString(finalId) + "). Đã CỘNG DỒN vào kho!");
    } else if (result == -3) {
        QMessageBox::warning(this, "Không thể nhập kho",
                             "Thuốc cùng tên đã tồn tại trong một nhóm thuốc khác.");
        return;
    } else {
        QMessageBox::warning(this, "Thông tin không hợp lệ",
                             "Kiểm tra lại giá và số lượng nhập kho (phải lớn hơn 0).");
        return;
    }

    refreshMedicineTable();
    ui->txtMedId->clear();
    ui->txtMedName->clear();
    ui->txtDosage->clear();
    ui->txtMedPrice->clear();
    ui->spinMedQty->setValue(1);
}

// ==========================================
// 4. QUẢN LÝ DỊCH VỤ Y TẾ (TRANG SỐ 3)
// ==========================================

void AdminMenu::on_btnManageServices_clicked() {
    ui->stackedWidget->setCurrentIndex(3);
    refreshServiceTable();
}

void AdminMenu::on_btnAddService_clicked() {
    string name = ui->txtServiceName->text().trimmed().toStdString();
    string priceStr = ui->txtServicePrice->text().trimmed().toStdString();
    
    // Rút 2 ký tự đầu tiên từ ComboBox (Ví dụ: "S1", "S2")
    string prefix = ui->cbServicePrefix->currentText().left(2).trimmed().toStdString();

    if (name.empty() || priceStr.empty()) {
        QMessageBox::warning(this, "Lỗi nhập liệu", "Vui lòng nhập đầy đủ thông tin dịch vụ!");
        return;
    }

    double price = 0.0;
    try { price = std::stod(priceStr); } catch (...) {
        QMessageBox::warning(this, "Lỗi", "Phí dịch vụ không hợp lệ!");
        return;
    }
    string finalId = ""; // Biến chứa ID trả về
    
    int result = db.addServiceFromQT(prefix, name, price, finalId);

    if (result == -1) {
        QMessageBox::critical(this, "Lỗi", "Dịch vụ này đã tồn tại trong hệ thống!\nMã dịch vụ trùng: " + QString::fromStdString(finalId));
    } else if (result == 1) {
        db.saveAllData(); 
        QMessageBox::information(this, "Thành công", "Đã phân loại và thêm Dịch vụ y tế mới!\nMã tự cấp phát: " + QString::fromStdString(finalId));
        
        refreshServiceTable();
        ui->txtServiceName->clear();
        ui->txtServicePrice->clear();
    } else {
        QMessageBox::warning(this, "Thông tin không hợp lệ",
                             "Kiểm tra lại tên dịch vụ và giá dịch vụ (phải lớn hơn 0).");
    }
}
// ==========================================
// CÁC HÀM XÓA (GIỮ NGUYÊN)
// ==========================================
void AdminMenu::on_btnDeleteUser_clicked() {
    string id = ui->txtUserId->text().trimmed().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một tài khoản trên bảng để xóa!");
        return;
    }

    if (id == "U01" || id == "ADMIN" || id == "ADMIN01") {
        QMessageBox::critical(this, "Cảnh báo", "Không thể xóa tài khoản Quản trị viên gốc!");
        return;
    }

    if (auth.getCurrentUser() != nullptr && auth.getCurrentUser()->getId() == id) {
        QMessageBox::warning(this, "Không thể xóa phiên hiện tại",
                             "Không thể xóa tài khoản đang đăng nhập!");
        return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận xóa",
                                       "Xóa tài khoản có mã [" + QString::fromStdString(id) + "]?",
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (db.deleteUserFromQT(id)) {
            db.saveAllData(); refreshUserTable(); ui->txtUserId->clear();
        } else QMessageBox::critical(this, "Không thể xóa",
                                     "Tài khoản không tồn tại, thuộc bác sĩ (hãy xóa qua hồ sơ bác sĩ), hoặc là quản trị viên cuối cùng đang hoạt động.");
    }
}

void AdminMenu::on_btnToggleUserStatus_clicked() {
    const string id = ui->txtUserId->text().toStdString();
    User* user = db.getUsers().search(id);
    if (user == nullptr) {
        QMessageBox::warning(this, "Chưa chọn", "Chọn một tài khoản trong danh sách.");
        return;
    }
    if (auth.getCurrentUser() != nullptr && auth.getCurrentUser()->getId() == id) {
        QMessageBox::warning(this, "Không thể khóa phiên hiện tại",
                             "Không thể khóa tài khoản đang đăng nhập. Hãy dùng tài khoản quản trị khác.");
        return;
    }

    const bool newActiveState = !user->getIsActive();
    if (!db.setUserActiveFromQT(id, newActiveState)) {
        QMessageBox::warning(this, "Không thể đổi trạng thái",
                             "Không thể khóa quản trị viên đang hoạt động cuối cùng.");
        return;
    }
    db.saveAllData();
    refreshUserTable();
}

void AdminMenu::on_btnDeleteDoctor_clicked() {
    string id = ui->txtDocId->text().trimmed().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn một Bác sĩ trên bảng để xóa!");
        return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận", "Xóa Bác sĩ [" + QString::fromStdString(id) + "] sẽ xóa luôn tài khoản liên kết?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int result = db.deleteDoctorFromQT(id);
        if (result == 1) {
            db.saveAllData(); refreshDoctorTable(); refreshUserTable(); ui->txtDocId->clear();
        } else if (result == -2) {
            QMessageBox::critical(this, "Lỗi Ràng Buộc", "Không thể xóa bác sĩ đã được gắn với lịch hẹn hoặc hồ sơ khám.");
        } else {
            QMessageBox::warning(this, "Không thể xóa", "Không tìm thấy bác sĩ có mã [" + QString::fromStdString(id) + "]!");
        }
    }
}

void AdminMenu::on_btnDeleteMedicine_clicked() {
    string id = ui->txtMedId->text().trimmed().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng click chọn thuốc trên bảng để xóa!"); return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận", "Xóa thuốc [" + QString::fromStdString(id) + "] khỏi kho?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        int result = db.deleteMedicineFromQT(id);
        if (result == 1) {
            db.saveAllData(); refreshMedicineTable(); ui->txtMedId->clear();
        } else if (result == -2) {
            QMessageBox::critical(this, "Lỗi", "Thuốc đã được kê trong Bệnh án, không thể xóa!");
        } else {
            QMessageBox::warning(this, "Không thể xóa", "Không tìm thấy thuốc có mã [" + QString::fromStdString(id) + "]!");
        }
    }
}

void AdminMenu::on_btnDeleteService_clicked() {
    string id = ui->txtServiceId->text().trimmed().toStdString();
    if (id.empty()) {
        QMessageBox::warning(this, "Chưa chọn", "Vui lòng chọn dịch vụ trên bảng!"); return;
    }
    auto reply = QMessageBox::question(this, "Xác nhận", "Xóa dịch vụ [" + QString::fromStdString(id) + "]?", QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        if (db.deleteServiceFromQT(id)) {
            db.saveAllData(); refreshServiceTable(); ui->txtServiceId->clear();
        } else {
            QMessageBox::warning(this, "Không thể xóa",
                                 "Không tìm thấy dịch vụ có mã [" + QString::fromStdString(id) + "] hoặc dịch vụ đang được sử dụng!");
        }
    }
}

// ==========================================
// 5. ĐĂNG XUẤT HỆ THỐNG
// ==========================================

void AdminMenu::on_btnLogout_clicked() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Đăng xuất", "Bạn có chắc chắn muốn thoát quyền Admin?",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        auth.logout();
        Login *loginForm = new Login(this->db, this->auth);
        loginForm->show();
        this->close();
    }
}
//làm thêm 1 giao diện xem hóa đơn
void AdminMenu::on_btnViewRevenue_clicked() {
    ui->stackedWidget->setCurrentIndex(4); 
    
    // ==========================================
    // 1. TÍNH VÀ HIỂN THỊ DOANH THU
    // ==========================================
    double totalRev = db.calculateTotalRevenue();
    QString formattedRevenue = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(totalRev, 'f', 0);
    ui->lblTotalRevenue->setText(formattedRevenue + " VNĐ");
    
    // ==========================================
    // 2. ĐẾM VÀ HIỂN THỊ SỐ BỆNH NHÂN (THÊM MỚI Ở ĐÂY)
    // ==========================================
    // Đếm tổng số hồ sơ bệnh nhân đang có trong hệ thống (Cây BST)
    int totalPatients = db.getPatients().getAllPointers().size();
    ui->lblTotalPatients->setText(QString::number(totalPatients) + " người");
    
    vector<double> allRevenueValues;
    const vector<Invoice>& invoices = db.getInvoices();
    allRevenueValues.reserve(invoices.size());
    for (const auto& inv : invoices) {
        allRevenueValues.push_back(inv.getTotalAmount());
    }

    vector<double> revenueValues;
    vector<QString> invoiceLabels;
    const size_t firstVisible = invoices.size() > RevenueChart::kMaxRecentInvoices
        ? invoices.size() - RevenueChart::kMaxRecentInvoices
        : 0;
    for (size_t index = firstVisible; index < invoices.size(); ++index) {
        revenueValues.push_back(invoices[index].getTotalAmount());
        invoiceLabels.push_back(QString::fromStdString(invoices[index].getId()));
    }
    revenueChart->setData(revenueValues, invoiceLabels, allRevenueValues);
    const QList<int> sizes = ui->revenueSplitter->sizes();
    if (sizes.size() == 2 && (sizes[0] < 200 || sizes[1] < 200)) {
        ui->revenueSplitter->setSizes({520, 620});
    }

    // ==========================================
    // 3. LÀM MỚI BẢNG (Nếu có)
    // ==========================================
    refreshRevenueTable(); 
}
void AdminMenu::refreshRevenueTable() {
    ui->tableRevenue->setColumnCount(4);
    ui->tableRevenue->setHorizontalHeaderLabels({"Mã Hóa Đơn", "Mã Bệnh Nhân", "Họ và Tên Bệnh Nhân", "Tổng Thu (VNĐ)"});
    ui->tableRevenue->verticalHeader()->setVisible(false);
    ui->tableRevenue->horizontalHeader()->setMinimumSectionSize(75);
    ui->tableRevenue->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableRevenue->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    
    ui->tableRevenue->setRowCount(0);
    vector<Invoice> invList = db.getInvoices();
    
    int row = 0;
    for (const auto& inv : invList) {
        ui->tableRevenue->insertRow(row);
        ui->tableRevenue->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(inv.getId())));
        ui->tableRevenue->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(inv.getPatientId())));

        Patient* pat = db.findPatientById(inv.getPatientId());
        QString patientName = (pat != nullptr)
            ? QString::fromStdString(pat->getFullName())
            : QString::fromUtf8("Khách vãng lai / Đã xóa");
        ui->tableRevenue->setItem(row, 2, new QTableWidgetItem(patientName));
        
        QString feeStr = QLocale(QLocale::Vietnamese, QLocale::Vietnam).toString(inv.getTotalAmount(), 'f', 0);
        QTableWidgetItem* feeItem = new QTableWidgetItem(feeStr);
        feeItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableRevenue->setItem(row, 3, feeItem);
        row++;
    }
}

// ==========================================
// 6. CÁC HÀM LÀM MỚI BẢNG (REFRESH TABLES)
// ==========================================

void AdminMenu::refreshUserTable() {
    ui->tableUsers->setColumnCount(5);
    ui->tableUsers->setHorizontalHeaderLabels({"User ID", "Username", "Họ và Tên", "Vai trò", "Trạng thái"});
    ui->tableUsers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableUsers->setRowCount(0);
    vector<User*> usersList = db.getUsers().getAllPointers();

    int row = 0;
    for (auto u : usersList) {
        ui->tableUsers->insertRow(row);
        ui->tableUsers->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(u->getId())));
        ui->tableUsers->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(u->getUsername())));
        ui->tableUsers->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(u->getFullName())));
        ui->tableUsers->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(u->getRole())));
        ui->tableUsers->setItem(row, 4, new QTableWidgetItem(u->getIsActive() ? "Hoạt động" : "Bị khóa"));
        row++;
    }
}

void AdminMenu::refreshDoctorTable() {
    ui->tableDoctors->setColumnCount(5);
    ui->tableDoctors->setHorizontalHeaderLabels({"Mã BS", "Họ Tên", "Chuyên Khoa", "SĐT", "Kinh Nghiệm"});
    ui->tableDoctors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableDoctors->setRowCount(0);
    vector<Doctor*> doctorsList = db.getDoctors().getAllPointers();

    int row = 0;
    for (auto doc : doctorsList) {
        ui->tableDoctors->insertRow(row);
        ui->tableDoctors->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(doc->getId())));
        ui->tableDoctors->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(doc->getFullName())));
        ui->tableDoctors->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(doc->getSpecialty())));
        ui->tableDoctors->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(doc->getPhoneNumber())));
        ui->tableDoctors->setItem(row, 4, new QTableWidgetItem(QString::number(doc->getYearsOfExperience())));
        row++;
    }
}

void AdminMenu::refreshMedicineTable() {
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

void AdminMenu::refreshServiceTable() {
    ui->tableServices->setColumnCount(3);
    ui->tableServices->setHorizontalHeaderLabels({"Mã Dịch Vụ", "Tên Dịch Vụ Y Tế", "Giá Tiền"});
    ui->tableServices->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableServices->setRowCount(0);
    vector<Service*> srvList = db.getServices().getAllPointers();

    int row = 0;
    for (auto srv : srvList) {
        ui->tableServices->insertRow(row);
        ui->tableServices->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(srv->getId())));
        ui->tableServices->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(srv->getServiceName())));
        ui->tableServices->setItem(row, 2, new QTableWidgetItem(QString::number(srv->getPrice(), 'f', 0)));
        row++;
    }
}