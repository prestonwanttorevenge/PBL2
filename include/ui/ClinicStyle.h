#ifndef CLINICSTYLE_H
#define CLINICSTYLE_H

#include <QApplication>
#include <QColor>
#include <QGuiApplication>
#include <QPalette>
#include <QStyleHints>
#include <QWidget>

inline void applyClinicStyle(QWidget* widget) {
    if (!widget) return;

#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
    if (QGuiApplication::styleHints()) {
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }
#endif

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(QStringLiteral("#f3f6fb")));
    lightPalette.setColor(QPalette::WindowText, QColor(QStringLiteral("#1f2937")));
    lightPalette.setColor(QPalette::Base, QColor(QStringLiteral("#ffffff")));
    lightPalette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#f7f9fc")));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#ffffff")));
    lightPalette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#1f2937")));
    lightPalette.setColor(QPalette::Text, QColor(QStringLiteral("#1f2937")));
    lightPalette.setColor(QPalette::Button, QColor(QStringLiteral("#cceee4")));
    lightPalette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#164e42")));
    lightPalette.setColor(QPalette::BrightText, QColor(QStringLiteral("#ffffff")));
    lightPalette.setColor(QPalette::Highlight, QColor(QStringLiteral("#dbeafe")));
    lightPalette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#172554")));
    lightPalette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#64748b")));

    lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(QStringLiteral("#9ca3af")));
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(QStringLiteral("#9ca3af")));
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(QStringLiteral("#9ca3af")));
    lightPalette.setColor(QPalette::Disabled, QPalette::Base, QColor(QStringLiteral("#f1f5f9")));

    QApplication::setPalette(lightPalette);
    widget->setPalette(lightPalette);

    widget->setStyleSheet(QStringLiteral(
        "QMainWindow, QDialog, QWidget#centralwidget { background: #f3f6fb; color: #1f2937; "
        "font-family: 'Segoe UI'; font-size: 10pt; }"
        "QLabel, QRadioButton, QCheckBox, QGroupBox { color: #1f2937; }"
        "QGroupBox { background: #ffffff; border: 1px solid #dce4ef; border-radius: 12px; "
        "margin-top: 12px; padding: 14px; font-weight: 600; color: #1f2937; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; color: #334155; }"
        "QLineEdit, QComboBox, QDateEdit, QSpinBox, QTextEdit, QPlainTextEdit { background: #ffffff; color: #1f2937; "
        "border: 1px solid #cbd5e1; border-radius: 7px; padding: 8px 10px; min-height: 20px; "
        "selection-background-color: #2563eb; selection-color: #ffffff; }"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus, QTextEdit:focus, QPlainTextEdit:focus { border: 1px solid #2563eb; }"
        "QLineEdit:disabled, QComboBox:disabled, QDateEdit:disabled, QSpinBox:disabled { "
        "background: #f1f5f9; color: #64748b; }"
        "QPushButton { background: #cceee4; color: #164e42; border: 1px solid #a6ddca; "
        "border-radius: 7px; padding: 9px 14px; font-weight: 600; }"
        "QPushButton:hover { background: #afe3d2; border-color: #79c9ad; }"
        "QPushButton:pressed { background: #8ed4b9; }"
        "QPushButton:disabled { background: #e5e7eb; color: #9ca3af; }"
        "QPushButton#btnLogout { background: #e9eff5; color: #334155; border-color: #d3dde8; }"
        "QPushButton#btnLogout:hover { background: #dce6ef; }"
        "QPushButton#btnDeleteUser, QPushButton#btnDeleteDoctor, QPushButton#btnDeleteMedicine, "
        "QPushButton#btnDeleteService, QPushButton#btnCancelAppointment { "
        "background: #fff0ed; color: #9f3a32; border-color: #f2c7bf; }"
        "QPushButton#btnDeleteUser:hover, QPushButton#btnDeleteDoctor:hover, "
        "QPushButton#btnDeleteMedicine:hover, QPushButton#btnDeleteService:hover, "
        "QPushButton#btnCancelAppointment:hover { background: #ffe0da; }"
        "QTableWidget, QTableView, QListView { background: #ffffff; color: #1f2937; alternate-background-color: #f7f9fc; "
        "border: 1px solid #dce4ef; border-radius: 9px; gridline-color: #edf1f6; "
        "selection-background-color: #dbeafe; selection-color: #172554; }"
        "QHeaderView::section { background: #edf2f8; color: #334155; border: 0; "
        "border-bottom: 1px solid #dce4ef; padding: 9px; font-weight: 600; }"
        "QTabWidget::pane { background: #ffffff; border: 1px solid #dce4ef; border-radius: 9px; }"
        "QTabBar::tab { background: #e3f2ed; color: #315c50; padding: 9px 14px; margin-right: 4px; "
        "border-top-left-radius: 7px; border-top-right-radius: 7px; }"
        "QTabBar::tab:selected { background: #ffffff; color: #147d66; font-weight: 600; }"
        "QCalendarWidget QToolButton { background: #cceee4; color: #164e42; border-radius: 5px; padding: 5px; }"
        "QCalendarWidget QTableView { background: #ffffff; color: #1f2937; alternate-background-color: #f7f9fc; "
        "selection-background-color: #2563eb; selection-color: #ffffff; }"
        "QComboBox { combobox-popup: 0; }"
        "QComboBox QAbstractItemView { background: #ffffff; color: #1f2937; border: 1px solid #cbd5e1; "
        "border-radius: 8px; padding: 4px; selection-background-color: #dbeafe; selection-color: #172554; }"
        "QComboBox QAbstractItemView::item { min-height: 28px; padding: 4px 8px; color: #1f2937; }"
        "QMessageBox { background: #ffffff; color: #1f2937; }"
        "QMessageBox QLabel { color: #1f2937; }"
    ));
}

#endif
