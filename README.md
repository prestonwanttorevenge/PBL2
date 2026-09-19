# Hospital Management System (PBL2)

Hệ thống quản lý bệnh viện được xây dựng bằng ngôn ngữ **C++** theo hướng **Lập trình hướng đối tượng (OOP)** và lưu trữ dữ liệu thông qua file **CSV**.

## 🚀 Tính năng chính
- **Quản lý tài khoản & Phân quyền**: Đăng nhập theo phân quyền (Admin, Doctor, Receptionist, Cashier).
- **Quản lý Bệnh nhân**: Hỗ trợ phân loại bệnh nhân Nội trú (Inpatient) và Ngoại trú (Outpatient).
- **Quản lý Bác sĩ & Lịch hẹn**: Theo dõi lịch khám, phân công bác sĩ.
- **Hồ sơ bệnh án & Kê đơn thuốc**: Quản lý triệu chứng, chẩn đoán và danh mục thuốc.
- **Hóa đơn & Dịch vụ**: Tính toán chi phí khám chữa bệnh và dịch vụ y tế.
- **Lưu trữ dữ liệu**: Đọc/ghi dữ liệu tự động với các file `.csv` tại thư mục `data/`.

## 📂 Cấu trúc thư mục dự án
```text
HospitalManagementSystem/
│
├── bin/                 # Thư mục chứa file thực thi sau khi build
├── data/                # Chứa các file dữ liệu CSV (.csv)
├── include/             # Chứa các file Header (.h)
│   ├── models/          # Các lớp đối tượng (Patient, Doctor, Invoice, v.v.)
│   ├── services/        # Các dịch vụ xử lý (Database, AuthService)
│   ├── ui/              # Giao diện dòng lệnh (Menu các vai trò)
│   └── utils/           # Các tiện ích chung (Constants, Helper)
│
├── src/                 # Chứa các file mã nguồn (.cpp) tương ứng
├── CMakeLists.txt       # File cấu hình build dự án bằng CMake
└── README.md            # Tài liệu giới thiệu dự án