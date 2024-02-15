#include <drogon/HttpController.h>

using namespace drogon;

class CustomerController : public drogon::HttpController<CustomerController>
{
  public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(CustomerController::addTransaction, "clientes/{1}/transacoes", Post);
    ADD_METHOD_TO(CustomerController::getExtract, "clientes/{1}/extrato", Get);
    METHOD_LIST_END

    void addTransaction(const drogon::HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, short customerId) const;

    void getExtract(const HttpRequestPtr &req, std::function<void (const HttpResponsePtr &)> &&callback, short customerId) const;
};
