#ifndef BST_H
#define BST_H

#include <string>
#include <vector>

// ==========================================
// 1. CẤU TRÚC NODE (LÁ CÂY)
// ==========================================
template <typename T>
struct TreeNode {
    std::string key; // Chìa khóa (Ví dụ: ID Bác sĩ "BS01", ID Thuốc "M01")
    T data;          // Hành lý (Đối tượng Doctor, User, Medicine...)
    TreeNode* left;
    TreeNode* right;

    // Hàm dựng khởi tạo Node
    TreeNode(std::string k, T d) : key(k), data(d), left(nullptr), right(nullptr) {}
};

// ==========================================
// 2. LỚP CÂY NHỊ PHÂN TÌM KIẾM (BST)
// ==========================================
template <typename T>
class BST {
private:
    TreeNode<T>* root;

    // --- CÁC HÀM ĐỆ QUY ẨN (BACKEND CỦA CÂY) ---
    // Người dùng bên ngoài sẽ không nhìn thấy và không thể gọi các hàm này

    void insertRec(TreeNode<T>*& node, std::string key, T data) {
        if (node == nullptr) {
            node = new TreeNode<T>(key, data);
        } else if (key < node->key) {
            insertRec(node->left, key, data);
        } else if (key > node->key) {
            insertRec(node->right, key, data);
        } else {
            // Nếu Key (ID) đã tồn tại, tự động cập nhật dữ liệu mới đè lên dữ liệu cũ
            node->data = data; 
        }
    }

    TreeNode<T>* searchRec(TreeNode<T>* node, std::string key) const {
        if (node == nullptr || node->key == key) return node;
        if (key < node->key) return searchRec(node->left, key);
        return searchRec(node->right, key);
    }

    TreeNode<T>* findMin(TreeNode<T>* node) {
        while (node && node->left != nullptr) node = node->left;
        return node;
    }

    TreeNode<T>* removeRec(TreeNode<T>* node, std::string key, bool& success) {
        if (node == nullptr) return node;
        
        if (key < node->key) {
            node->left = removeRec(node->left, key, success);
        } else if (key > node->key) {
            node->right = removeRec(node->right, key, success);
        } else {
            success = true;
            // Trường hợp 1 & 2: Node có 1 con hoặc không có con nào
            if (node->left == nullptr) {
                TreeNode<T>* temp = node->right;
                delete node;
                return temp;
            } else if (node->right == nullptr) {
                TreeNode<T>* temp = node->left;
                delete node;
                return temp;
            }
            // Trường hợp 3: Node có đủ 2 con (Tìm node nhỏ nhất ở nhánh phải để thế chỗ)
            TreeNode<T>* temp = findMin(node->right);
            node->key = temp->key;
            node->data = temp->data;
            node->right = removeRec(node->right, temp->key, success);
        }
        return node;
    }

    // Hàm đệ quy duyệt cây (In-order) và gom địa chỉ (Con trỏ)
    void getPointersRec(TreeNode<T>* node, std::vector<T*>& result) const {
        if (node != nullptr) {
            getPointersRec(node->left, result);
            result.push_back(&(node->data)); // Chỉ lấy địa chỉ của data (Siêu tiết kiệm RAM)
            getPointersRec(node->right, result);
        }
    }

    void clearRec(TreeNode<T>* node) {
        if (node != nullptr) {
            clearRec(node->left);
            clearRec(node->right);
            delete node; // Giải phóng bộ nhớ
        }
    }

public:
    // Khởi tạo cây rỗng
    BST() : root(nullptr) {}
    
    // Hủy cây, tự động dọn rác RAM khi chương trình kết thúc
    ~BST() { clear(); }

    // --- CÁC HÀM GIAO TIẾP RA BÊN NGOÀI (DÀNH CHO DATABASE.CPP) ---

    // 1. Thêm một đối tượng vào cây
    void insert(std::string key, T data) {
        insertRec(root, key, data);
    }

    // 2. Tìm kiếm và trả về ĐỊA CHỈ của dữ liệu (pointer) để có thể chỉnh sửa trực tiếp
    T* search(std::string key) const {
        TreeNode<T>* result = searchRec(root, key);
        if (result != nullptr) return &(result->data);
        return nullptr;
    }

    // 3. Xóa một đối tượng khỏi cây theo ID
    bool remove(std::string key) {
        bool success = false;
        root = removeRec(root, key, success);
        return success;
    }

    // 4. Quét toàn bộ Cây và gom các con trỏ (địa chỉ) thành 1 Mảng (Vector)
    // Dùng cho vòng lặp lưu file hoặc hiển thị lên bảng Qt
    std::vector<T*> getAllPointers() const {
        std::vector<T*> result;
        getPointersRec(root, result);
        return result;
    }

    // 5. Xóa sạch toàn bộ cây
    void clear() {
        clearRec(root);
        root = nullptr;
    }
    
    // 6. Kiểm tra cây có rỗng không
    bool isEmpty() const { 
        return root == nullptr; 
    }
};

#endif