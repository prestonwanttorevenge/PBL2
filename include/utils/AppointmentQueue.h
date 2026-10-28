#ifndef APPOINTMENT_QUEUE_H
#define APPOINTMENT_QUEUE_H

#include "models/Appointment.h"
#include <string>
#include <vector>

inline bool isNextPendingAppointment(const std::vector<Appointment>& appointments,
                                    const Appointment& target) {
    if (target.getStatus() != "Pending") return false;

    for (const Appointment& appointment : appointments) {
        if (appointment.getStatus() != "Pending" ||
            appointment.getDoctorId() != target.getDoctorId() ||
            appointment.getAppointmentDate() != target.getAppointmentDate() ||
            appointment.getTimeSlot() != target.getTimeSlot()) {
            continue;
        }
        return appointment.getAppointmentId() == target.getAppointmentId();
    }

    return false;
}

#endif
