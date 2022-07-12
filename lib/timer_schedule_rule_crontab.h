#ifndef __TIMER_SCHEDULE_RULE_CRONTAB_H__
#define __TIMER_SCHEDULE_RULE_CRONTAB_H__

#include <regex>
#include <memory>

#include "timer_return.h"
#include "timer_schedule_rule.h"

namespace xg::timer {

class ScheduleRuleCrontab : public ScheduleRule {
public:
    class FieldRule {
    public:
        enum class RuleType {
            SyntaxCheck,
            Any,
            Frequency,
            Range,
            FrequencyRange,
            Value,
        };
    public:
        FieldRule(ScheduleRule::RefTimePoint start_time, std::string rule);
        FieldRule(FieldRule&& other);
        virtual ~FieldRule() { };

        bool Valid();
        void Reset();
        virtual std::tuple<Return, int> GetNextValue() = 0;
        virtual std::tuple<Return, int> GetNextValue(int curr_value) = 0;
        void Print();
        void parse_rule_();
    protected:
        bool _parsed;
        std::string _raw_rule;
        ScheduleRule::RefTimePoint _start_time;

        int _last_value;
        std::map<RuleType, std::string> _rule_map;
    protected:
        static std::map<RuleType, std::regex> RegexTable;
    };

    class YearRule : public FieldRule {
    public:
        YearRule(ScheduleRule::RefTimePoint start_time, std::string rule);
        YearRule(YearRule&& other);
        ~YearRule();

        std::tuple<Return, int> GetNextValue();
        std::tuple<Return, int> GetNextValue(int curr_value);
    };

    class MonthRule : public FieldRule{
    public:
        MonthRule(ScheduleRule::RefTimePoint start_time, std::string rule);
        MonthRule(MonthRule&& other);
        ~MonthRule();

        std::tuple<Return, int> GetNextValue();
        std::tuple<Return, int> GetNextValue(int curr_value);
    private:
        void valid_rule_();
    };

public:
    enum Field : int {
        Begin = 0,
        Year = Begin,
        Month,
        DayOfMonth,
        DayOfWeek,
        Hour,
        Minute,
        Second,
        End,
    };
public:
    ScheduleRuleCrontab(std::string rule);
    ScheduleRuleCrontab(RefTimePoint start_time, std::string rule);
    ~ScheduleRuleCrontab();

    /**
    * @brief Valid - Inherited function(ScheduleRule).
    */
    bool Valid(WheelAccuracy& accuracy);
    /**
    * @brief Valid - Inherited function(ScheduleRule).
    */
    std::tuple<Return, WheelScale> GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy);

    /**
    * @brief Valid - Inherited function(ScheduleRule).
    */
    RefTimePoint GetNextExprieTime(RefTimePoint&& reftime);

private:
    bool _parsed;
    std::string _raw_rule;
    RefTimePoint _start_time;
    RefTimePoint _last_time;
    std::map<int, FieldRule*> _crontab_rule;
private:
    void parse_rule_();
    FieldRule* parse_field_rule_(int field, std::string rule);
};

}

#endif
