#include <regex>
#include <algorithm>

#include "timer_log.h"
#include "timer_schedule_rule_crontab.h"

namespace xg::timer {

//Field
std::map<ScheduleRuleCrontab::FieldRule::RuleType, std::regex>
ScheduleRuleCrontab::FieldRule::RegexTable = {
    {ScheduleRuleCrontab::FieldRule::RuleType::SyntaxCheck, std::regex("([0-9]|[\\/]|[\\*]|[\\-]|[\\,])*")},
    {ScheduleRuleCrontab::FieldRule::RuleType::Any, std::regex("(\\*)")},
    {ScheduleRuleCrontab::FieldRule::RuleType::Frequency, std::regex("\\*[\\/]([0-9]+)")},
    {ScheduleRuleCrontab::FieldRule::RuleType::Range, std::regex("[0-9]+\\-[0-9]+")},
    {ScheduleRuleCrontab::FieldRule::RuleType::FrequencyRange, std::regex("[0-9]+\\-[0-9]+[\\/]([0-9]+)")},
    {ScheduleRuleCrontab::FieldRule::RuleType::Value, std::regex("[0-9]+")},
};

ScheduleRuleCrontab::FieldRule::FieldRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : _parsed(false), _raw_rule(rule), _start_time(start_time), _last_time(start_time), _last_value(-1)
{
    parse_rule_();
}
ScheduleRuleCrontab::FieldRule::FieldRule(ScheduleRuleCrontab::FieldRule&& other)
{
    _parsed = other._parsed;
    _raw_rule = other._raw_rule;
    _start_time = other._start_time;
    _last_value = other._last_value;
    _rule_map = other._rule_map;
}

bool ScheduleRuleCrontab::FieldRule::Valid()
{
    return _parsed;
}

void ScheduleRuleCrontab::FieldRule::Reset()
{
}

void ScheduleRuleCrontab::FieldRule::Print()
{
}

void ScheduleRuleCrontab::FieldRule::parse_rule_()
{
    if (!std::regex_match(_raw_rule, RegexTable[RuleType::SyntaxCheck])) {
        TIMER_RULE_ERROR("Parse Month rule[", _raw_rule, "] syntax error");
        _parsed = false;
        return;
    }

    std::regex word_regex("[^,]+");
    auto words_begin = std::sregex_iterator(_raw_rule.begin(), _raw_rule.end(), word_regex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string sub_rule = match.str();
        TIMER_RULE_INFO("sub rule[", sub_rule, "]");
        if (std::regex_match(sub_rule, RegexTable[RuleType::Any])) {
            _rule_map.insert({RuleType::Any, sub_rule});
        } else if (std::regex_match(sub_rule, RegexTable[RuleType::Frequency])) {
            _rule_map.insert({RuleType::Frequency, sub_rule});
        } else if (std::regex_match(sub_rule, RegexTable[RuleType::Range])) {
            _rule_map.insert({RuleType::Range, sub_rule});
        } else if (std::regex_match(sub_rule, RegexTable[RuleType::FrequencyRange])) {
            _rule_map.insert({RuleType::FrequencyRange, sub_rule});
        } else if (std::regex_match(sub_rule, RegexTable[RuleType::Value])) {
            _rule_map.insert({RuleType::Value, sub_rule});
        } else {
            TIMER_RULE_ERROR("Parse Month rule[", _raw_rule, "] syntax error");
            _parsed = false;
            return;
        }
    }

    _parsed = true;
}

//ScheduleRuleCrontab::YearRule
ScheduleRuleCrontab::YearRule::YearRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
}
ScheduleRuleCrontab::YearRule::YearRule(ScheduleRuleCrontab::YearRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::YearRule::~YearRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::YearRule::GetNextValue()
{
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_year = std::chrono::year_month_day(start_days).year();
        _last_value = (int)start_year;
        return {Return::SUCCESS, _last_value};
    }
    return GetNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::YearRule::GetNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    int next_value_min = -1;
    Return ret = Return::ERROR;
    for (auto rule : _rule_map) {
        int next_value_tmp = -1;
        Return ret_tmp = Return::SUCCESS;
        TIMER_RULE_ERROR("--grh-- rule [", rule.second, "]");
        switch (rule.first) {
            case RuleType::Any:
                next_value_tmp = curr_value + 1;
                break;
            case RuleType::Frequency:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                next_value_tmp = curr_value + std::stoi(sm.str(1));
            }
            break;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (curr_value < std::stoi(sm.str(1))) {
                    next_value_tmp = std::stoi(sm.str(1));
                } else if (curr_value >= std::stoi(sm.str(2))) {
                    ret_tmp = Return::ESCHEDULE_RULE_REACH_LIMIT;
                } else {
                    next_value_tmp = curr_value + 1;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (curr_value < std::stoi(sm.str(1))) {
                    next_value_tmp = std::stoi(sm.str(1));
                } else if (curr_value >= std::stoi(sm.str(2))) {
                    ret_tmp = Return::ESCHEDULE_RULE_REACH_LIMIT;
                } else {
                    next_value_tmp = curr_value + std::stoi(sm.str(3));
                    if (next_value_tmp > std::stoi(sm.str(2))) {
                        ret_tmp = Return::ESCHEDULE_RULE_REACH_LIMIT;
                    }
                }
            }
            break;
            case RuleType::Value:
                if (curr_value < std::stoi(rule.second)) {
                    next_value_tmp = std::stoi(rule.second);
                } else {
                    ret_tmp = Return::ESCHEDULE_RULE_REACH_LIMIT;
                }
                break;
            default:
                return {Return::ESCHEDULE_RULE_INVALID, -1};
        }
        TIMER_RULE_ERROR("---grh---[", next_value_tmp, "]");
        if (ret != Return::SUCCESS) {
            ret = ret_tmp;
        }
        if (ret_tmp == Return::SUCCESS) {
            if (next_value_min == -1) {
                next_value_min = next_value_tmp;
            } else {
                next_value_min = next_value_tmp < next_value_min ? next_value_tmp : next_value_min;
            }
        }
    }
    _last_value = ret == Return::SUCCESS ? next_value_min : _last_value;
    return {ret, _last_value};
}


//ScheduleRuleCrontab::MonthRule
ScheduleRuleCrontab::MonthRule::MonthRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
    valid_rule_();
}
ScheduleRuleCrontab::MonthRule::MonthRule(ScheduleRuleCrontab::MonthRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::MonthRule::~MonthRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::MonthRule::GetNextValue()
{
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_month = std::chrono::year_month_day(start_days).month();
        _last_value = (unsigned int)start_month;
        return {Return::SUCCESS, _last_value};
    }
    return GetNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::MonthRule::GetNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    int next_value_min = -1;
    for (auto rule : _rule_map) {
        int next_value_tmp = -1;
        TIMER_RULE_ERROR("--grh-- rule [", rule.second, "]");
        switch (rule.first) {
            case RuleType::Any:
                next_value_tmp = curr_value + 1;
                break;
            case RuleType::Frequency:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                next_value_tmp = curr_value + std::stoi(sm.str(1));
            }
            break;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (curr_value < std::stoi(sm.str(1))) {
                    next_value_tmp = std::stoi(sm.str(1));
                } else if (curr_value >= std::stoi(sm.str(2))) {
                    next_value_tmp = 12 + std::stoi(sm.str(1));
                } else {
                    next_value_tmp = curr_value + 1;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::ESCHEDULE_RULE_INVALID, -1};
                }
                if (curr_value < std::stoi(sm.str(1))) {
                    next_value_tmp = std::stoi(sm.str(1));
                } else if (curr_value >= std::stoi(sm.str(2))) {
                    next_value_tmp = 12 + std::stoi(sm.str(1));
                } else {
                    next_value_tmp = curr_value + std::stoi(sm.str(3));
                    if (next_value_tmp > std::stoi(sm.str(2))) {
                        next_value_tmp = 12 + std::stoi(sm.str(1));
                    }
                }
            }
            break;
            case RuleType::Value:
                if (curr_value < std::stoi(rule.second)) {
                    next_value_tmp = std::stoi(rule.second);
                } else {
                    next_value_tmp = 12 + std::stoi(rule.second);
                }
                break;
            default:
                return {Return::ESCHEDULE_RULE_INVALID, -1};
        }
        TIMER_RULE_ERROR("---grh---[", next_value_tmp, "]");
        if (next_value_min == -1) {
            next_value_min = next_value_tmp;
        } else {
            next_value_min = next_value_tmp < next_value_min ? next_value_tmp : next_value_min;
        }
    }
    _last_value = next_value_min;
    return {Return::SUCCESS, _last_value};
}

void ScheduleRuleCrontab::MonthRule::valid_rule_()
{
    for (auto rule : _rule_map) {
        switch (rule.first) {
            case RuleType::Frequency:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) == 0 || std::stoi(sm.str(1)) > 12) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) == 0 || std::stoi(sm.str(2)) > 12) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) == 0 || std::stoi(sm.str(1)) > 12) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) == 0 || std::stoi(sm.str(2)) > 12) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(3)) > (std::stoi(sm.str(2)) - std::stoi(sm.str(1)))) {
                    TIMER_RULE_ERROR("Range value < frequency value in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Value:
                if (std::stoi(rule.second) == 0 || std::stoi(rule.second) > 12) {
                    TIMER_RULE_ERROR("Value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                break;
            default:
                break;
        }
    }
}


//ScheduleRuleCrontab::DayOfMonthRule
ScheduleRuleCrontab::DayOfMonthRule::DayOfMonthRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
    valid_rule_();
}
ScheduleRuleCrontab::DayOfMonthRule::DayOfMonthRule(ScheduleRuleCrontab::DayOfMonthRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::DayOfMonthRule::~DayOfMonthRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::DayOfMonthRule::GetNextValue()
{
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_day = std::chrono::year_month_day(start_days).day();
        _last_value = (unsigned int)start_day;
        return {Return::SUCCESS, _last_value};
    }
    return GetNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::DayOfMonthRule::GetNextValue(int curr_value)
{

}

void ScheduleRuleCrontab::DayOfMonthRule::valid_rule_()
{

}

//ScheduleRuleCrontab::DayOfWeekRule
ScheduleRuleCrontab::DayOfWeekRule::DayOfWeekRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
    valid_rule_();
}
ScheduleRuleCrontab::DayOfWeekRule::DayOfWeekRule(ScheduleRuleCrontab::DayOfWeekRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::DayOfWeekRule::~DayOfWeekRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::DayOfWeekRule::GetNextValue()
{

}

std::tuple<Return, int> ScheduleRuleCrontab::DayOfWeekRule::GetNextValue(int curr_value)
{

}

void ScheduleRuleCrontab::DayOfWeekRule::valid_rule_()
{

}

//ScheduleRuleCrontab::HourRule
ScheduleRuleCrontab::HourRule::HourRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
    valid_rule_();
}
ScheduleRuleCrontab::HourRule::HourRule(ScheduleRuleCrontab::HourRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::HourRule::~HourRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::HourRule::GetNextValue()
{

}

std::tuple<Return, int> ScheduleRuleCrontab::HourRule::GetNextValue(int curr_value)
{

}

void ScheduleRuleCrontab::HourRule::valid_rule_()
{
   for (auto rule : _rule_map) {
        switch (rule.first) {
            case RuleType::Frequency:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) > 23) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) > 23) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) > 23) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) > 23) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(3)) > (std::stoi(sm.str(2)) - std::stoi(sm.str(1)))) {
                    TIMER_RULE_ERROR("Range value < frequency value in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Value:
                if (std::stoi(rule.second) > 23) {
                    TIMER_RULE_ERROR("Value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                break;
            default:
                break;
        }
    }
}

//ScheduleRuleCrontab::MinuteRule
ScheduleRuleCrontab::MinuteRule::MinuteRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
    valid_rule_();
}
ScheduleRuleCrontab::MinuteRule::MinuteRule(ScheduleRuleCrontab::MinuteRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::MinuteRule::~MinuteRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::MinuteRule::GetNextValue()
{

}

std::tuple<Return, int> ScheduleRuleCrontab::MinuteRule::GetNextValue(int curr_value)
{

}

void ScheduleRuleCrontab::MinuteRule::valid_rule_()
{
   for (auto rule : _rule_map) {
        switch (rule.first) {
            case RuleType::Frequency:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) > 59) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) > 59) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) > 59) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) > 59) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(3)) > (std::stoi(sm.str(2)) - std::stoi(sm.str(1)))) {
                    TIMER_RULE_ERROR("Range value < frequency value in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Value:
                if (std::stoi(rule.second) > 59) {
                    TIMER_RULE_ERROR("Value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                break;
            default:
                break;
        }
    }
}

//ScheduleRuleCrontab::SecondRule
ScheduleRuleCrontab::SecondRule::SecondRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule)
{
    valid_rule_();
}
ScheduleRuleCrontab::SecondRule::SecondRule(ScheduleRuleCrontab::SecondRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::SecondRule::~SecondRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::SecondRule::GetNextValue()
{

}

std::tuple<Return, int> ScheduleRuleCrontab::SecondRule::GetNextValue(int curr_value)
{

}

void ScheduleRuleCrontab::SecondRule::valid_rule_()
{
   for (auto rule : _rule_map) {
        switch (rule.first) {
            case RuleType::Frequency:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) > 59) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) > 59) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) >= std::stoi(sm.str(2))) {
                    TIMER_RULE_ERROR("Range start >= end in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(1)) > 59) {
                    TIMER_RULE_ERROR("Range start value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) > 59) {
                    TIMER_RULE_ERROR("Range end value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(3)) > (std::stoi(sm.str(2)) - std::stoi(sm.str(1)))) {
                    TIMER_RULE_ERROR("Range value < frequency value in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Value:
                if (std::stoi(rule.second) > 59) {
                    TIMER_RULE_ERROR("Value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                break;
            default:
                break;
        }
    }
}

//ScheduleRuleCrontab
ScheduleRuleCrontab::ScheduleRuleCrontab(std::string rule) : _parsed(false), _raw_rule(rule)
{
    _start_time = std::chrono::system_clock::now();
    parse_rule_();
}

ScheduleRuleCrontab::ScheduleRuleCrontab(RefTimePoint start_time, std::string rule)
        : _parsed(false), _raw_rule(rule), _start_time(start_time)
{
    parse_rule_();
}

ScheduleRuleCrontab::~ScheduleRuleCrontab()
{
    for (auto it : _crontab_rule) {
        delete it.second;
    }
}

bool ScheduleRuleCrontab::Valid(WheelAccuracy& accuracy)
{
    if (!_parsed) return false;
    std::ignore = accuracy;
    return true;
}

std::tuple<Return, WheelScale>
ScheduleRuleCrontab::GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy)
{
    std::ignore = reftime;
    std::ignore = accuracy;
    return std::make_tuple(Return(Return::ESCHEDULE_RULE_INVALID), WheelScale());
}

ScheduleRuleCrontab::RefTimePoint
ScheduleRuleCrontab::GetNextExprieTime(RefTimePoint&& reftime)
{
    TIMER_RULE_INFO("next year[",  std::get<1>(_crontab_rule[Field::Year]->GetNextValue()), "]");
    TIMER_RULE_INFO("next year[",  std::get<1>(_crontab_rule[Field::Year]->GetNextValue()), "]");
    TIMER_RULE_INFO("next month[", std::get<1>(_crontab_rule[Field::Month]->GetNextValue()), "]");
    TIMER_RULE_INFO("next month[", std::get<1>(_crontab_rule[Field::Month]->GetNextValue()), "]");
    return reftime;
}

void ScheduleRuleCrontab::parse_rule_()
{
    std::regex word_regex("[^ ]+");
    auto words_begin = std::sregex_iterator(_raw_rule.begin(), _raw_rule.end(), word_regex);
    auto words_end = std::sregex_iterator();
    int field_index = Field::Begin;
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        if (field_index >= Field::End) {
            _parsed = false;
            return;
        }
        std::smatch match = *i;
        TIMER_RULE_INFO("rule[", match.str(), "]");
        auto field_rule_p = parse_field_rule_(field_index, match.str());
        if (!field_rule_p || !field_rule_p->Valid()) {
            _parsed = false;
            //return;
        }
        _crontab_rule.insert({field_index, field_rule_p});
        ++field_index;
    }
}

ScheduleRuleCrontab::FieldRule* ScheduleRuleCrontab::parse_field_rule_(int field, std::string rule)
{
    TIMER_RULE_INFO("field rule[", rule, "]");
    
    switch (field) {
        case Field::Year:
            return new ScheduleRuleCrontab::YearRule(_start_time, rule);
        case Field::Month:
            return new ScheduleRuleCrontab::MonthRule(_start_time, rule);
        case Field::DayOfMonth:
        case Field::DayOfWeek:
        case Field::Hour:
        case Field::Minute:
        case Field::Second:
        default:
            break;
    }
    return nullptr;
}

}
