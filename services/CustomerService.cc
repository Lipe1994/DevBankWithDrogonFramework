#ifndef CUSTOMER_SERVICE_CC
#define CUSTOMER_SERVICE_CC
#endif 

#include "CustomerService.h"
#include <vector>
#include <string>
#include <drogon/HttpAppFramework.h>
#include <drogon/orm/DbClient.h>
#include <stdexcept>

#define CREDITO 0
#define DEBITO 1

short CustomerService::charToTypeShort(char typeChar) {
    char lowerCaseTypeChar = std::tolower(typeChar);

    if (lowerCaseTypeChar == 'c')
        return CREDITO;
    else if (lowerCaseTypeChar == 'd')
        return DEBITO;
    else
        throw std::invalid_argument("Tipo de transação inválido");
}

char CustomerService::shortToTypeChar(short type) {
    if (type == CREDITO)
        return 'c';
    else if (type == DEBITO)
        return 'd';
    else
        throw std::invalid_argument("Tipo de transação inválido");
}


void CustomerService::get(short customerId, std::function<void(std::optional<Customer>&)> callback)
{
    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
        "select id, \"limit\", balance, to_char(created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') created_at from public.customers where id = $1",
        [callback](const drogon::orm::Result &result) {
            
            if(result.size() == 0)
            {
                std::optional<Customer> optionalCustomer;
                callback(optionalCustomer);
            }

            Customer customer;


            customer.id = result[0]["id"].as<int>();
            customer.limit = result[0]["limit"].as<int>();
            customer.balance = result[0]["balance"].as<int>();
            customer.createdAt = result[0]["created_at"].as<std::string>();

            std::optional<Customer> optionalCustomer(customer);
            callback(optionalCustomer);
        },
        [](const drogon::orm::DrogonDbException &e) {
            throw std::invalid_argument("Erro ao buscar cliente");
        },
        customerId);
}

void CustomerService::getAll(std::function<void(std::vector<Customer>&)> callback)
{
    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync("select id, \"limit\", balance, to_char(created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') created_at from public.customers",
        [callback](const drogon::orm::Result &result) {

            std::vector<Customer> customers;

            for (auto row : result) {
                customers.push_back({
                    row["id"].as<int>(),
                    row["limit"].as<int>(),
                    row["balance"].as<int>(),
                    row["created_at"].as<std::string>(),
                });
            }

            callback(customers);
        },
        [](const drogon::orm::DrogonDbException &e) {
            throw std::invalid_argument("Erro ao buscar clientes");
        });
}

void CustomerService::getExtract(short customerId, std::function<void(std::optional<Customer>&)> callback)
{
    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
"SELECT c.id, c.balance, c.limit, to_char(c.created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') customer_created_at, to_char(NOW(), 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') extract_at, t.amount, t.description, t.type, to_char(t.created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') created_at "
"   FROM customers c "
"   left join transactions t on t.customer_id = c.id "
"WHERE c.id = $1 "
"ORDER BY t.id DESC "
"LIMIT 10",
    [callback](const drogon::orm::Result &result) {
        if(result.size() == 0)
        {
            std::optional<Customer> emptyCustomer;
            callback(emptyCustomer);
            return;
        }

        std::cout << result.size() << " rows selected!" << std::endl;
        
        Customer customer;

        customer.id = result[0]["id"].as<int>();
        customer.limit = result[0]["limit"].as<int>();
        customer.balance = result[0]["balance"].as<int>();
        customer.createdAt = result[0]["customer_created_at"].as<std::string>();
        customer.extractAt = result[0]["extract_at"].as<std::string>();

        for (auto row : result) {
            customer.transactions.push_back({
                row["amount"].as<int>(),
                row["type"].as<short>(),
                row["description"].as<std::string>(),
                row["created_at"].as<std::string>(),
            });
        }

        std::optional<Customer> optionalCustomer(customer);
        callback(optionalCustomer);
    },
    [](const drogon::orm::DrogonDbException &e) {
        throw std::invalid_argument("Erro ao buscar extrato");
    },
    customerId);
}

void CustomerService::addTransaction(short customerId, int amount, char type, std::string description, std::function<void(std::optional<TransactionResume>&)> callback)
{
    auto transactionType = CustomerService::charToTypeShort(type);

    auto amountPtr = std::make_shared<int>(abs(amount));
    CustomerService::get(customerId, [customerId, amountPtr, transactionType, description, callback](std::optional<Customer>& customer) {
        if(customer.has_value() == false)
        {
            std::optional<TransactionResume> emptyCustomer;
            callback(emptyCustomer);
            return;
        }

        int amount = *amountPtr;

        if(transactionType == DEBITO)
        {
            if( abs(customer->balance - amount) > abs(customer->limit))
            {
                throw std::invalid_argument("Saldo insuficiente");
            }

            customer->balance -= abs(amount);
        }else{
            customer->balance += abs(amount);
        }

        auto clientPtr = drogon::app().getDbClient();
        auto transPtr = clientPtr->newTransaction();

        transPtr->execSqlAsync(
            "INSERT INTO public.transactions (customer_id, amount, description, type, created_at) VALUES ($1, $2, $3, $4, NOW())",
            [transPtr, customerId, amount, callback](const drogon::orm::Result &result) {
                
                transPtr->execSqlAsync("UPDATE public.customers SET balance = balance + $1 WHERE id = $2 RETURNING id, balance, \"limit\"",
                    [transPtr, callback](const drogon::orm::Result &result) {
                        
                        if(result.size() == 0)
                        {
                            throw std::invalid_argument("Erro ao atualizar saldo");
                            return;
                        }

                        TransactionResume resume;
                        resume.balance= result[0]["balance"].as<int>();
                        resume.limit = result[0]["limit"].as<int>();

                        std::optional<TransactionResume> optionalResume(resume);
                        callback(optionalResume);
                    },
                    [](const drogon::orm::DrogonDbException &e) {},
                    amount, customerId);

            },
            [](const drogon::orm::DrogonDbException &e) {
                throw std::invalid_argument(e.base().what());
            },
            customerId, abs(amount), description.substr(0,10), transactionType);

    });
}