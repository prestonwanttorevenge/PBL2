#include "utils/CustomMap.h"
#include "utils/RBTree.h"
#include "models/User.h"

#include <cstdlib>
#include <iostream>
#include <map>
#include <random>
#include <string>
#include <vector>

#define CHECK(expr)                                                                      \
    do {                                                                                 \
        if (!(expr)) {                                                                   \
            std::cerr << "CHECK failed at " << __FILE__ << ":" << __LINE__               \
                      << " -> " << #expr << '\n';                                        \
            std::exit(1);                                                                \
        }                                                                                \
    } while (false)

int main() {
    // Kiểm tra cấu trúc cây, phòng/giường, bộ đếm và username.
    CHECK(User::isValidUsername("admin_01"));
    CHECK(User::isValidUsername("Nurse-01"));
    CHECK(!User::isValidUsername("người_dùng"));
    CHECK(!User::isValidUsername("ab"));
    CHECK(!User::isValidUsername("invalid name"));

    const int count = 5000;
    RBTree<int, int> tree;
    std::vector<int> expected(count);
    std::vector<bool> active(count, true);

    for (int key = 0; key < count; ++key) {
        tree.insert(key, key);
        expected[key] = key;
    }
    CHECK(tree.verifyInvariants());

    for (int key = 0; key < count; key += 3) {
        tree.insert(key, -key);
        expected[key] = -key;
    }
    CHECK(tree.verifyInvariants());

    for (int key = 0; key < count; ++key) {
        int* value = tree.search(key);
        CHECK(value != nullptr && *value == expected[key]);
    }

    std::vector<int*> values = tree.getAllPointers();
    CHECK(values.size() == static_cast<std::size_t>(count));
    for (int key = 0; key < count; ++key) CHECK(*values[key] == expected[key]);

    for (int index = 0; index < count; ++index) {
        const int key = (index * 7919) % count;
        CHECK(active[key] && tree.remove(key));
        active[key] = false;
        CHECK(!tree.remove(key));
        if (index % 250 == 0) {
            CHECK(tree.verifyInvariants());
            std::size_t remaining = 0;
            for (bool exists : active) if (exists) ++remaining;
            values = tree.getAllPointers();
            CHECK(tree.size() == remaining && values.size() == remaining);
            std::size_t position = 0;
            for (int candidate = 0; candidate < count; ++candidate) {
                if (active[candidate]) CHECK(*values[position++] == expected[candidate]);
            }
        }
    }
    CHECK(tree.isEmpty() && tree.verifyInvariants());
    tree.clear();
    tree.insert(7, 42);
    CHECK(tree.size() == 1 && *tree.search(7) == 42 && tree.verifyInvariants());
    tree.clear();
    CHECK(tree.isEmpty() && tree.verifyInvariants());

    // Fuzz test ngẫu nhiên 100.000 thao tác chèn/xóa đối chiếu std::map và kiểm tra 5 tính chất đỏ-đen.
    {
        RBTree<int, int> fuzzTree;
        std::map<int, int> reference;
        std::mt19937 rng(20261124u);
        std::uniform_int_distribution<int> keyDist(0, 4999);
        std::uniform_int_distribution<int> opDist(0, 99);

        for (int step = 0; step < 100000; ++step) {
            const int key = keyDist(rng);
            const int op = opDist(rng);
            if (op < 55) {
                const int val = step;
                fuzzTree.insert(key, val);
                reference[key] = val;
            } else if (op < 90) {
                const bool removedTree = fuzzTree.remove(key);
                const bool removedRef = reference.erase(key) > 0;
                CHECK(removedTree == removedRef);
            } else {
                const int* found = fuzzTree.search(key);
                const auto it = reference.find(key);
                if (it == reference.end()) {
                    CHECK(found == nullptr);
                } else {
                    CHECK(found != nullptr && *found == it->second);
                }
            }

            if (step % 200 == 0) {
                CHECK(fuzzTree.verifyInvariants());
                CHECK(fuzzTree.size() == reference.size());
            }
        }
        CHECK(fuzzTree.verifyInvariants());
        const std::vector<int*> fuzzPointers = fuzzTree.getAllPointers();
        CHECK(fuzzPointers.size() == reference.size());
        std::size_t idx = 0;
        for (const auto& [k, v] : reference) {
            (void)k;
            CHECK(*fuzzPointers[idx++] == v);
        }
    }

    // Chèn tuần tự 100.000 khóa tăng dần để kiểm tra giới hạn chiều cao 2*log2(N+1) ~ 33.2.
    {
        RBTree<int, int> seqTree;
        for (int key = 0; key < 100000; ++key) {
            seqTree.insert(key, key);
        }
        CHECK(seqTree.size() == 100000);
        CHECK(seqTree.verifyInvariants());
        CHECK(seqTree.maxHeight() <= 33);
    }

    CustomRoomMap rooms;
    for (int index = 0; index < 80; ++index) {
        rooms.insert("Room" + std::to_string(index), std::vector<std::string>{"Bed1", "Bed2"});
    }
    CHECK(rooms.contains("Room47") && !rooms.contains("Missing"));
    rooms.removeBedFromRoom("Room47", "Bed1");
    CHECK(rooms["Room47"].size() == 1 && rooms["Room47"][0] == "Bed2");
    const std::vector<RoomInfo> flattened = rooms.getValidRooms();
    CHECK(flattened.size() == 80);
    for (std::size_t index = 1; index < flattened.size(); ++index) {
        CHECK(flattened[index - 1].first < flattened[index].first);
    }
    rooms.removeBedFromRoom("Room10", "Bed1");
    rooms.removeBedFromRoom("Room10", "Bed2");
    CHECK(rooms.getValidRooms().size() == 79);

    CounterMap counters;
    counters["Admin"] += 1;
    counters["Admin"] += 2;
    counters["Doctor"] += 1;
    CHECK(counters["Admin"] == 3 && counters["Doctor"] == 1);
    counters.clear();
    CHECK(counters.keys.empty() && counters.values.empty());

    std::cout << "All algorithm checks passed\n";
}
