# Hospital Management System (PBL2)

Hệ thống quản lý bệnh viện được xây dựng bằng ngôn ngữ **C++** theo hướng **Lập trình hướng đối tượng (OOP)** và lưu trữ dữ liệu thông qua file **CSV**.

## 🚀 Tính năng chính
- **Quản lý tài khoản & Phân quyền**: Đăng nhập theo phân quyền (Admin, Doctor, Receptionist, Cashier).
- **Quản lý Bệnh nhân**: Hỗ trợ phân loại bệnh nhân Nội trú (Inpatient) và Ngoại trú (Outpatient).
- **Quản lý Bác sĩ & Lịch hẹn**: Theo dõi lịch khám, phân công bác sĩ.
- **Hồ sơ bệnh án & Kê đơn thuốc**: Quản lý triệu chứng, chẩn đoán và danh mục thuốc.
- **Hóa đơn & Dịch vụ**: Tính toán chi phí khám chữa bệnh và dịch vụ y tế.
- **Lưu trữ dữ liệu**: Đọc/ghi dữ liệu tự động với các file `.csv` tại thư mục `data/`.

## Quy trình nghiệp vụ hiện có

1. **Lễ tân** tạo hồ sơ bệnh nhân với họ tên, ngày sinh, giới tính, số điện thoại và thông tin BHYT; chọn bác sĩ theo chuyên khoa, ngày làm việc và ca trực; đặt lịch theo ngày cụ thể.
2. Lịch mới ở trạng thái `Scheduled`. Nhiều bệnh nhân có thể đặt cùng bác sĩ/ngày/ca; lễ tân xác nhận bệnh nhân đến đúng ngày hẹn để chuyển lịch sang `Pending`. Bác sĩ tiếp nhận lần lượt theo thứ tự đặt lịch; lượt trước chuyển sang `Waiting` hoặc `Completed` thì lượt `Pending` tiếp theo mới vào hàng khám. Hủy lịch không xóa lịch sử.
3. **Bác sĩ** chỉ xem lượt đang khám kế tiếp và các hồ sơ đang chờ CLS của mình. Bác sĩ ghi nhận chẩn đoán, thêm các dịch vụ CLS cần thiết rồi lưu hồ sơ sang `Waiting`; khi hoàn tất khám/CLS, lịch chuyển sang `Completed`.
4. Bác sĩ kê đơn trên hồ sơ đã hoàn tất. **Kho thuốc** xem hàng đợi đơn chưa cấp, xác nhận cấp phát rồi mới trừ tồn kho. Đơn đã cấp phát không thể cấp lần nữa.
5. **Lễ tân** chỉ lập hóa đơn cho lượt khám `Completed`, sau khi thuốc trong đơn đã được cấp phát. Lượt khám chuyển sang `Billed` và không thể lập hóa đơn trùng.
6. **Admin** quản lý tài khoản, khóa/mở tài khoản, bác sĩ (kèm ít nhất một ca trực), thuốc, dịch vụ và tổng tiền hóa đơn đã lập.

Các file CSV bổ sung trường ngày hẹn, mã lượt khám trên hóa đơn, giá tại thời điểm kê và trạng thái cấp phát thuốc. Bộ đọc vẫn chấp nhận các dòng CSV cũ thiếu các trường mới.

## Giao diện và kiểm thử

- Tên đăng nhập mới chỉ chấp nhận 3–32 ký tự ASCII không dấu: chữ Latin, chữ số, dấu chấm, gạch dưới hoặc gạch nối.
- Các màn hình được chia theo nhóm công việc để giảm số thao tác hiển thị cùng lúc; lịch hẹn dùng lịch dạng lưới và hiển thị ngày `dd/MM/yyyy`.
- Màn hình doanh thu hiển thị tổng tiền, số hồ sơ và biểu đồ giá trị 12 hóa đơn gần nhất. Biểu đồ theo thứ tự hóa đơn vì dữ liệu hóa đơn hiện chưa lưu ngày lập.
- Kiểm thử cấu trúc cây đỏ-đen, cây phòng/giường và bộ đếm bằng `ctest --test-dir <build-directory> --output-on-failure`.

> Đây là đồ án quản lý mô phỏng, không phải hồ sơ bệnh án điện tử dùng trong điều trị thực tế. Danh mục/chi trả BHYT là quy tắc minh họa; dự án chưa có xác thực danh tính bệnh nhân, ký số, nhật ký kiểm toán, thanh toán thực tế, quản lý lô/hạn dùng thuốc hoặc tích hợp hệ thống y tế.

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