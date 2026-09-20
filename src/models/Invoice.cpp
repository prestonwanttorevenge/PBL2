  #include"Invoice.h"
  #include<iostream>

  void Invoice::setMedicineFee(double fee){
        medicineFee = fee;
        totalAmount = consultationFee + medicineFee;
}
  void Invoice::printInvoice() const {
        cout << "--- HOA DON: " << invoiceId << " | BN: " << patientId << " ---" << endl;
        for (const auto& d : details) {
            cout << "DV: " << d.getServiceId() << " | Tien: " << d.getSubTotal() << endl;
        }
        cout << "Phi DV: " << consultationFee << " | Phi Thuoc: " << medicineFee << endl;
        cout << "TONG THANH TOAN: " << totalAmount << endl;
}