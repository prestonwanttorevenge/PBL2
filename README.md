# Hospital Patient Management System — Bệnh viện Long Phát (PBL2)

Hệ thống quản lý bệnh nhân của bệnh viện (mô phỏng quy trình vận hành tại Bệnh viện Long Phát) được xây dựng bằng ngôn ngữ **C++17** kết hợp giao diện đồ họa **Qt Widgets**, theo hướng **Lập trình hướng đối tượng (OOP)** và lưu trữ dữ liệu qua tệp **CSV** phân tách bằng dấu chấm phẩy (`;`).

## Tính năng chính
- **Quản lý tài khoản & Phân quyền (RBAC)**: Đăng nhập với 4 vai trò (`Admin`, `Doctor`, `Receptionist`, `Inventory`), mật khẩu băm Bcrypt sinh muối bằng `BCryptGenRandom` trên Windows.
- **Quản lý Bệnh nhân**: Phân loại bệnh nhân Nội trú (`Inpatient`) và Ngoại trú (`Outpatient`), xếp phòng/giường trống và xuất viện.
- **Quản lý Bác sĩ & Lịch hẹn**: Quản lý hồ sơ bác sĩ, lịch trực theo thứ/ca, đặt lịch và tiếp nhận hàng đợi theo thứ tự đến.
- **Hồ sơ bệnh án & Kê đơn thuốc**: Ghi nhận triệu chứng, chẩn đoán, chỉ định cận lâm sàng và kê đơn theo tồn kho khả dụng (giữ chỗ ảo).
- **Kho thuốc & Cấp phát**: Tìm kiếm thuốc thời gian thực, nhập kho và xác nhận cấp phát trừ tồn kho.
- **Hóa đơn & BHYT**: Tính viện phí áp dụng quy tắc đồng chi trả BHYT theo tiền tố mã thuốc/dịch vụ, xem và xuất PDF hóa đơn.
- **Lưu trữ dữ liệu**: Đọc/ghi dữ liệu từ 11 tệp `.csv` trong `data/` qua tệp tạm `.tmp` với định dạng số nguyên cố định (`std::fixed`).

## Quy trình nghiệp vụ hiện có

1. **Lễ tân** tạo hồ sơ bệnh nhân với họ tên, ngày sinh, giới tính, số điện thoại và thông tin BHYT; chọn bác sĩ theo chuyên khoa, ngày làm việc và ca trực; đặt lịch theo ngày cụ thể.
2. Lịch mới ở trạng thái `Scheduled`. Lễ tân xác nhận bệnh nhân đến đúng ngày hẹn để chuyển lịch sang `Pending`. Bác sĩ tiếp nhận lần lượt theo thứ tự đặt lịch; lượt trước chuyển sang `Waiting` hoặc `Completed` thì lượt `Pending` tiếp theo mới vào hàng khám.
3. **Bác sĩ** chỉ xem lượt đang khám kế tiếp và các hồ sơ đang chờ CLS hoặc chờ cấp thuốc của mình. Bác sĩ ghi triệu chứng/chẩn đoán, chỉ định dịch vụ CLS cần thiết rồi chuyển sang `Waiting`, hoặc kê đơn chuyển sang `WaitingInventory`. Nếu bệnh nhân không nhận phần thuốc còn lại, bác sĩ có thể hủy các dòng chưa cấp từ `DoctorMenu`.
4. **Kho thuốc (`Inventory`)** xem hàng đợi đơn chưa cấp, xác nhận cấp phát rồi mới trừ tồn kho. Lượt chỉ chuyển sang `Completed` khi mọi thuốc trong đơn đã cấp.
5. **Lễ tân** lập hóa đơn cho lượt khám `Completed` (1-1 với `appointmentId`) và xem/xuất PDF qua `InvoiceDialog`.
6. **Admin** quản lý tài khoản, khóa/mở tài khoản, bác sĩ (kèm ca trực), thuốc, dịch vụ và xem thống kê/biểu đồ doanh thu.

## Giao diện và kiểm thử

- Tên đăng nhập mới chỉ chấp nhận 3–32 ký tự ASCII không dấu: chữ Latin, chữ số, dấu chấm, gạch dưới hoặc gạch nối.
- Màn hình doanh thu hiển thị tổng tiền, số hồ sơ bệnh nhân, cơ cấu hóa đơn theo mức thu và biểu đồ 8 hóa đơn gần nhất.
- Chạy kiểm thử tự động (bao gồm kiểm tra 5 bất biến Đỏ-Đen, fuzz test ngẫu nhiên và kiểm thử luồng nghiệp vụ) bằng:
  ```bash
  ctest --test-dir <build-directory> --output-on-failure
  ```
- Chạy phép đo vi mô cây Đỏ-Đen (`tests/rbtree_bench.cpp`) qua target `PBL2_RBTreeBench`.

> Đây là đồ án quản lý mô phỏng, không phải hồ sơ bệnh án điện tử dùng trong điều trị thực tế. Danh mục/chi trả BHYT là quy tắc minh họa; dự án chưa có xác thực danh tính bệnh nhân, ký số, nhật ký kiểm toán, thanh toán thực tế hay tích hợp hệ thống y tế.

## Cấu trúc thư mục dự án
```text
PBL2/
├── data/                # 11 tệp dữ liệu CSV (;)
├── include/             # Các tệp Header (.h)
│   ├── models/          # Thực thể miền (Patient, Doctor, Invoice, User, bcrypt...)
│   ├── services/        # Database, AuthService
│   ├── ui/              # Giao diện Qt Widgets (Login, AdminMenu, DoctorMenu, ReceptionistMenu, InventoryMenu, InvoiceDialog, SearchableComboBox...)
│   └── utils/           # RBTree, CustomMap, AppointmentQueue, Constants, Helper
├── resources/           # QSS styling
├── src/                 # Các tệp mã nguồn (.cpp) và .ui tương ứng
├── tests/               # algorithm_smoke.cpp, workflow_smoke.cpp, rbtree_bench.cpp
├── CMakeLists.txt       # Cấu hình build CMake + CTest
└── README.md            # Tài liệu giới thiệu dự án
```
