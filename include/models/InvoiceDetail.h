#ifndef INVOICE_DETAIL_H
#define INVOICE_DETAIL_H
#include <string>

using namespace std;

class InvoiceDetail {
private:
    string detailId;
    string invoiceId;
    string serviceId;
    int quantity;
    double unitPrice;
    double subTotal;

public:
    InvoiceDetail(string dId = "", string sId = "", string iId= "", int qty = 0, double price = 0.0)
        : detailId(dId), serviceId(sId), invoiceId(iId),quantity(qty), unitPrice(price), subTotal(qty * price) {}
    ~InvoiceDetail(){}
    double getSubTotal() const { return subTotal; }
    string getServiceId() const { return serviceId; }
};
#endif