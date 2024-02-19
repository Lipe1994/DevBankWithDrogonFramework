#ifndef CUSTOMER_SERVICE_CC
#include "exceptions/BusinessException.cc"
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

short CustomerService::charToTypeShort(char typeChar, std::function<void(BusinessException&)> callbackError) {
    char lowerCaseTypeChar = std::tolower(typeChar);

    if (lowerCaseTypeChar == 'c')
        return CREDITO;
    else if (lowerCaseTypeChar == 'd')
        return DEBITO;
    else{
        auto b = BusinessException("Tipo de transação inválido");
        callbackError(b);
        return -1;
    }
}

char CustomerService::shortToTypeChar(short type) {
    if (type == CREDITO)
        return 'c';
    else if (type == DEBITO)
        return 'd';
    else{
        return ' ';
    }
}

void CustomerService::getExtract(short customerId, std::function<void(std::optional<Customer>&)> callback, std::function<void(BusinessException&)> callbackError)
{
    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
"SELECT c.id, c.balance, c.limit, to_char(c.created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') customer_created_at, to_char(NOW(), 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') extract_at, t.amount, t.description, t.type, to_char(t.created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') created_at "
"   FROM customers c "
"   left join transactions t on t.customer_id = c.id "
"WHERE c.id = $1 "
"ORDER BY t.id DESC "
"LIMIT 10",
    [callback, callbackError](const drogon::orm::Result &result) {
        if(result.size() == 0)
        {
            auto error = BusinessException("Cliente não encontrado");
            callbackError(error);
            return;
        }
        
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
    [callback](const drogon::orm::DrogonDbException &e) {
            std::optional<Customer> emptyCustomer;
            callback(emptyCustomer);
            return;
    },
    customerId);
}


void CustomerService::addTransaction(
    short customerId, 
    int amount, 
    char type, 
    std::string description, 
    std::function<void(TransactionResume&)> callback,
    std::function<void(BusinessException&)> callbackError)
{
    auto transactionType = CustomerService::charToTypeShort(type, callbackError);

    auto clientPtr = drogon::app().getDbClient();
    auto transPtr = clientPtr->newTransaction();

    auto amountPtr = std::make_shared<int>(abs(amount));

    transPtr->execSqlAsync(
        "select id, version, \"limit\", balance, to_char(created_at, 'YYYY-MM-DD\"T\"HH24:MI:SS.US\"Z\"') created_at from public.customers where id = $1",
        [transPtr, customerId, amountPtr, transactionType, description, callback, callbackError](const drogon::orm::Result &result) {

        Customer customer;
        customer.id = result[0]["id"].as<int>();
        customer.limit = result[0]["limit"].as<int>();
        customer.balance = result[0]["balance"].as<int>();
        customer.createdAt = result[0]["created_at"].as<std::string>();
        auto version = result[0]["version"].as<int>();

        int amount = *amountPtr;

        if(transactionType == DEBITO)
        {
            if( abs(customer.balance - amount) > abs(customer.limit))
            {
                auto b = BusinessException("saldo insuficiente");
                LOG_DEBUG<<b.what();
                callbackError(b);
                return;
            }

            customer.balance -= abs(amount);
        }else if(transactionType == CREDITO){
            customer.balance += abs(amount);
        }else{
            auto b = BusinessException("Tipo de transação inválido");
            LOG_DEBUG<<b.what();
            callbackError(b);
            return;
        }

        transPtr->execSqlAsync("UPDATE public.customers SET balance = $1, version=$4 WHERE id = $2 and version=$3",
            [transPtr, customerId, customer, amount, description, transactionType, callback, callbackError](const drogon::orm::Result &result) {

                if(result.affectedRows() < 1)
                {
                    transPtr->rollback();
                    auto b = BusinessException("conflito de concorrência - Erro ao atualizar saldo do cliente");
                    LOG_DEBUG<<b.what();
                    callbackError(b);
                    return;
                }

                transPtr->execSqlAsync(
                "INSERT INTO public.transactions (customer_id, amount, description, type, created_at) VALUES ($1, $2, $3, $4, NOW())",
                [transPtr, customerId, customer, amount, callback, callbackError](const drogon::orm::Result &result) {
                    
                    TransactionResume resume;
                    resume.balance = customer.balance;
                    resume.limit = customer.limit;
                    callback(resume);

                },
                [transPtr, callbackError](const drogon::orm::DrogonDbException &e) {
                    transPtr->rollback();
                    auto b = BusinessException(e.base().what());
                    LOG_DEBUG<<b.what();
                    callbackError(b);
                    return;
                },
                customerId, abs(amount), description, transactionType);
            },
            [transPtr, callbackError](const drogon::orm::DrogonDbException &e) {
                transPtr->rollback();
                auto b = BusinessException(e.base().what());
                LOG_DEBUG<<b.what();
                callbackError(b);
                return;
            },
        customer.balance, customerId, version, (version+1));
    },
    [callbackError](const drogon::orm::DrogonDbException &e) {
        auto b = BusinessException(e.base().what());
        LOG_DEBUG<<b.what();
        callbackError(b);
        return;
    },
    customerId);
}