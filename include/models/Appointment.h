#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include <iostream>
#include <string>
class Appointment {
private:
    std::string appointmentId;
    std::string patientId;
    std::string doctorId;
    std::string receptionistId;
    std::string specialty;
    std::string day;
    std::string time;
    // Ngày cụ thể tách riêng thứ trong tuần và ca trực.
    std::string appointmentDate;
    std::string status;

public:
    Appointment(const std::string& aId = "-1",
                const std::string& pId = "-1",
                const std::string& dId = "-1",
                const std::string& rId = "-1",
                const std::string& spec = "-1",
                const std::string& day = "",
                const std::string& time = "",
                const std::string& stat = "Pending",
                const std::string& date = "")
        : appointmentId(aId), patientId(pId), doctorId(dId),
        receptionistId(rId), specialty(spec), day(day), time(time),
        appointmentDate(date), status(stat) {}

    ~Appointment() {}

    // Getters tiêu chuẩn
    std::string getAppointmentId() const { return appointmentId; }
    std::string getPatientId() const { return patientId; }
    std::string getDoctorId() const { return doctorId; }
    std::string getReceptionistId() const { return receptionistId; }
    std::string getDayOfWeek() const { return day; }
    std::string getTimeSlot() const { return time; }
    std::string getStatus() const { return status; }
    std::string getSpecialty() const { return specialty; }

    // Bổ sung hàm khớp với giao diện DoctorMenu.cpp
    std::string getAppointmentDate() const {
        // Dòng CSV cũ chưa có ngày cụ thể nên dùng lại giá trị day.
        if (appointmentDate.empty()) return day;
        return appointmentDate;
    }
    std::string getDate() const { return appointmentDate; }

    // Setters
    void setDayOfWeek(const std::string& day) { this->day = day; }
    void setTimeSlot(const std::string& time) { this->time = time; }
    void setStatus(const std::string& st) { this->status = st; }
    void setAppointmentDate(const std::string& date) { appointmentDate = date; }
};

#endif // APPOINTMENT_H