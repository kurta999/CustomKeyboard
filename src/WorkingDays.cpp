#include "pch.hpp"

WorkingDays::WorkingDays()
{

}

WorkingDays::~WorkingDays()
{

}

void WorkingDays::Update()
{
    boost::gregorian::date today = boost::gregorian::day_clock::local_day();
    int year = today.year();
    int month = today.month();
    try 
    {
        boost::gregorian::date first_day_of_month(year, month, 1);
        boost::gregorian::date last_day_of_month = first_day_of_month.end_of_month();
        std::set<boost::gregorian::date> holidays = GetSlovakHolidays(year);

        CountWorkingDays(first_day_of_month, last_day_of_month, true, holidays);
    }
    catch (std::exception& e) 
    {
        LOG(LogLevel::Error, "Exception with working days: {}", e.what());
    }
}

bool WorkingDays::IsWeekend(boost::gregorian::day_iterator dit)
{
    return dit->day_of_week() == boost::gregorian::Saturday || dit->day_of_week() == boost::gregorian::Sunday;
}

boost::gregorian::date WorkingDays::CalculateEaster(int year)
{
    int a = year % 19;
    int b = year / 100;
    int c = year % 100;
    int d = b / 4;
    int e = b % 4;
    int f = (b + 8) / 25;
    int g = (b - f + 1) / 3;
    int h = (19 * a + b - d - g + 15) % 30;
    int i = c / 4;
    int k = c % 4;
    int l = (32 + 2 * e + 2 * i - h - k) % 7;
    int m = (a + 11 * h + 22 * l) / 451;
    int month = (h + l - 7 * m + 114) / 31;
    int day = ((h + l - 7 * m + 114) % 31) + 1;

    return boost::gregorian::date(year, month, day);
}

std::set<boost::gregorian::date> WorkingDays::GetSlovakHolidays(int year)
{
    std::set<boost::gregorian::date> holidays =
    {
        boost::gregorian::date(year, boost::gregorian::Jan, 1),   // New Year's Day
        boost::gregorian::date(year, boost::gregorian::Jan, 6),   // Epiphany
        boost::gregorian::date(year, boost::gregorian::May, 1),   // Labour Day
        boost::gregorian::date(year, boost::gregorian::May, 8),   // Liberation Day
        boost::gregorian::date(year, boost::gregorian::Jul, 5),   // St. Cyril and Methodius Day
        boost::gregorian::date(year, boost::gregorian::Aug, 29),  // Slovak National Uprising Day
        boost::gregorian::date(year, boost::gregorian::Sep, 1),   // Constitution Day
        boost::gregorian::date(year, boost::gregorian::Sep, 15),  // Day of Our Lady of Sorrows
        boost::gregorian::date(year, boost::gregorian::Nov, 1),   // All Saints' Day
        boost::gregorian::date(year, boost::gregorian::Nov, 17),  // Struggle for Freedom and Democracy Day
        boost::gregorian::date(year, boost::gregorian::Dec, 24),  // Christmas Eve
        boost::gregorian::date(year, boost::gregorian::Dec, 25),  // Christmas Day
        boost::gregorian::date(year, boost::gregorian::Dec, 26)   // St. Stephen's Day
    };

    boost::gregorian::date easter_sunday = CalculateEaster(year);
    holidays.insert(easter_sunday + boost::gregorian::days(1));  // Easter Monday

    return holidays;
}

bool WorkingDays::IsSlovakHoliday(const boost::gregorian::date& d, const std::set<boost::gregorian::date>& holidays)
{
    return holidays.find(d) != holidays.end();
}

void WorkingDays::CountWorkingDays(const boost::gregorian::date& start_date, const boost::gregorian::date& end_date, bool skip_slovak_holidays, const std::set<boost::gregorian::date>& holidays)
{
    int working_days = 0;
    int holidays_count = 0;
    for (boost::gregorian::day_iterator dit = start_date; dit != end_date; ++dit)
    {
        if (!IsWeekend(dit) && !(skip_slovak_holidays && IsSlovakHoliday(*dit, holidays)))
        {
            ++working_days;
        }
        if (skip_slovak_holidays && IsSlovakHoliday(*dit, holidays))
        {
            ++holidays_count;
        }
    }
    // Check the end date as well
    if (!IsWeekend(boost::gregorian::day_iterator(end_date)) && !(skip_slovak_holidays && IsSlovakHoliday(end_date, holidays)))
    {
        ++working_days;
    }
    if (skip_slovak_holidays && IsSlovakHoliday(end_date, holidays))
    {
        ++holidays_count;
    }

    m_WorkingDays = working_days;
    m_Holidays = holidays_count;
}