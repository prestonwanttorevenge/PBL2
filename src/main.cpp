#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "ui/Login.h"
#include "services/Database.h"

// Thư viện phục vụ Benchmark
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <fstream>
#include <iomanip>
#include <sstream>

// Hàm tự động sinh N dòng bệnh nhân giả mạo vào file
void generateDummyPatientFile(int N, const QString& filePath) {
    std::ofstream f(filePath.toStdString()); 
    for (int i = 1; i <= N; ++i) {
        f << "NgoaiTru;BN" << std::setfill('0') << std::setw(5) << i 
          << ";Benh Nhan Test " << i 
          << ";01/01/2000;Nam;0901234567;;;01/01/2026;P101\n";
    }
    f.close();
}

void runPerformanceBenchmark() {
    qDebug() << "Bat dau kiem thu hieu nang. Vui long doi...";
    
    // Tên file theo thiết lập lưu trữ hiện tại của đồ án
    QString realPatientFilePath = "data/patient.csv";
    QString backupFilePath = "data/patient_backup.csv";

    // ==========================================
    // BƯỚC 1: SAO LƯU BẢO VỆ DỮ LIỆU THẬT
    // ==========================================
    if (QFile::exists(backupFilePath)) {
        QFile::remove(backupFilePath); // Xóa bản backup cũ nếu bị kẹt từ lần test trước
    }
    if (QFile::exists(realPatientFilePath)) {
        QFile::rename(realPatientFilePath, backupFilePath); // Đổi tên file thật thành file backup
    }

    std::vector<int> N_values;
    int startN = 100;
    int endN = 10000;
    int numPoints = 50;
    
    // Tính bước nhảy (Step = (10000 - 100) / 49 = 202.04)
    double step = (double)(endN - startN) / (numPoints - 1);
    
    for (int i = 0; i < numPoints; ++i) {
        N_values.push_back(startN + (int)(i * step));
    }
    
    const int K = 50; // Giữ nguyên số lần lặp K = 50 để lấy trung bình cộng
    std::ofstream outFile("benchmark_results.csv");
    outFile << "N,T_Load_ms,T_Save_ms,T_Search_us\n";

    std::random_device rd;
    std::mt19937 gen(rd());

    for (int N : N_values) {
        // ==========================================
        // BƯỚC 2: TẠO FILE DỮ LIỆU ẢO THEO MỐC N
        // ==========================================
        generateDummyPatientFile(N, realPatientFilePath);

        long long totalLoadTime = 0, totalSaveTime = 0, totalSearchTime = 0;

        for (int i = 0; i < K; ++i) {
            Database db; 

            // --- 2.1: ĐO THỜI GIAN LOAD FILE ---
            auto startLoad = std::chrono::high_resolution_clock::now();
            db.loadAllData(); // Gọi hàm đọc file thực tế
            auto endLoad = std::chrono::high_resolution_clock::now();
            totalLoadTime += std::chrono::duration_cast<std::chrono::milliseconds>(endLoad - startLoad).count();

            // --- 2.2: ĐO THỜI GIAN SEARCH TRÊN CÂY BST ---
            std::uniform_int_distribution<> dis(1, N);
            int randomID = dis(gen);
            std::stringstream ss;
            ss << "BN" << std::setfill('0') << std::setw(5) << randomID;
            std::string searchId = ss.str();
            
            auto startSearch = std::chrono::high_resolution_clock::now();
            db.findPatientById(searchId); // Gọi hàm tìm kiếm thực tế
            auto endSearch = std::chrono::high_resolution_clock::now();
            totalSearchTime += std::chrono::duration_cast<std::chrono::microseconds>(endSearch - startSearch).count();

            // --- 2.3: ĐO THỜI GIAN SAVE FILE ---
            auto startSave = std::chrono::high_resolution_clock::now();
            db.saveAllData(); // Gọi hàm lưu file thực tế
            auto endSave = std::chrono::high_resolution_clock::now();
            totalSaveTime += std::chrono::duration_cast<std::chrono::milliseconds>(endSave - startSave).count();
        }

        outFile << N << "," << (double)totalLoadTime/K << "," 
                << (double)totalSaveTime/K << "," << (double)totalSearchTime/K << "\n";
        qDebug() << "Da test xong moc N =" << N;
    }
    
    outFile.close();

    // ==========================================
    // BƯỚC 3: KHÔI PHỤC LẠI DỮ LIỆU THẬT
    // ==========================================
    QFile::remove(realPatientFilePath); // Xóa bỏ file bệnh nhân ảo 10.000 dòng
    QFile::rename(backupFilePath, realPatientFilePath); // Trả file 54 bệnh nhân về chỗ cũ

    qDebug() << "Hoan thanh kiem thu! Du lieu 54 benh nhan cua ban da duoc tra lai an toan.";
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    
    // GỌI CHẠY TEST TẠI ĐÂY
    //runPerformanceBenchmark();

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
    // Giữ một phiên xác thực dùng chung cho Login và các menu.
    AuthService auth(db);

    Login *w = new Login(db, auth);
    w->show();

    int result = a.exec(); 
    db.saveAllData(); 

    return result;
}