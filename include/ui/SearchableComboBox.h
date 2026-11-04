#ifndef SEARCHABLECOMBOBOX_H
#define SEARCHABLECOMBOBOX_H

#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFocusEvent>

// Widget ComboBox thông minh:
// 1. Cho phép vừa gõ tìm kiếm trực tiếp (lọc thời gian thực) vừa có danh sách chọn đầy đủ.
// 2. Danh sách dropdown chỉ kéo dài từ mép dưới của box đi xuống, không bị trồi đè lên box.
// 3. Giữ popup mở ổn định khi click chuột, không bị biến mất khi thả chuột.
// 4. Hỗ trợ phím mũi tên Lên / Xuống để duyệt các kết quả đã lọc và Enter để chọn.
// 5. Hoàn toàn tương thích với các hàm chuẩn của QComboBox: currentData(), itemData(), addItem(), clear().
class SearchableComboBox : public QComboBox {
    Q_OBJECT

public:
    explicit SearchableComboBox(QWidget* parent = nullptr);
    ~SearchableComboBox() override = default;

    void setPlaceholder(const QString& placeholder);

protected:
    void showPopup() override;
    void hidePopup() override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;

private slots:
    void onTextEdited(const QString& text);
    void onItemClicked(const QModelIndex& index);

private:
    void filterItems(const QString& keyword);
    void resetFilter();
    void selectRow(int row);
    int findNextVisibleRow(int currentRow, int direction);

    QString m_placeholder;
    bool m_updatingText = false;
};

#endif // SEARCHABLECOMBOBOX_H
