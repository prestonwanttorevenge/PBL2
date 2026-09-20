#include "../../include/utils/Helper.h"
#include <sstream>
#include <iostream>
#include <cstdlib>
std::vector<std::string> Helper::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(str);
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}
void Helper::clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
void Helper::pauseScreen() {
    std::cout << "\nNhan Enter de tiep tuc...";
    std::cin.ignore();
    std::cin.get();
}