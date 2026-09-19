#ifndef INVOICE_DETAIL_H
#define INVOICE_DETAIL_H
#include <string>

using namespace std;

class InvoiceDetail{
private:
    string detailId;
    string serviceId;
    int quantity;
    double unitPrice;
    double subTotal;

public:
    InvoiceDetail(string dId, string sId, int qty, double price)
        : detailId(dId), serviceId(sId), quantity(qty), unitPrice(price) {
        subTotal = quantity * unitPrice;
    }
    //Bỏ được thì bỏ InvoiceDetail(InvoiceDetail&);
    ~InvoiceDetail(){}
    double getSubTotal() const { return subTotal; }
};
#endif