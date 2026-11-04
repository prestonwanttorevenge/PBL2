#include "ui/SearchableComboBox.h"
#include <QListView>
#include <QScrollBar>
#include <QApplication>
#include <QMouseEvent>

SearchableComboBox::SearchableComboBox(QWidget* parent)
    : QComboBox(parent) {
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    setMaxVisibleItems(10);
    setFocusPolicy(Qt::StrongFocus);

    // Ép QComboBox luôn sử dụng danh sách drop-down mở bên dưới thay vì popup menu trùm lên box
    setStyleSheet(QStringLiteral("QComboBox { combobox-popup: 0; }"));

    if (view()) {
        view()->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        view()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view()->setSelectionBehavior(QAbstractItemView::SelectRows);
        view()->setSelectionMode(QAbstractItemView::SingleSelection);
        view()->installEventFilter(this);
        connect(view(), &QAbstractItemView::clicked, this, &SearchableComboBox::onItemClicked);
    }

    if (lineEdit()) {
        lineEdit()->installEventFilter(this);
        connect(lineEdit(), &QLineEdit::textEdited, this, &SearchableComboBox::onTextEdited);
    }
}

void SearchableComboBox::setPlaceholder(const QString& placeholder) {
    m_placeholder = placeholder;
    if (lineEdit()) {
        lineEdit()->setPlaceholderText(m_placeholder);
    }
}

void SearchableComboBox::showPopup() {
    resetFilter();
    if (view()) {
        // Đảm bảo chiều rộng danh sách đủ rộng để hiển thị rõ thông tin chi tiết
        view()->setMinimumWidth(qMax(width(), 420));
    }
    QComboBox::showPopup();

    if (lineEdit()) {
        lineEdit()->selectAll();
    }
}

void SearchableComboBox::hidePopup() {
    QComboBox::hidePopup();
    resetFilter();

    if (currentIndex() >= 0 && currentIndex() < count() && lineEdit()) {
        m_updatingText = true;
        lineEdit()->setText(itemText(currentIndex()));
        m_updatingText = false;
    }
}

void SearchableComboBox::resetFilter() {
    auto* lv = qobject_cast<QListView*>(view());
    if (!lv) return;
    for (int i = 0; i < count(); ++i) {
        lv->setRowHidden(i, false);
    }
}

void SearchableComboBox::filterItems(const QString& keyword) {
    auto* lv = qobject_cast<QListView*>(view());
    if (!lv) return;
    const QString trimmed = keyword.trimmed();
    int firstVisible = -1;

    for (int i = 0; i < count(); ++i) {
        const QString text = itemText(i);
        const bool match = trimmed.isEmpty() || text.contains(trimmed, Qt::CaseInsensitive);
        lv->setRowHidden(i, !match);
        if (match && firstVisible == -1) {
            firstVisible = i;
        }
    }

    if (firstVisible != -1 && model()) {
        lv->setCurrentIndex(model()->index(firstVisible, 0));
        lv->scrollTo(model()->index(firstVisible, 0));
    }
}

int SearchableComboBox::findNextVisibleRow(int currentRow, int direction) {
    auto* lv = qobject_cast<QListView*>(view());
    if (!lv) return -1;
    if (direction > 0) {
        for (int i = currentRow + 1; i < count(); ++i) {
            if (!lv->isRowHidden(i)) return i;
        }
    } else {
        for (int i = currentRow - 1; i >= 0; --i) {
            if (!lv->isRowHidden(i)) return i;
        }
    }
    if (currentRow >= 0 && currentRow < count() && !lv->isRowHidden(currentRow)) {
        return currentRow;
    }
    return -1;
}

void SearchableComboBox::selectRow(int row) {
    if (row >= 0 && row < count()) {
        m_updatingText = true;
        setCurrentIndex(row);
        if (lineEdit()) {
            lineEdit()->setText(itemText(row));
        }
        m_updatingText = false;
        hidePopup();
        resetFilter();
        emit activated(row);
        emit currentIndexChanged(row);
    }
}

void SearchableComboBox::onTextEdited(const QString& text) {
    if (m_updatingText) return;
    filterItems(text);

    if (view() && !view()->isVisible()) {
        showPopup();
    }
}

void SearchableComboBox::onItemClicked(const QModelIndex& index) {
    if (index.isValid()) {
        selectRow(index.row());
    }
}

bool SearchableComboBox::eventFilter(QObject* watched, QEvent* event) {
    if (watched == lineEdit()) {
        // Chỉ mở popup khi người dùng ĐÃ THẢ CHUỘT TRÁI (MouseButtonRelease)
        // Tuyệt đối không mở ở MouseButtonPress vì sự kiện Release tiếp theo sẽ bị Qt hiểu nhầm là click ngoài popup và tự đóng!
        if (event->type() == QEvent::MouseButtonRelease) {
            auto* me = static_cast<QMouseEvent*>(event);
            if (me->button() == Qt::LeftButton) {
                if (view() && !view()->isVisible() && count() > 0 && isEnabled()) {
                    showPopup();
                    if (lineEdit()) {
                        lineEdit()->selectAll();
                    }
                    return true;
                }
            }
        }
    }

    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        const int key = keyEvent->key();
        auto* lv = qobject_cast<QListView*>(view());

        if (lv && lv->isVisible()) {
            if (key == Qt::Key_Down) {
                const int cur = lv->currentIndex().isValid() ? lv->currentIndex().row() : -1;
                const int next = findNextVisibleRow(cur, 1);
                if (next != -1 && model()) {
                    lv->setCurrentIndex(model()->index(next, 0));
                    lv->scrollTo(model()->index(next, 0));
                }
                return true;
            } else if (key == Qt::Key_Up) {
                const int cur = lv->currentIndex().isValid() ? lv->currentIndex().row() : 0;
                const int prev = findNextVisibleRow(cur, -1);
                if (prev != -1 && model()) {
                    lv->setCurrentIndex(model()->index(prev, 0));
                    lv->scrollTo(model()->index(prev, 0));
                }
                return true;
            } else if (key == Qt::Key_Return || key == Qt::Key_Enter) {
                const QModelIndex cur = lv->currentIndex();
                if (cur.isValid() && !lv->isRowHidden(cur.row())) {
                    selectRow(cur.row());
                } else {
                    hidePopup();
                }
                return true;
            } else if (key == Qt::Key_Escape) {
                hidePopup();
                return true;
            }
        }
    }

    return QComboBox::eventFilter(watched, event);
}

void SearchableComboBox::focusOutEvent(QFocusEvent* event) {
    QComboBox::focusOutEvent(event);
    if (view() && !view()->isVisible() && currentIndex() >= 0 && currentIndex() < count()) {
        if (lineEdit() && lineEdit()->text() != itemText(currentIndex())) {
            m_updatingText = true;
            lineEdit()->setText(itemText(currentIndex()));
            m_updatingText = false;
        }
    }
}
