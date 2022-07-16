#ifndef __TIMER_RULE_CRONTAB_H__
#define __TIMER_RULE_CRONTAB_H__

#include <regex>
#include <memory>

#include "timer_return.h"
#include "timer_rule.h"

#define TIMER_MAX_YEAR (3000)
#define TIMER_MIN_YEAR (0)

#define TIMER_MAX_MONTH (12)
#define TIMER_MIN_MONTH (1)
#define TIMER_MONTH_COUNT (TIMER_MAX_MONTH - TIMER_MIN_MONTH + 1)

#define TIMER_MAX_DAYOFMONTH (31)
#define TIMER_MIN_DAYOFMONTH (1)
#define TIMER_DAYOFMONTH_COUNT (TIMER_MAX_DAYOFMONTH - TIMER_MIN_DAYOFMONTH + 1)

#define TIMER_MAX_DAYOFWEEK (7)
#define TIMER_MIN_DAYOFWEEK (1)
#define TIMER_DAYOFWEEK_COUNT (TIMER_MAX_DAYOFWEEK - TIMER_MIN_DAYOFWEEK + 1)

#define TIMER_MAX_HOUR (23)
#define TIMER_MIN_HOUR (0)
#define TIMER_HOUR_COUNT (TIMER_MAX_HOUR - TIMER_MIN_HOUR + 1)

#define TIMER_MAX_MINUTE (59)
#define TIMER_MIN_MINUTE (0)
#define TIMER_MINUTE_COUNT (TIMER_MAX_MINUTE - TIMER_MIN_MINUTE + 1)

#define TIMER_MAX_SECOND (59)
#define TIMER_MIN_SECOND (0)
#define TIMER_SECOND_COUNT (TIMER_MAX_SECOND - TIMER_MIN_SECOND + 1)

namespace xg::timer {

class RuleCrontab : public Rule {
private:
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
        FieldRule(Rule::RefTimePoint start_time, std::string rule, int max, int min);
        FieldRule(FieldRule&& other);
        virtual ~FieldRule() { };

        void ParseRule();
        void ValidRule();
        bool Valid();
        void Reset();
        void SetLastTime(Rule::RefTimePoint last_time);
        virtual std::tuple<Return, int> PeekNextValue();
        virtual std::tuple<Return, int> PeekNextValue(int curr_value);
        virtual std::tuple<Return, int> GetNextValue();
        virtual std::tuple<Return, int> GetNextValue(int curr_value);
        void Print();
    protected:
        bool _parsed;
        std::string _raw_rule;
        Rule::RefTimePoint _start_time;
        Rule::RefTimePoint _last_time;
        int _last_value;
        int _field_max_value;
        int _field_min_value;

        std::map<RuleType, std::string> _rule_map;
        static std::map<RuleType, std::regex> RegexTable;
    };

    class YearRule : public FieldRule {
    public:
        YearRule(Rule::RefTimePoint start_time, std::string rule);
        YearRule(YearRule&& other);
        ~YearRule();

        std::tuple<Return, int> PeekNextValue();
        std::tuple<Return, int> PeekNextValue(int curr_value);
    };

    class MonthRule : public FieldRule {
    public:
        MonthRule(Rule::RefTimePoint start_time, std::string rule);
        MonthRule(MonthRule&& other);
        ~MonthRule();

        std::tuple<Return, int> PeekNextValue();
    };

    class DayOfMonthRule : public FieldRule {
    public:
        DayOfMonthRule(Rule::RefTimePoint start_time, std::string rule);
        DayOfMonthRule(DayOfMonthRule&& other);
        ~DayOfMonthRule();

        std::tuple<Return, int> PeekNextValue();
        std::tuple<Return, int> PeekNextValue(int curr_value);
    private:
        void valid_rule_();
    };

    class DayOfWeekRule : public FieldRule {
    public:
        DayOfWeekRule(Rule::RefTimePoint start_time, std::string rule);
        DayOfWeekRule(DayOfWeekRule&& other);
        ~DayOfWeekRule();

        std::tuple<Return, int> PeekNextValue();
    private:
        void valid_rule_();
    };

    class HourRule : public FieldRule {
    public:
        HourRule(Rule::RefTimePoint start_time, std::string rule);
        HourRule(HourRule&& other);
        ~HourRule();

        std::tuple<Return, int> PeekNextValue();
    private:
        void valid_rule_();
    };

    class MinuteRule : public FieldRule {
    public:
        MinuteRule(Rule::RefTimePoint start_time, std::string rule);
        MinuteRule(MinuteRule&& other);
        ~MinuteRule();

        std::tuple<Return, int> PeekNextValue();
    private:
        void valid_rule_();
    };

    class SecondRule : public FieldRule {
    public:
        SecondRule(Rule::RefTimePoint start_time, std::string rule);
        SecondRule(SecondRule&& other);
        ~SecondRule();

        std::tuple<Return, int> PeekNextValue();
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
    RuleCrontab(std::string rule);
    RuleCrontab(RefTimePoint start_time, std::string rule);
    ~RuleCrontab();

    /**
    * @brief Valid - Inherited function(Rule).
    */
    bool Valid(WheelAccuracy& accuracy);
    /**
    * @brief Valid - Inherited function(Rule).
    */
    std::tuple<Return, WheelScale> GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy);

    /**
    * @brief Valid - Inherited function(Rule).
    */
    std::tuple<Return, RefTimePoint> GetNextExprieTime(RefTimePoint&& reftime);
    std::tuple<Return, RefTimePoint> GetNextExprieTime();

    static int GetMonthMaxDays(int year, int month);
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
