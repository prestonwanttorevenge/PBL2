#include "utils/CustomMap.h"
#include "utils/RBTree.h"
#include "models/User.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

int main() {
    assert(User::isValidUsername("admin_01"));
    assert(User::isValidUsername("Nurse-01"));
    assert(!User::isValidUsername("người_dùng"));
    assert(!User::isValidUsername("ab"));
    assert(!User::isValidUsername("invalid name"));

    const int count = 5000;
    RBTree<int, int> tree;
    std::vector<int> expected(count);
    std::vector<bool> active(count, true);

    for (int key = 0; key < count; ++key) {
        tree.insert(key, key);
        expected[key] = key;
    }
    for (int key = 0; key < count; key += 3) {
        tree.insert(key, -key);
        expected[key] = -key;
    }
    for (int key = 0; key < count; ++key) {
        int* value = tree.search(key);
        assert(value != nullptr && *value == expected[key]);
    }

    std::vector<int*> values = tree.getAllPointers();
    assert(values.size() == static_cast<std::size_t>(count));
    for (int key = 0; key < count; ++key) assert(*values[key] == expected[key]);

    for (int index = 0; index < count; ++index) {
        const int key = (index * 7919) % count;
        assert(active[key] && tree.remove(key));
        active[key] = false;
        assert(!tree.remove(key));
        if (index % 250 == 0) {
            std::size_t remaining = 0;
            for (bool exists : active) if (exists) ++remaining;
            values = tree.getAllPointers();
            assert(tree.size() == remaining && values.size() == remaining);
            std::size_t position = 0;
            for (int candidate = 0; candidate < count; ++candidate) {
                if (active[candidate]) assert(*values[position++] == expected[candidate]);
            }
        }
    }
    assert(tree.isEmpty());
    tree.clear();
    tree.insert(7, 42);
    assert(tree.size() == 1 && *tree.search(7) == 42);
    tree.clear();
    assert(tree.isEmpty());

    CustomRoomMap rooms;
    for (int index = 0; index < 80; ++index) {
        rooms.insert("Room" + std::to_string(index), std::vector<std::string>{"Bed1", "Bed2"});
    }
    assert(rooms.contains("Room47") && !rooms.contains("Missing"));
    rooms.removeBedFromRoom("Room47", "Bed1");
    assert(rooms["Room47"].size() == 1 && rooms["Room47"][0] == "Bed2");
    const std::vector<RoomInfo> flattened = rooms.getValidRooms();
    assert(flattened.size() == 80);
    for (std::size_t index = 1; index < flattened.size(); ++index) {
        assert(flattened[index - 1].first < flattened[index].first);
    }
    rooms.removeBedFromRoom("Room10", "Bed1");
    rooms.removeBedFromRoom("Room10", "Bed2");
    assert(rooms.getValidRooms().size() == 79);

    CounterMap counters;
    counters["Admin"] += 1;
    counters["Admin"] += 2;
    counters["Doctor"] += 1;
    assert(counters["Admin"] == 3 && counters["Doctor"] == 1);
    counters.clear();
    assert(counters.keys.empty() && counters.values.empty());

    std::cout << "All algorithm checks passed\n";
}
