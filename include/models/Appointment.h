#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>

class Appointment {
private:
    std::string appointmentId;
    std::string patientId;
    std::string doctorId;
    std::string receptionistId;
    std::string day;
    std::string time;
    std::string status;

public:
    Appointment(const std::string& aId = "-1",
                const std::string& pId = "-1",
                const std::string& dId = "-1",
                const std::string& rId = "-1",
                const std::string& day = "",
                const std::string& time = "",
                const std::string& stat = "Pending")
        : appointmentId(aId), patientId(pId), doctorId(dId),
        receptionistId(rId), day(day), time(time), status(stat) {}

    ~Appointment() {}

    // Getters tiêu chuẩn
    std::string getAppointmentId() const { return appointmentId; }
    std::string getPatientId() const { return patientId; }
    std::string getDoctorId() const { return doctorId; }
    std::string getReceptionistId() const { return receptionistId; }
    std::string getDayOfWeek() const { return day; }
    std::string getTimeSlot() const { return time; }
    std::string getStatus() const { return status; }

    // Bổ sung hàm khớp với giao diện DoctorMenu.cpp
    std::string getAppointmentDate() const {
        if (time.empty()) return day;
        return day + " " + time;
    }
    std::string getDate() const { return day; }

    // Setters
    void setDayOfWeek(const std::string& day) { this->day = day; }
    void setTimeSlot(const std::string& time) { this->time = time; }
    void setStatus(const std::string& st) { this->status = st; }
};

#endif // APPOINTMENT_H