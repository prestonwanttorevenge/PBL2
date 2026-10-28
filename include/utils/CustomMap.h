#ifndef CUSTOM_MAP_H
#define CUSTOM_MAP_H
#include <string>
#include <vector>
using namespace std;

// Bộ đếm nhỏ theo nhóm, lưu cặp khóa/giá trị trong hai vector song song.
struct CounterMap {
    vector<string> keys;
    vector<int> values;

    int& operator[](const string& key) {
        for (size_t i = 0; i < keys.size(); ++i) {
            if (keys[i] == key) return values[i];
        }
        keys.push_back(key);
        values.push_back(0);
        return values.back();
    }

    void clear() {
        keys.clear();
        values.clear();
    }
};
struct RoomInfo {
    string first;
    vector<string> second;
    
    RoomInfo(string f, vector<string> s) {
        first = f;
        second = s;
    }
};

// Node phòng chứa các giường còn trống trong cây tìm kiếm theo tên phòng.
struct RoomNode {
    string roomName;
    vector<string> availableBeds;
    RoomNode* left;
    RoomNode* right;

    RoomNode(string name, vector<string> beds) 
        : roomName(name), availableBeds(beds), left(nullptr), right(nullptr) {}
};

class CustomRoomMap {
private:
    RoomNode* root;

    RoomNode* insertRec(RoomNode* node, string roomName, vector<string> beds) {
        if (node == nullptr) return new RoomNode(roomName, beds);
        if (roomName < node->roomName)
            node->left = insertRec(node->left, roomName, beds);
        else if (roomName > node->roomName)
            node->right = insertRec(node->right, roomName, beds);
        return node;
    }

    RoomNode* searchRec(RoomNode* node, string roomName) {
        if (node == nullptr || node->roomName == roomName) return node;
        if (roomName < node->roomName) return searchRec(node->left, roomName);
        return searchRec(node->right, roomName);
    }

    void destroyRec(RoomNode* node) {
        if (node != nullptr) {
            destroyRec(node->left);
            destroyRec(node->right);
            delete node;
        }
    }

    void flattenRec(RoomNode* node, vector<RoomInfo>& result) {
        if (node != nullptr) {
            flattenRec(node->left, result);
            if (!node->availableBeds.empty()) {
                result.push_back(RoomInfo(node->roomName, node->availableBeds));
            }
            flattenRec(node->right, result);
        }
    }

public:
    CustomRoomMap() : root(nullptr) {}
    ~CustomRoomMap() { destroyRec(root); }

    void insert(string roomName, vector<string> beds) {
        root = insertRec(root, roomName, beds);
    }

    vector<string>& operator[](const string& roomName) {
        RoomNode* node = searchRec(root, roomName);
        if (node == nullptr) {
            insert(roomName, vector<string>());
            node = searchRec(root, roomName);
        }
        return node->availableBeds;
    }

    bool contains(const string& roomName) {
        return searchRec(root, roomName) != nullptr;
    }
    
    void removeBedFromRoom(const string& roomName, const string& bedNum) {
        RoomNode* node = searchRec(root, roomName);
        if (node != nullptr) {
            vector<string>& beds = node->availableBeds;
            vector<string> tempBeds;
            for (size_t i = 0; i < beds.size(); i++) {
                if (beds[i] != bedNum) {
                    tempBeds.push_back(beds[i]);
                }
            }
            node->availableBeds = tempBeds; 
        }
    }

    vector<RoomInfo> getValidRooms() {
        vector<RoomInfo> result;
        flattenRec(root, result);
        return result;
    }
};

#endif // CUSTOM_MAP_H