#ifndef CUSTOMER_CONTROLLER_CC
#define CUSTOMER_CONTROLLER_CC
    #include <ctime>
    #include "utils/time.cc"
    #include <vector>
#endif 
#include "CustomerController.h"
#include "services/CustomerService.cc"


void CustomerController::get(const HttpRequestPtr &req,
                 std::function<void (const HttpResponsePtr &)> &&callback,
                 short customerId) const
{
    CustomerService customerService;
    customerService.get(customerId, [callback](std::optional<Customer>& customer) {
        if (!customer.has_value()) {
            auto resp = HttpResponse::newNotFoundResponse();
            callback(resp);
            return;
        }

        Json::Value jresp;
        jresp["id"] = customer.value().id;
        jresp["limite"] = customer.value().limit;
        jresp["total"] = customer.value().balance;
        jresp["registrato_em"] = customer.value().createdAt;


        auto resp = HttpResponse::newHttpJsonResponse(jresp);
        callback(resp);
    });
}

void CustomerController::getAll(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback) const
{
    CustomerService customerService;
    customerService.getAll([callback](std::vector<Customer> customers){
        Json::Value jresp;

        for (const auto& customer : customers) {
            Json::Value customerJson;
            customerJson["id"] = customer.id;
            customerJson["limite"] = customer.limit;
            customerJson["total"] = customer.balance;
            customerJson["registrado_em"] = customer.createdAt;
            jresp.append(customerJson);
        }

        auto resp = HttpResponse::newHttpJsonResponse(jresp);
        callback(resp);
    });
}

    void CustomerController::getExtract(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, short customerId) const
    {
        CustomerService customerService;
        customerService.getExtract(customerId, [callback](std::optional<Customer> customer){

        if (!customer.has_value()) {
            auto resp = HttpResponse::newNotFoundResponse();
            callback(resp);
            return;
        }

        Json::Value customerJson;

        customerJson["saldo"]["total"] = customer.value().balance;
        customerJson["saldo"]["limit"] = customer.value().limit;
        customerJson["saldo"]["data_extrato"] = customer.value().extractAt;

        
        for (const auto& customer : customer.value().transactions) {
            std::string tipo; 
            tipo.push_back(CustomerService::shortToTypeChar(customer.type));

            Json::Value transactionJson;
            transactionJson["valor"] = customer.amount;
            transactionJson["tipo"] = tipo;
            transactionJson["descricao"] = customer.description;
            transactionJson["realizada_em"] = customer.createdAt;

            customerJson["ultimas_transacoes"].append(transactionJson);
        }

        auto resp = HttpResponse::newHttpJsonResponse(customerJson);
        callback(resp);
    });

}

void CustomerController::addTransaction(const drogon::HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, short customerId)const
{
    CustomerService customerService;
    customerService.get(customerId, [customerId, req, callback, &customerService](std::optional<Customer>& customer) {

        if (!customer.has_value()) {
            auto resp = HttpResponse::newNotFoundResponse();
            callback(resp);
            return;
        }
        auto json(req->getJsonObject());

        if(json == nullptr)
        {
            return;
        }

        int amount = (*json)["valor"].asInt();
        char type = (*json)["tipo"].asString()[0];
        std::string description = (*json)["descricao"].asString();
        
        customerService.addTransaction(customerId, amount, type, description, [callback](std::optional<TransactionResume>& transactionResume) {
    
            Json::Value resume;

            resume["limite"] = transactionResume.value().limit;
            resume["saldo"] = transactionResume.value().balance;
        
            auto resp = HttpResponse::newHttpJsonResponse(resume);
            callback(resp);
        });

    });
}