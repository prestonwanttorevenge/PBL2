#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <algorithm>
#include <string>
#include <vector>
namespace Constants{
    const std::string USER_FILE = "data/users.csv";
    const std::string DOCTORS_FILE = "data/doctors.csv";
    const std::string PATIENTS_FILE = "data/patients.csv";
    const std::string MEDICINES_FILE = "data/medicines.csv";
    const std::string MEDICAL_RECORDS_FILE = "data/medical_records.csv";
    const std::string INVOICES_FILE = "data/invoices.csv";

    const std::string ROLE_ADMIN = "Admin";
    const std::string ROLE_DOCTOR = "Doctor";
    const std::string ROLE_RECEPTIONIST = "Receptionist";

    inline const std::vector<std::string>& medicalSpecialties() {
        static const std::vector<std::string> specialties = {
            "Khoa Hồi sức cấp cứu",
            "Khoa Nội tổng hợp",
            "Khoa Truyền nhiễm",
            "Khoa Nhi",
            "Khoa Ngoại tổng hợp",
            "Khoa Phụ sản",
            "Liên chuyên khoa (Tai - Mũi - Họng, Răng - Hàm - Mặt, Mắt)",
            "Khoa Xét nghiệm (Huyết học, Hóa sinh, Vi sinh)",
            "Khoa Chẩn đoán hình ảnh",
            "Khoa Giải phẫu bệnh",
            "Khoa Chống nhiễm khuẩn",
            "Khoa Dược",
            "Khoa Dinh dưỡng"
        };
        return specialties;
    }

    inline bool isMedicalSpecialty(const std::string& specialty) {
        const auto& specialties = medicalSpecialties();
        return std::find(specialties.begin(), specialties.end(), specialty) != specialties.end();
    }
}

#endif