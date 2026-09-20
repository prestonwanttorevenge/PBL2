#ifndef SERVICE_H
#define SERVICE_H
#include <string>

using namespace std;

class Service {
private:
    string serviceId;
    string serviceName;
    double price;
public:
    Service(string id = "-1", string name = "", double p = 0.0)
        : serviceId(id), serviceName(name), price(p){}
    //Bỏ được thì bỏ Service(Service&);
    ~Service(){}
    string getId() const { return serviceId; }
    string getServiceName() const {return serviceName;}
    double getPrice() const { return price; }
};
#endif