#ifndef CLINICSTYLE_H
#define CLINICSTYLE_H

#include <QWidget>

inline void applyClinicStyle(QWidget* widget) {
    widget->setStyleSheet(QStringLiteral(
        "QMainWindow, QWidget#centralwidget { background: #f3f6fb; color: #1f2937; "
        "font-family: 'Segoe UI'; font-size: 10pt; }"
        "QGroupBox { background: #ffffff; border: 1px solid #dce4ef; border-radius: 12px; "
        "margin-top: 12px; padding: 14px; font-weight: 600; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 14px; padding: 0 6px; color: #334155; }"
        "QLineEdit, QComboBox, QDateEdit, QSpinBox { background: #ffffff; border: 1px solid #cbd5e1; "
        "border-radius: 7px; padding: 8px 10px; min-height: 20px; selection-background-color: #2563eb; }"
        "QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QSpinBox:focus { border: 1px solid #2563eb; }"
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
        "QTableWidget { background: #ffffff; alternate-background-color: #f7f9fc; "
        "border: 1px solid #dce4ef; border-radius: 9px; gridline-color: #edf1f6; "
        "selection-background-color: #dbeafe; selection-color: #172554; }"
        "QHeaderView::section { background: #edf2f8; color: #334155; border: 0; "
        "border-bottom: 1px solid #dce4ef; padding: 9px; font-weight: 600; }"
        "QTabWidget::pane { background: #ffffff; border: 1px solid #dce4ef; border-radius: 9px; }"
        "QTabBar::tab { background: #e3f2ed; color: #315c50; padding: 9px 14px; margin-right: 4px; "
        "border-top-left-radius: 7px; border-top-right-radius: 7px; }"
        "QTabBar::tab:selected { background: #ffffff; color: #147d66; font-weight: 600; }"
        "QCalendarWidget QToolButton { background: #cceee4; border-radius: 5px; padding: 5px; }"
        "QCalendarWidget QTableView { background: #ffffff; alternate-background-color: #f7f9fc; "
        "selection-background-color: #2563eb; selection-color: #ffffff; }"
    ));
}

#endif
