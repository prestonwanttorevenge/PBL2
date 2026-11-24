#include "services/Database.h"
#include "utils/RBTree.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> generateSyntheticOutpatientLines(int count) {
    std::vector<std::string> lines;
    lines.reserve(static_cast<std::size_t>(count));
    for (int i = 1; i <= count; ++i) {
        std::ostringstream oss;
        oss << "NgoaiTru;BN" << std::setfill('0') << std::setw(5) << i
            << ";Benh Nhan Thu Nghiem " << i
            << ";01/01/2000;Nam;0901234567;;;01/01/2026;P101";
        lines.push_back(oss.str());
    }
    return lines;
}

std::string extractPatientId(const std::string& csvLine) {
    const std::size_t firstSep = csvLine.find(';');
    if (firstSep == std::string::npos) return "";
    const std::size_t secondSep = csvLine.find(';', firstSep + 1);
    if (secondSep == std::string::npos) return "";
    return csvLine.substr(firstSep + 1, secondSep - firstSep - 1);
}

double medianOf(std::vector<double>& samples) {
    std::sort(samples.begin(), samples.end());
    const std::size_t n = samples.size();
    if (n % 2 == 1) {
        return samples[n / 2];
    }
    return 0.5 * (samples[n / 2 - 1] + samples[n / 2]);
}

} // namespace

int main() {
    namespace fs = std::filesystem;
    const fs::path tempDir = fs::temp_directory_path() /
        ("pbl2-rbtree-bench-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::create_directories(tempDir);

    const std::vector<int> sizes = {
        100, 500, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000
    };
    const int repeats = 11;
    const int lookupsPerRun = 2000;

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "N,LoadIndex_ms,SearchAvg_us,WriteCsv_ms\n";

    for (int n : sizes) {
        const fs::path inputFile = tempDir / ("patients_" + std::to_string(n) + ".csv");
        const fs::path outputFile = tempDir / ("patients_out_" + std::to_string(n) + ".csv");

        const auto syntheticLines = generateSyntheticOutpatientLines(n);
        {
            std::ofstream out(inputFile);
            for (const std::string& line : syntheticLines) {
                out << line << '\n';
            }
        }

        std::vector<std::string> queryKeys;
        queryKeys.reserve(lookupsPerRun);
        for (int q = 0; q < lookupsPerRun; ++q) {
            const int idx = ((q * 7919) % n) + 1;
            std::ostringstream oss;
            oss << "BN" << std::setfill('0') << std::setw(5) << idx;
            queryKeys.push_back(oss.str());
        }

        std::vector<double> loadSamples;
        std::vector<double> searchSamples;
        std::vector<double> writeSamples;
        loadSamples.reserve(repeats);
        searchSamples.reserve(repeats);
        writeSamples.reserve(repeats);

        volatile std::size_t sink = 0;

        for (int r = 0; r < repeats; ++r) {
            RBTree<std::string, std::string> tree;

            const auto tLoadStart = std::chrono::steady_clock::now();
            {
                std::ifstream in(inputFile);
                std::string line;
                while (std::getline(in, line)) {
                    if (line.empty()) continue;
                    tree.insert(extractPatientId(line), line);
                }
            }
            const auto tLoadEnd = std::chrono::steady_clock::now();
            loadSamples.push_back(
                std::chrono::duration<double, std::milli>(tLoadEnd - tLoadStart).count());

            const auto tSearchStart = std::chrono::steady_clock::now();
            for (const std::string& key : queryKeys) {
                const std::string* found = tree.search(key);
                if (found != nullptr) {
                    sink += found->size();
                }
            }
            const auto tSearchEnd = std::chrono::steady_clock::now();
            const double searchTotalUs =
                std::chrono::duration<double, std::micro>(tSearchEnd - tSearchStart).count();
            searchSamples.push_back(searchTotalUs / static_cast<double>(lookupsPerRun));

            const auto tWriteStart = std::chrono::steady_clock::now();
            {
                std::ofstream out(outputFile);
                const std::vector<std::string*> allRows = tree.getAllPointers();
                for (const std::string* row : allRows) {
                    out << *row << '\n';
                }
            }
            const auto tWriteEnd = std::chrono::steady_clock::now();
            writeSamples.push_back(
                std::chrono::duration<double, std::milli>(tWriteEnd - tWriteStart).count());
        }

        (void)sink;
        std::cout << n << ","
                  << medianOf(loadSamples) << ","
                  << medianOf(searchSamples) << ","
                  << medianOf(writeSamples) << '\n';
    }

    std::error_code ec;
    fs::remove_all(tempDir, ec);
    return 0;
}
