#include <future>
#include <string>

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
    void get(short customerId, std::function<void(std::optional<Customer>&)> callback);
    void getAll(std::function<void(std::vector<Customer>&)> callback);
    void getExtract(short customerId, std::function<void(std::optional<Customer>&)> callback);
    void addTransaction(short customerId, int amount, char type, std::string description, std::function<void(std::optional<TransactionResume>&)> callback);
    static short charToTypeShort(char typeChar);
    static char shortToTypeChar(short type);
};
