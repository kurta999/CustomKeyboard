#include "utils/CSingleton.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>

class WorkingDays : public CSingleton < WorkingDays >
{
    friend class CSingleton < WorkingDays >;

public:
    WorkingDays();
    ~WorkingDays();

    // !\brief Initialize TCP backend server for sensors
    void Update();

    uint8_t m_WorkingDays;

    uint8_t m_Holidays;
private:
    bool IsWeekend(boost::gregorian::day_iterator dit);
    boost::gregorian::date CalculateEaster(int year);
    std::set<boost::gregorian::date> GetSlovakHolidays(int year);
    bool IsSlovakHoliday(const boost::gregorian::date& d, const std::set<boost::gregorian::date>& holidays);
    void CountWorkingDays(const boost::gregorian::date& start_date, const boost::gregorian::date& end_date, bool skip_slovak_holidays, const std::set<boost::gregorian::date>& holidays);
};