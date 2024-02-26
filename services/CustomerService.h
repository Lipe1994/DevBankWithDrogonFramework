#ifndef CUSTOMER_SERVICE_H
#include <string>
#define CUSTOMER_SERVICE_H
#endif 
#include <future>

struct Transaction {
    int amount;
    char type;
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
    bool affectedRow;
};


class CustomerService
{
  public:
    void getExtract(short customerId, std::function<void(std::optional<Customer>&)> callback, std::function<void(BusinessException&)> callbackError);
    void addTransaction(short customerId, int amount, char type, std::string description, std::function<void(TransactionResume&)> callback, std::function<void(BusinessException&)> error);

    void debit(
        short customerId,
        int amount, 
        char type, 
        std::string description,  
        std::function<void(TransactionResume&)> callback, 
        std::function<void(BusinessException&)> callbackError);

    void credit(
        short customerId,
        int amount, 
        char type, 
        std::string description,  
        std::function<void(TransactionResume&)> callback, 
        std::function<void(BusinessException&)> callbackError);
};
