#ifndef CUSTOMER_SERVICE_H
#include <string>
#define CUSTOMER_SERVICE_H
#endif 
#include <future>

struct Transaction {
    int amount;
    short type;
    std::string description;
    std::string createdAt;
};

struct Customer {
    int id;
    int balance;
    int limit;
    std::string createdAt;
    std::string extractAt;
    std::vector<Transaction> transactions;
};

struct TransactionResume {
    int limit;
    int balance;
};


class CustomerService
{
  public:
    void getExtract(short customerId, std::function<void(std::optional<Customer>&)> callback, std::function<void(BusinessException&)> callbackError);
    void addTransaction(short customerId, int amount, char type, std::string description, std::function<void(TransactionResume&)> callback, std::function<void(BusinessException&)> error);
    static short charToTypeShort(char typeChar, std::function<void(BusinessException&)> callbackError);
    static char shortToTypeChar(short type);
};
