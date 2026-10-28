#ifndef RBTREE_H
#define RBTREE_H

#include <vector>

// Cây đỏ-đen tổng quát, giữ dữ liệu có khóa luôn cân bằng sau thêm/xóa.
template <typename Key, typename Value>
class RBTree {
private:
    struct Node {
        Key key;
        Value value;
        bool isRed;
        Node* parent;
        Node* left;
        Node* right;

        Node()
            : key(), value(), isRed(false), parent(nullptr), left(nullptr), right(nullptr) {}

        Node(const Key& nodeKey, const Value& nodeValue, Node* nil)
            : key(nodeKey), value(nodeValue), isRed(true),
              parent(nil), left(nil), right(nil) {}
    };

    Node* nil_;
    Node* root_;
    std::size_t size_;

    Node* findNode(const Key& key) const {
        Node* current = root_;
        while (current != nil_) {
            if (key == current->key) {
                return current;
            }
            current = key < current->key ? current->left : current->right;
        }
        return nil_;
    }

    void rotateLeft(Node* node) {
        Node* pivot = node->right;
        node->right = pivot->left;
        if (pivot->left != nil_) {
            pivot->left->parent = node;
        }

        pivot->parent = node->parent;
        if (node->parent == nil_) {
            root_ = pivot;
        } else if (node == node->parent->left) {
            node->parent->left = pivot;
        } else {
            node->parent->right = pivot;
        }

        pivot->left = node;
        node->parent = pivot;
    }

    void rotateRight(Node* node) {
        Node* pivot = node->left;
        node->left = pivot->right;
        if (pivot->right != nil_) {
            pivot->right->parent = node;
        }

        pivot->parent = node->parent;
        if (node->parent == nil_) {
            root_ = pivot;
        } else if (node == node->parent->right) {
            node->parent->right = pivot;
        } else {
            node->parent->left = pivot;
        }

        pivot->right = node;
        node->parent = pivot;
    }

    void insertFixup(Node* node) {
        while (node->parent->isRed) {
            if (node->parent == node->parent->parent->left) {
                Node* uncle = node->parent->parent->right;
                if (uncle->isRed) {
                    node->parent->isRed = false;
                    uncle->isRed = false;
                    node->parent->parent->isRed = true;
                    node = node->parent->parent;
                } else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        rotateLeft(node);
                    }
                    node->parent->isRed = false;
                    node->parent->parent->isRed = true;
                    rotateRight(node->parent->parent);
                }
            } else {
                Node* uncle = node->parent->parent->left;
                if (uncle->isRed) {
                    node->parent->isRed = false;
                    uncle->isRed = false;
                    node->parent->parent->isRed = true;
                    node = node->parent->parent;
                } else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        rotateRight(node);
                    }
                    node->parent->isRed = false;
                    node->parent->parent->isRed = true;
                    rotateLeft(node->parent->parent);
                }
            }
        }
        root_->isRed = false;
    }

    Node* minimum(Node* node) const {
        while (node->left != nil_) {
            node = node->left;
        }
        return node;
    }

    void transplant(Node* oldNode, Node* newNode) {
        if (oldNode->parent == nil_) {
            root_ = newNode;
        } else if (oldNode == oldNode->parent->left) {
            oldNode->parent->left = newNode;
        } else {
            oldNode->parent->right = newNode;
        }
        newNode->parent = oldNode->parent;
    }

    void deleteFixup(Node* node) {
        while (node != root_ && !node->isRed) {
            if (node == node->parent->left) {
                Node* sibling = node->parent->right;
                if (sibling == nil_) {
                    node = node->parent;
                    continue;
                }
                if (sibling->isRed) {
                    sibling->isRed = false;
                    node->parent->isRed = true;
                    rotateLeft(node->parent);
                    sibling = node->parent->right;
                }
                if (!sibling->left->isRed && !sibling->right->isRed) {
                    sibling->isRed = true;
                    node = node->parent;
                } else {
                    if (!sibling->right->isRed) {
                        sibling->left->isRed = false;
                        sibling->isRed = true;
                        rotateRight(sibling);
                        sibling = node->parent->right;
                    }
                    sibling->isRed = node->parent->isRed;
                    node->parent->isRed = false;
                    sibling->right->isRed = false;
                    rotateLeft(node->parent);
                    node = root_;
                }
            } else {
                Node* sibling = node->parent->left;
                if (sibling == nil_) {
                    node = node->parent;
                    continue;
                }
                if (sibling->isRed) {
                    sibling->isRed = false;
                    node->parent->isRed = true;
                    rotateRight(node->parent);
                    sibling = node->parent->left;
                }
                if (!sibling->right->isRed && !sibling->left->isRed) {
                    sibling->isRed = true;
                    node = node->parent;
                } else {
                    if (!sibling->left->isRed) {
                        sibling->right->isRed = false;
                        sibling->isRed = true;
                        rotateLeft(sibling);
                        sibling = node->parent->left;
                    }
                    sibling->isRed = node->parent->isRed;
                    node->parent->isRed = false;
                    sibling->left->isRed = false;
                    rotateRight(node->parent);
                    node = root_;
                }
            }
        }
        node->isRed = false;
    }

    template <typename PointerType>
    void collectPointers(Node* node, std::vector<PointerType>& result) const {
        if (node == nil_) {
            return;
        }
        collectPointers(node->left, result);
        result.push_back(&node->value);
        collectPointers(node->right, result);
    }

    void destroySubtree(Node* node) {
        if (node == nil_) {
            return;
        }
        destroySubtree(node->left);
        destroySubtree(node->right);
        delete node;
    }

public:
    RBTree() : nil_(new Node()), root_(nullptr), size_(0) {
        nil_->parent = nil_;
        nil_->left = nil_;
        nil_->right = nil_;
        root_ = nil_;
    }

    ~RBTree() {
        clear();
        delete nil_;
    }

    RBTree(const RBTree&) = delete;
    RBTree& operator=(const RBTree&) = delete;

    void insert(const Key& key, const Value& value) {
        Node* parent = nil_;
        Node* current = root_;

        while (current != nil_) {
            parent = current;
            if (key == current->key) {
                current->value = value;
                return;
            }
            current = key < current->key ? current->left : current->right;
        }

        Node* node = new Node(key, value, nil_);
        node->parent = parent;
        if (parent == nil_) {
            root_ = node;
        } else if (key < parent->key) {
            parent->left = node;
        } else {
            parent->right = node;
        }

        ++size_;
        insertFixup(node);
    }

    Value* search(const Key& key) {
        Node* node = findNode(key);
        return node == nil_ ? nullptr : &node->value;
    }

    const Value* search(const Key& key) const {
        Node* node = findNode(key);
        return node == nil_ ? nullptr : &node->value;
    }

    bool remove(const Key& key) {
        Node* target = findNode(key);
        if (target == nil_) {
            return false;
        }

        Node* removedNode = target;
        bool removedWasRed = removedNode->isRed;
        Node* fixupNode = nil_;

        if (target->left == nil_) {
            fixupNode = target->right;
            transplant(target, target->right);
        } else if (target->right == nil_) {
            fixupNode = target->left;
            transplant(target, target->left);
        } else {
            removedNode = minimum(target->right);
            removedWasRed = removedNode->isRed;
            fixupNode = removedNode->right;

            if (removedNode->parent == target) {
                fixupNode->parent = removedNode;
            } else {
                transplant(removedNode, removedNode->right);
                removedNode->right = target->right;
                removedNode->right->parent = removedNode;
            }

            transplant(target, removedNode);
            removedNode->left = target->left;
            removedNode->left->parent = removedNode;
            removedNode->isRed = target->isRed;
        }

        delete target;
        --size_;
        if (!removedWasRed) {
            deleteFixup(fixupNode);
        }
        nil_->parent = nil_;
        return true;
    }

    void clear() {
        destroySubtree(root_);
        root_ = nil_;
        nil_->parent = nil_;
        size_ = 0;
    }

    bool isEmpty() const {
        return size_ == 0;
    }

    std::size_t size() const {
        return size_;
    }

    std::vector<Value*> getAllPointers() {
        std::vector<Value*> result;
        result.reserve(size_);
        collectPointers(root_, result);
        return result;
    }

    std::vector<const Value*> getAllPointers() const {
        std::vector<const Value*> result;
        result.reserve(size_);
        collectPointers(root_, result);
        return result;
    }
};

#endif
