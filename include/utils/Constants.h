#ifndef CONSTANTS_H
#define CONSTANTS_H
#include <string>
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
}

#endif