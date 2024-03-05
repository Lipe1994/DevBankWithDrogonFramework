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

const std::string ECC = "ECC";   //"conflito de concorrência - Erro ao atualizar saldo do cliente"
const std::string ECT = "ECT";   //"Erro ao confirmar transação"
const std::string ECTC = "ECTC"; //"Erro ao confirmar transação pego no catch"

void CustomerService::getExtract(short customerId, std::function<void(std::optional<Customer> &)> callback, std::function<void(BusinessException &)> callbackError)
{
    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
        "SELECT c.id, c.balance, c.limit, NOW() extract_at, t.amount, t.description, t.type, t.created_at created_at "
        "   FROM customers c "
        "   left join transactions t on t.customer_id = c.id "
        "WHERE c.id = $1 "
        "ORDER BY t.id DESC "
        "LIMIT 10",
        [callback, callbackError](const drogon::orm::Result &result)
        {
            if (result.size() == 0)
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

            for (auto row : result)
            {
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
        [callback](const drogon::orm::DrogonDbException &e)
        {
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
    std::function<void(TransactionResume &)> callback,
    std::function<void(BusinessException &)> callbackError)
{

    if (type == 'd')
    {
        CustomerService::debit(customerId, amount, type, description, callback, callbackError);
    }
    else if (type == 'c')
    {
        CustomerService::credit(customerId, amount, type, description, callback, callbackError);
    }
}

void CustomerService::debit(
    short customerId,
    int amount,
    char type,
    std::string description,
    std::function<void(TransactionResume &)> callback,
    std::function<void(BusinessException &)> callbackError)
{

    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
        "select affectedRow, _limit, _balance from debitar($1, $2, $3)",
        [customerId, amount, type, description, callback, callbackError](const drogon::orm::Result &result)
        {
            auto affectedRow = result[0]["affectedRow"].as<bool>();
            if (!affectedRow)
            {
                auto b = BusinessException(ECC);
                LOG_DEBUG << b.what();
                callbackError(b);
                return;
            }

            TransactionResume resume;
            resume.limit = result[0]["_limit"].as<int>();
            resume.balance = result[0]["_balance"].as<int>();

            callback(resume);
        },
        [callbackError](const drogon::orm::DrogonDbException &e)
        {
            auto b = BusinessException(ECTC);
            // LOG_DEBUG << b.what();
            callbackError(b);
        },
        customerId, amount, description);
}

void CustomerService::credit(
    short customerId,
    int amount,
    char type,
    std::string description,
    std::function<void(TransactionResume &)> callback,
    std::function<void(BusinessException &)> callbackError)
{

    auto clientPtr = drogon::app().getDbClient();

    clientPtr->execSqlAsync(
        "select affectedRow, _limit, _balance from creditar($1, $2, $3)",
        [customerId, amount, type, description, callback, callbackError](const drogon::orm::Result &result)
        {
            auto affectedRow = result[0]["affectedRow"].as<bool>();

            if (!affectedRow)
            {
                auto b = BusinessException(ECC);
                LOG_DEBUG << b.what();
                callbackError(b);
            }

            TransactionResume resume;
            resume.limit = result[0]["_limit"].as<int>();
            resume.balance = result[0]["_balance"].as<int>();
            callback(resume);
        },
        [callbackError](const drogon::orm::DrogonDbException &e)
        {
            auto b = BusinessException(ECTC);
            // LOG_DEBUG << b.what();
            callbackError(b);
        },
        customerId, amount, description);
}