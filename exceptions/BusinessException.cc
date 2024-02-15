#include <stdexcept>
#include <string>

class BusinessException : public std::runtime_error {
public:
    explicit BusinessException(const std::string& message)
        : std::runtime_error(message) {}
};
