#include <string>
#include <ctime>

class TimeUtils
{
  public:
    static std::string timestampToDateTimeString(std::time_t timestamp);
    static std::time_t stringToTimeU(const std::string& dateTimeString);

};
