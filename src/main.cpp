#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "ui/Login.h"
#include "services/Database.h"
#include "mainwindow.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <sstream>

void runPerformanceBenchmark() {
    qDebug() << "Bat dau kiem thu hieu nang. Vui long doi...";
    std::vector<int> N_values = {100, 500, 1000, 2000, 3000, 5000, 8000, 10000};
    const int K = 50; 
    
    std::ofstream outFile("benchmark_results.csv");
    outFile << "N,T_Load_ms,T_Save_ms,T_Search_us\n";

    std::random_device rd;
    std::mt19937 gen(rd());

    // Khởi tạo đối tượng Database thực tế
    Database db; 

    for (int N : N_values) {
        long long totalLoadTime = 0, totalSaveTime = 0, totalSearchTime = 0;

        for (int i = 0; i < K; ++i) {

            // --- ĐO THỜI GIAN LOAD ---
            auto startLoad = std::chrono::high_resolution_clock::now();
            db.loadAllData(); // Gọi hàm load thực tế từ Database.cpp
            auto endLoad = std::chrono::high_resolution_clock::now();
            totalLoadTime += std::chrono::duration_cast<std::chrono::milliseconds>(endLoad - startLoad).count();

            // --- ĐO THỜI GIAN SEARCH ---
            // Database của bạn định dạng ID là BN00001, cần chuyển đổi số random sang định dạng chuỗi này
            std::uniform_int_distribution<> dis(1, N);
            int randomID = dis(gen);
            
            std::stringstream ss;
            ss << "BN" << std::setfill('0') << std::setw(5) << randomID;
            std::string searchId = ss.str();
            
            auto startSearch = std::chrono::high_resolution_clock::now();
            db.findPatientById(searchId); // Gọi hàm tìm kiếm thực tế
            auto endSearch = std::chrono::high_resolution_clock::now();
            totalSearchTime += std::chrono::duration_cast<std::chrono::microseconds>(endSearch - startSearch).count();

            // --- ĐO THỜI GIAN SAVE ---
            auto startSave = std::chrono::high_resolution_clock::now();
            db.saveAllData(); // Gọi hàm save thực tế
            auto endSave = std::chrono::high_resolution_clock::now();
            totalSaveTime += std::chrono::duration_cast<std::chrono::milliseconds>(endSave - startSave).count();
            
            // Không cần gọi db.clear() vì bên trong db.loadAllData() đã có lệnh patients.clear() tự động dọn dẹp bộ nhớ
        }

        outFile << N << "," << (double)totalLoadTime/K << "," 
                << (double)totalSaveTime/K << "," << (double)totalSearchTime/K << "\n";
        qDebug() << "Da test xong moc N =" << N;
    }
    outFile.close();
    qDebug() << "Hoan thanh! File benchmark_results.csv da duoc sinh ra.";
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    // GỌI HÀM BENCHMARK TẠI ĐÂY (TRƯỚC KHI KHỞI ĐỘNG GIAO DIỆN)
    runPerformanceBenchmark();

    a.setQuitOnLastWindowClosed(false);
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

    Login *w = new Login(db);
    w->show();

    int result = a.exec(); 
    db.saveAllData(); 

    return result;
}