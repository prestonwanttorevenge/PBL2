#ifndef HELPER_H
#define HELPER_H
#include <string>
#include <vector>

class Helper {
    public:
        static std::vector<std::string> split(const std::string& str, char delimiter);
        static void clearScreen();
        static void pauseScreen();
        static bool isLeapYear(int year) {
            return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
        }
};

#endif