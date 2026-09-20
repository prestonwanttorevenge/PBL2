#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include <string>

using namespace std;

class Appointment {
private:
    string appointmentId;
    string patientId;      
    string doctorId;     
    string receptionistId; 
    string appointmentDate;
    string status;         
public:
    Appointment(string aId = "-1", string pId = "-1", string dId = "-1",
                string rId = "-1", string date = "", string stat = "Pending")
        : appointmentId(aId), patientId(pId), doctorId(dId),
          receptionistId(rId), appointmentDate(date), status(stat) {}
    ~Appointment(){}
    string getAppointmentId()const {return appointmentId;}
    string getPatientId()const {return patientId;}
    string getDoctorId() const{return doctorId; }
    string getReceptionistId ()const{return receptionistId;}
    string getAppointmentDate ()const{return appointmentDate;}
    string getStatus() const{return status; }
    void setStatus(string newStatus){status = newStatus; }
};
#endif