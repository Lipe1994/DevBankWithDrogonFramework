#ifndef CUSTOMER_CONTROLLER_CC
#define CUSTOMER_CONTROLLER_CC
    #include <ctime>
    #include "utils/time.cc"
    #include <vector>
#endif 
#include "CustomerController.h"
#include "services/CustomerService.cc"

    void CustomerController::getExtract(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, short customerId) const
    {
        if(customerId > 5 || customerId < 1)
        {
            auto resp = HttpResponse::newHttpResponse();
            resp->setStatusCode(k404NotFound);
            callback(resp);
            return;
        }

        CustomerService customerService;
        customerService.getExtract(customerId, [callback](std::optional<Customer> customer){

        Json::Value customerJson;

        customerJson["saldo"]["total"] = customer.value().balance;
        customerJson["saldo"]["limite"] = customer.value().limit;
        customerJson["saldo"]["data_extrato"] = customer.value().extractAt;

        
        for (const auto& customer : customer.value().transactions) {
            if(customer.createdAt != "")
            {      
                std::string tipo; 
                tipo.push_back(CustomerService::shortToTypeChar(customer.type));

                Json::Value transactionJson;
                transactionJson["valor"] = customer.amount;
                transactionJson["tipo"] = tipo;
                transactionJson["descricao"] = customer.description;
                transactionJson["realizada_em"] = customer.createdAt;

                customerJson["ultimas_transacoes"].append(transactionJson);
            }
        }

        auto resp = HttpResponse::newHttpJsonResponse(customerJson);
        resp->setStatusCode(k200OK);
        callback(resp);
    }, [callback](BusinessException& e) {

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k422UnprocessableEntity);
        callback(resp);
    });

}

void CustomerController::addTransaction(const drogon::HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, short customerId)const
{
    if(customerId > 5 || customerId < 1)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k404NotFound);
        callback(resp);
        return;
    }

    CustomerService customerService;

    auto json(req->getJsonObject());

    if(json == nullptr)
    {
        return;
    }

    double amountF = (*json)["valor"].asDouble();

    if(amountF - int(amountF) != 0)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k422UnprocessableEntity);
        callback(resp);
        return;
    }

    if((*json)["descricao"] == Json::nullValue)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k422UnprocessableEntity);
        callback(resp);
        return;
    }
    
    char type = (*json)["tipo"].asString()[0];
    std::string description = (*json)["descricao"].asString();
    if(description.length() > 10 ||description.length() == 0)
    {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k422UnprocessableEntity);
        callback(resp);
        return;
    }

    
    customerService.addTransaction(customerId, int(amountF), type, description, [callback](TransactionResume& transactionResume) {

        Json::Value resume;

        resume["limite"] = transactionResume.limit;
        resume["saldo"] = transactionResume.balance;
        auto resp = HttpResponse::newHttpJsonResponse(resume);
        resp->setStatusCode(k200OK);
        callback(resp);
    }, [callback](BusinessException& e) {
        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(k422UnprocessableEntity);
        callback(resp);
    });
}