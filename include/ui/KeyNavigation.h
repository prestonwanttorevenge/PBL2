#ifndef KEYNAVIGATION_H
#define KEYNAVIGATION_H

#include <QObject>
#include <QEvent>
#include <QKeyEvent>
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QAbstractSpinBox>
#include <QAbstractItemView>
#include <QApplication>

// Helper truy cập hàm điều hướng focus chuẩn của QWidget
class FocusNavigator : public QWidget {
public:
    static bool stepFocus(QWidget* target, bool next) {
        if (!target) return false;
        return static_cast<FocusNavigator*>(target)->focusNextPrevChild(next);
    }
};

// Bộ lọc sự kiện bàn phím toàn cục:
// 1. Cho phép dùng phím mũi tên Lên / Xuống (Up / Down) để chuyển đổi mượt mà giữa các ô nhập liệu (box).
// 2. Bảo toàn hoàn toàn chức năng phím Enter (đăng nhập, tìm kiếm, kích hoạt nút bấm).
// 3. Không ảnh hưởng khi đang duyệt danh sách ComboBox hoặc duyệt dòng trong Table.
class KeyNavigationFilter : public QObject {
    Q_OBJECT

public:
    explicit KeyNavigationFilter(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::KeyPress) {
            auto* keyEvent = static_cast<QKeyEvent*>(event);
            const int key = keyEvent->key();

            auto* widget = qobject_cast<QWidget*>(watched);
            if (!widget) return false;

            // 1. Không can thiệp nếu sự kiện xảy ra bên trong Bảng (QTableWidget, QTableView)
            // để người dùng vẫn dùng phím mũi tên duyệt hàng/cột trong bảng bình thường.
            if (qobject_cast<QAbstractItemView*>(widget) || 
                (widget->parentWidget() && qobject_cast<QAbstractItemView*>(widget->parentWidget()))) {
                return false;
            }

            // 2. Kiểm tra xem widget có phải là QComboBox hoặc lineEdit con của QComboBox không
            QComboBox* cb = qobject_cast<QComboBox*>(widget);
            if (!cb && widget->parentWidget()) {
                cb = qobject_cast<QComboBox*>(widget->parentWidget());
            }

            if (cb) {
                // Nếu dropdown popup của ComboBox đang mở, để popup tự do xử lý Up / Down / Enter
                if (cb->view() && cb->view()->isVisible()) {
                    return false;
                }
            }

            // 3. Xử lý đối với QLineEdit thông thường (khi không nằm trong popup đang mở)
            // Phím mũi tên Xuống (Down): Chuyển sang box kế tiếp
            // Phím mũi tên Lên (Up): Chuyển về box trước đó
            // Giữ nguyên phím Enter/Return để QLineEdit tự do phát tín hiệu returnPressed() (như đăng nhập, tìm kiếm)
            if (qobject_cast<QLineEdit*>(widget)) {
                if (key == Qt::Key_Down) {
                    return FocusNavigator::stepFocus(widget, true);
                } else if (key == Qt::Key_Up) {
                    return FocusNavigator::stepFocus(widget, false);
                }
            }

            // 4. Xử lý đối với QComboBox (khi popup đang đóng)
            if (cb) {
                if (key == Qt::Key_Down) {
                    return FocusNavigator::stepFocus(cb, true);
                } else if (key == Qt::Key_Up) {
                    return FocusNavigator::stepFocus(cb, false);
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

inline void installGlobalKeyNavigation(QApplication* app) {
    if (app) {
        app->installEventFilter(new KeyNavigationFilter(app));
    }
}

#endif // KEYNAVIGATION_H
