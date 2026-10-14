#ifndef INVOICE_DETAIL_H
#define INVOICE_DETAIL_H
#include <string>

class InvoiceDetail {
private:
    std::string detailId;
    std::string invoiceId;
    std::string serviceId;
    int quantity;
    double unitPrice;
    double subTotal;

public:
    InvoiceDetail(std::string dId = "", std::string sId ="", std::string iId="", int qty = 0, double price = 0.0)
        : detailId(dId), serviceId(sId), invoiceId(iId) , quantity(qty), unitPrice(price) {
        subTotal = quantity * unitPrice;
    }

    // Yêu cầu C++ sinh hàm copy constructor chuẩn xác
    InvoiceDetail(const InvoiceDetail&) = default;

    ~InvoiceDetail(){}

    std::string getDetailId() const { return detailId; }
    std::string getServiceId() const { return serviceId; }
    int getQuantity() const { return quantity; }
    double getUnitPrice() const { return unitPrice; }
    double getSubTotal() const { return subTotal; }
};
#endif