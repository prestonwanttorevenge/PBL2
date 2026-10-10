#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include <string>
#include "Doctor.h"

using namespace std;

class Appointment {
private:
    string appointmentId;
    string patientId;      
    string doctorId;     
    string receptionistId; 
    string day;
    string time;
    string status;         
public:
    Appointment(string aId = "-1", string pId = "-1", string dId = "-1",
                string rId = "-1", string day = "", string time ="" ,string stat = "Pending")
        : appointmentId(aId), patientId(pId), doctorId(dId),
          receptionistId(rId), day(day) , time(time) , status(stat) {}
    ~Appointment(){}
    string getAppointmentId()const {return appointmentId;}
    string getPatientId()const {return patientId;}
    string getDoctorId() const{return doctorId; }
    string getReceptionistId ()const{return receptionistId;}
    string getDayOfWeek() const { return day; }
    string getTimeSlot() const { return time; }
    string getStatus() const{return status; }
    void setDayOfWeek(string day) { this->day = day; }
    void setTimeSlot(string time) { this->time = time; }
    void setStatus(string st) { this->status = st; }
};
#endif