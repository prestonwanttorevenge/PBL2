#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "ui/Login.h"
#include "ui/KeyNavigation.h"
#include "services/Database.h"

// Thư viện phục vụ Benchmark
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

// Hàm tự động sinh N dòng bệnh nhân giả mạo vào file
void generateDummyPatientFile(int N, const std::filesystem::path& filePath) {
    std::ofstream f(filePath); 
    for (int i = 1; i <= N; ++i) {
        f << "NgoaiTru;BN" << std::setfill('0') << std::setw(5) << i 
          << ";Benh Nhan Test " << i 
          << ";01/01/2000;Nam;0901234567;;;01/01/2026;P101\n";
    }
    f.close();
}

void runPerformanceBenchmark() {
    qDebug() << "Bat dau kiem thu hieu nang trong thu muc tam. Du lieu goc duoc giu nguyen...";

    namespace fs = std::filesystem;
    const fs::path originalCwd = fs::current_path();
    const fs::path sourceDataDir = originalCwd / "data";
    const fs::path benchRoot = fs::temp_directory_path() /
        ("pbl2-bench-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
    const fs::path benchDataDir = benchRoot / "data";
    const fs::path dummyPatientFilePath = benchDataDir / "patients.csv";
    const fs::path outCsvPath = originalCwd / "benchmark_results.csv";

    try {
        fs::create_directories(benchDataDir);
        if (fs::exists(sourceDataDir)) {
            for (const auto& entry : fs::directory_iterator(sourceDataDir)) {
                if (entry.is_regular_file() && entry.path().filename() != "patients.csv") {
                    fs::copy_file(entry.path(), benchDataDir / entry.path().filename(),
                                  fs::copy_options::overwrite_existing);
                }
            }
        }
        fs::current_path(benchRoot);

        std::vector<int> N_values;
        int startN = 100;
        int endN = 10000;
        int numPoints = 50;
        double step = static_cast<double>(endN - startN) / (numPoints - 1);
        for (int i = 0; i < numPoints; ++i) {
            N_values.push_back(startN + static_cast<int>(i * step));
        }

        const int K = 50;
        std::ofstream outFile(outCsvPath);
        outFile << std::fixed << std::setprecision(3);
        outFile << "N,T_Load_ms,T_Save_ms,T_Search_us\n";

        std::random_device rd;
        std::mt19937 gen(rd());

        for (int N : N_values) {
            generateDummyPatientFile(N, dummyPatientFilePath);

            double totalLoadMs = 0.0, totalSaveMs = 0.0, totalSearchUs = 0.0;
            for (int i = 0; i < K; ++i) {
                Database db;

                auto startLoad = std::chrono::high_resolution_clock::now();
                db.loadAllData();
                auto endLoad = std::chrono::high_resolution_clock::now();
                totalLoadMs += std::chrono::duration<double, std::milli>(endLoad - startLoad).count();

                std::uniform_int_distribution<> dis(1, N);
                int randomID = dis(gen);
                std::stringstream ss;
                ss << "BN" << std::setfill('0') << std::setw(5) << randomID;
                std::string searchId = ss.str();

                auto startSearch = std::chrono::high_resolution_clock::now();
                db.findPatientById(searchId);
                auto endSearch = std::chrono::high_resolution_clock::now();
                totalSearchUs += std::chrono::duration<double, std::micro>(endSearch - startSearch).count();

                auto startSave = std::chrono::high_resolution_clock::now();
                db.saveAllData();
                auto endSave = std::chrono::high_resolution_clock::now();
                totalSaveMs += std::chrono::duration<double, std::milli>(endSave - startSave).count();
            }

            outFile << N << "," << (totalLoadMs / K) << ","
                    << (totalSaveMs / K) << "," << (totalSearchUs / K) << "\n";
            qDebug() << "Da test xong moc N =" << N;
        }
        outFile.close();
    } catch (...) {
        fs::current_path(originalCwd);
        std::error_code ec;
        fs::remove_all(benchRoot, ec);
        throw;
    }

    fs::current_path(originalCwd);
    std::error_code ec;
    fs::remove_all(benchRoot, ec);
    qDebug() << "Hoan thanh kiem thu! Thu muc data/ goc duoc bao toan 100%.";
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    installGlobalKeyNavigation(&a);
    
    // GỌI CHẠY TEST TẠI ĐÂY
    //runPerformanceBenchmark();

    a.setQuitOnLastWindowClosed(true);
    a.setStyle(QStyleFactory::create("Fusion"));
    QFile styleFile("resources/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        a.setStyleSheet(stream.readAll());
        styleFile.close();
        qDebug() << "==> DA NAP STYLE.QSS THANH CONG!";
    } else {
        qDebug() << "==> LOI: Khong mo duoc file QSS tai:" << styleFile.fileName();
    }

    Database db;
    db.loadAllData();
    // Giữ một phiên xác thực dùng chung cho Login và các menu.
    AuthService auth(db);

    Login *w = new Login(db, auth);
    w->show();

    int result = a.exec(); 
    db.saveAllData(); 

    return result;
}