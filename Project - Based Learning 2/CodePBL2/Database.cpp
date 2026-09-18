#include "Database.h"

Database::~Database() {
    for (int i = 0; i < patients.size(); i++) {
        delete patients[i];
    }
    patients.clear();
}