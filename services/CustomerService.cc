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


void CustomerService::getExtract(short customerId, std::function<void(std::optional<Customer>&)> callback, std::function<void(BusinessException&)> callbackError)
{
    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
"SELECT c.id, c.balance, c.limit, NOW() extract_at, t.amount, t.description, t.type, t.created_at created_at "
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
        customer.extractAt = result[0]["extract_at"].as<std::string>();

        for (auto row : result) {
            customer.transactions.push_back({
                row["amount"].as<int>(),
                row["type"].as<char>(),
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

    if(type == 'd'){
        CustomerService::debit(customerId, amount, type, description, callback, callbackError);
    }else if(type == 'c'){
        CustomerService::credit(customerId, amount, type, description, callback, callbackError);
    }
}

void CustomerService::debit(
    short customerId,
    int amount, 
    char type, 
    std::string description,  
    std::function<void(TransactionResume&)> callback, 
    std::function<void(BusinessException&)> callbackError){
    
    auto clientPtr = drogon::app().getDbClient();
    auto transPtr = clientPtr->newTransaction();

    transPtr->execSqlAsync("WITH Updated as (UPDATE public.customers SET balance = balance - $2 WHERE id = $1 AND ( (balance - $2) < 0 AND (abs(balance - $2) <= \"limit\") OR (balance - $2 >= 0) ) RETURNING \"limit\", balance) SELECT true affectedRow, \"limit\", balance FROM Updated",
        [transPtr, customerId, amount, type, description, callback, callbackError](const drogon::orm::Result &result) {
            
            auto affectedRow = result[0]["affectedRow"];
            if(affectedRow.isNull())
            {
                transPtr->rollback();
                auto b = BusinessException("conflito de concorrência - Erro ao atualizar saldo do cliente");
                LOG_DEBUG<<b.what();
                callbackError(b);
                return;
            }

            transPtr->setCommitCallback([result, callback, callbackError](bool success) {
                if (success) {
                    TransactionResume resume;
                    resume.limit = result[0]["limit"].as<int>();
                    resume.balance = result[0]["balance"].as<int>();

                    callback(resume);
                } else {
                    auto b = BusinessException("Erro ao confirmar transação");
                    LOG_DEBUG<<b.what();
                    callbackError(b);
                }
            }); 

            transPtr->execSqlAsync(
            "INSERT INTO public.transactions (customer_id, amount, description, type, created_at) VALUES ($1, $2, $3, $4, NOW())",
            [](const drogon::orm::Result &result) {

            },
            [transPtr, callbackError](const drogon::orm::DrogonDbException &e) {
                transPtr->rollback();
                auto b = BusinessException(e.base().what());
                LOG_DEBUG<<b.what();
                callbackError(b);
            },
            customerId, abs(amount), description, type);
        },
        [transPtr, callbackError](const drogon::orm::DrogonDbException &e) {
            transPtr->rollback();
            auto b = BusinessException(e.base().what());
            LOG_DEBUG<<b.what();
            callbackError(b);
        },
        customerId, amount);
}

void CustomerService::credit(
    short customerId,
    int amount, 
    char type, 
    std::string description,  
    std::function<void(TransactionResume&)> callback, 
    std::function<void(BusinessException&)> callbackError){
    
    auto clientPtr = drogon::app().getDbClient();
    auto transPtr = clientPtr->newTransaction();

    transPtr->execSqlAsync("WITH Updated as (UPDATE public.customers SET balance = balance + $2 WHERE id = $1 RETURNING \"limit\", balance) SELECT true affectedRow, \"limit\", balance FROM Updated",
        [transPtr, customerId, amount, type, description, callback, callbackError](const drogon::orm::Result &result) {

            auto affectedRow = result[0]["affectedRow"];
            if(affectedRow.isNull())
            {
                transPtr->rollback();
                auto b = BusinessException("conflito de concorrência - Erro ao atualizar saldo do cliente");
                LOG_DEBUG<<b.what();
                callbackError(b);
            }

            transPtr->setCommitCallback([result, callback, callbackError](bool success) {
                if (success) {
                    TransactionResume resume;
                    resume.limit = result[0]["limit"].as<int>();
                    resume.balance = result[0]["balance"].as<int>();
                    callback(resume);
                } else {
                    auto b = BusinessException("Erro ao confirmar transação");
                    LOG_DEBUG<<b.what();
                    callbackError(b);
                }
            });
            
            transPtr->execSqlAsync(
            "INSERT INTO public.transactions (customer_id, amount, description, type, created_at) VALUES ($1, $2, $3, $4, NOW())",
            [](const drogon::orm::Result &result) {
                
            },
            [transPtr, callbackError](const drogon::orm::DrogonDbException &e) {
                transPtr->rollback();
                auto b = BusinessException(e.base().what());
                LOG_DEBUG<<b.what();
                callbackError(b);
            },
            customerId, abs(amount), description, type);
        },
        [transPtr, callbackError](const drogon::orm::DrogonDbException &e) {
            transPtr->rollback();
            auto b = BusinessException(e.base().what());
            LOG_DEBUG<<b.what();
            callbackError(b);
        },
        customerId, amount);
}