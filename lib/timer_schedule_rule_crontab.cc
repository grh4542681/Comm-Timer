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

ScheduleRuleCrontab::FieldRule::FieldRule(ScheduleRule::RefTimePoint start_time, std::string rule, int max, int min)
        : _parsed(false), _raw_rule(rule), _start_time(start_time), _last_time(start_time), _last_value(-1),
         _field_max_value(max), _field_min_value(min)
{
    ParseRule();
    ValidRule();
}
ScheduleRuleCrontab::FieldRule::FieldRule(ScheduleRuleCrontab::FieldRule&& other)
{
    _parsed = other._parsed;
    _raw_rule = other._raw_rule;
    _start_time = other._start_time;
    _last_value = other._last_value;
    _rule_map = other._rule_map;
    _field_max_value = other._field_max_value;
    _field_min_value = other._field_min_value;
}

bool ScheduleRuleCrontab::FieldRule::Valid()
{
    return _parsed;
}

void ScheduleRuleCrontab::FieldRule::SetLastTime(ScheduleRule::RefTimePoint last_time)
{
    _last_time = last_time;
}

void ScheduleRuleCrontab::FieldRule::Reset()
{
}

void ScheduleRuleCrontab::FieldRule::Print()
{
}

void ScheduleRuleCrontab::FieldRule::ParseRule()
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

void ScheduleRuleCrontab::FieldRule::ValidRule()
{
    if (!_parsed) return;
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
                if (std::stoi(sm.str(1)) < _field_min_value || std::stoi(sm.str(1)) > _field_max_value) {
                    TIMER_RULE_ERROR("Range start value [", std::stoi(sm.str(1)), "] invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) < _field_min_value || std::stoi(sm.str(2)) > _field_max_value) {
                    TIMER_RULE_ERROR("Range end value [", std::stoi(sm.str(2)), "] invalid in rule[" , rule.second, "]");
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
                if (std::stoi(sm.str(1)) < _field_min_value || std::stoi(sm.str(1)) > _field_max_value) {
                    TIMER_RULE_ERROR("Range start value [", std::stoi(sm.str(1)), "] invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(2)) < _field_min_value || std::stoi(sm.str(2)) > _field_max_value) {
                    TIMER_RULE_ERROR("Range end value [", std::stoi(sm.str(2)), "] invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                if (std::stoi(sm.str(3)) > (std::stoi(sm.str(2)) - std::stoi(sm.str(1)))) {
                    TIMER_RULE_ERROR("Range value < frequency value in rule[" , rule.second, "]");
                    _parsed = false;
                }
            }
            break;
            case RuleType::Value:
                if (std::stoi(rule.second) < _field_min_value || std::stoi(rule.second) > _field_max_value) {
                    TIMER_RULE_ERROR("Value invalid in rule[" , rule.second, "]");
                    _parsed = false;
                }
                break;
            default:
                break;
        }
    }
}

std::tuple<Return, int> ScheduleRuleCrontab::FieldRule::PeekNextValue()
{
    return PeekNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::FieldRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    int next_value_min = -1;
    for (auto rule : _rule_map) {
        int next_value_tmp = -1;
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
                    next_value_tmp = _field_max_value - _field_min_value + 1 + std::stoi(sm.str(1));
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
                    next_value_tmp = _field_max_value - _field_min_value + 1 + std::stoi(sm.str(1));
                } else {
                    next_value_tmp = curr_value + std::stoi(sm.str(3));
                    if (next_value_tmp > std::stoi(sm.str(2))) {
                        next_value_tmp = _field_max_value - _field_min_value + 1 + std::stoi(sm.str(1));
                    }
                }
            }
            break;
            case RuleType::Value:
                if (curr_value < std::stoi(rule.second)) {
                    next_value_tmp = std::stoi(rule.second);
                } else {
                    next_value_tmp = _field_max_value - _field_min_value + 1 + std::stoi(rule.second);
                }
                break;
            default:
                return {Return::ESCHEDULE_RULE_INVALID, -1};
        }
        if (next_value_min == -1) {
            next_value_min = next_value_tmp;
        } else {
            next_value_min = next_value_tmp < next_value_min ? next_value_tmp : next_value_min;
        }
    }
    return {Return::SUCCESS, next_value_min};
}

std::tuple<Return, int> ScheduleRuleCrontab::FieldRule::GetNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto ret = PeekNextValue();
        if (Return::SUCCESS == std::get<0>(ret)) {
            _last_value = std::get<1>(ret);
        }
        return ret;
    }
    return GetNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::FieldRule::GetNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value < _field_min_value || _last_value > _field_max_value) {
        return {Return::ERROR, -1};
    }
    auto ret = PeekNextValue(curr_value);
    if (std::get<0>(ret) == Return::SUCCESS) {
        _last_value = std::get<1>(ret) % (_field_max_value - _field_min_value + 1);
    }
    return ret;
}

//ScheduleRuleCrontab::YearRule
ScheduleRuleCrontab::YearRule::YearRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_YEAR, TIMER_MIN_YEAR)
{
}
ScheduleRuleCrontab::YearRule::YearRule(ScheduleRuleCrontab::YearRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::YearRule::~YearRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::YearRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_year = std::chrono::year_month_day(start_days).year();
        return {Return::SUCCESS, (int)start_year};
    }
    return PeekNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::YearRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    int next_value_min = -1;
    Return ret = Return::ERROR;
    for (auto rule : _rule_map) {
        int next_value_tmp = -1;
        Return ret_tmp = Return::SUCCESS;
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
    if (next_value_min > TIMER_MAX_YEAR) {
        ret = Return::ESCHEDULE_RULE_REACH_LIMIT;
    }
    return {ret, next_value_min};
}

//ScheduleRuleCrontab::MonthRule
ScheduleRuleCrontab::MonthRule::MonthRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_MONTH, TIMER_MIN_MONTH)
{
}
ScheduleRuleCrontab::MonthRule::MonthRule(ScheduleRuleCrontab::MonthRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::MonthRule::~MonthRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::MonthRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_month = std::chrono::year_month_day(start_days).month();
        return {Return::SUCCESS, (unsigned int)start_month};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//ScheduleRuleCrontab::DayOfMonthRule
ScheduleRuleCrontab::DayOfMonthRule::DayOfMonthRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_DAYOFMONTH, TIMER_MIN_DAYOFMONTH)
{
}
ScheduleRuleCrontab::DayOfMonthRule::DayOfMonthRule(ScheduleRuleCrontab::DayOfMonthRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::DayOfMonthRule::~DayOfMonthRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::DayOfMonthRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_day = std::chrono::year_month_day(start_days).day();
        _last_value = (unsigned int)start_day;
        return {Return::SUCCESS, (unsigned int)start_day};
    }
    return FieldRule::PeekNextValue(_last_value);
}

std::tuple<Return, int> ScheduleRuleCrontab::DayOfMonthRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};

    return {Return::SUCCESS, curr_value};
}

//ScheduleRuleCrontab::DayOfWeekRule
ScheduleRuleCrontab::DayOfWeekRule::DayOfWeekRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_DAYOFWEEK, TIMER_MIN_DAYOFWEEK)
{
}
ScheduleRuleCrontab::DayOfWeekRule::DayOfWeekRule(ScheduleRuleCrontab::DayOfWeekRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::DayOfWeekRule::~DayOfWeekRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::DayOfWeekRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_weekday = std::chrono::year_month_weekday(start_days).index();
        _last_value = (unsigned int)start_day;
        return {Return::SUCCESS, (unsigned int)start_day};
    }
    return FieldRule::PeekNextValue(_last_value);
    return {Return::SUCCESS, -1};

}

std::tuple<Return, int> ScheduleRuleCrontab::DayOfWeekRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};

    return {Return::SUCCESS, curr_value};
}

//ScheduleRuleCrontab::HourRule
ScheduleRuleCrontab::HourRule::HourRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_HOUR, TIMER_MIN_HOUR)
{
}
ScheduleRuleCrontab::HourRule::HourRule(ScheduleRuleCrontab::HourRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::HourRule::~HourRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::HourRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        std::chrono::hh_mm_ss start_time(_start_time- std::chrono::floor<std::chrono::days>(_start_time));
        return {Return::SUCCESS, start_time.hours().count()};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//ScheduleRuleCrontab::MinuteRule
ScheduleRuleCrontab::MinuteRule::MinuteRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_MINUTE, TIMER_MIN_MINUTE)
{
}
ScheduleRuleCrontab::MinuteRule::MinuteRule(ScheduleRuleCrontab::MinuteRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::MinuteRule::~MinuteRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::MinuteRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        std::chrono::hh_mm_ss start_time(_start_time- std::chrono::floor<std::chrono::days>(_start_time));
        return {Return::SUCCESS, start_time.minutes().count()};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//ScheduleRuleCrontab::SecondRule
ScheduleRuleCrontab::SecondRule::SecondRule(ScheduleRule::RefTimePoint start_time, std::string rule)
        : ScheduleRuleCrontab::FieldRule(start_time, rule, TIMER_MAX_SECOND, TIMER_MIN_SECOND)
{
}
ScheduleRuleCrontab::SecondRule::SecondRule(ScheduleRuleCrontab::SecondRule&& other)
    : ScheduleRuleCrontab::FieldRule(std::move(other)) { }

ScheduleRuleCrontab::SecondRule::~SecondRule() { }

std::tuple<Return, int> ScheduleRuleCrontab::SecondRule::PeekNextValue()
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
    if (_last_value == -1) {
        std::chrono::hh_mm_ss start_time(_start_time- std::chrono::floor<std::chrono::days>(_start_time));
        return {Return::SUCCESS, start_time.seconds().count()};
    }
    return FieldRule::PeekNextValue(_last_value);
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
    TIMER_RULE_INFO("next hour[", std::get<1>(_crontab_rule[Field::Hour]->GetNextValue()), "]");
    TIMER_RULE_INFO("next hour[", std::get<1>(_crontab_rule[Field::Hour]->GetNextValue()), "]");
    TIMER_RULE_INFO("next minute[", std::get<1>(_crontab_rule[Field::Minute]->GetNextValue()), "]");
    TIMER_RULE_INFO("next minute[", std::get<1>(_crontab_rule[Field::Minute]->GetNextValue()), "]");
    TIMER_RULE_INFO("next second[", std::get<1>(_crontab_rule[Field::Second]->GetNextValue()), "]");
    TIMER_RULE_INFO("next second[", std::get<1>(_crontab_rule[Field::Second]->GetNextValue()), "]");
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
            break;
        case Field::DayOfWeek:
            break;
        case Field::Hour:
            return new ScheduleRuleCrontab::HourRule(_start_time, rule);
        case Field::Minute:
            return new ScheduleRuleCrontab::MinuteRule(_start_time, rule);
        case Field::Second:
            return new ScheduleRuleCrontab::SecondRule(_start_time, rule);
        default:
            break;
    }
    return nullptr;
}

}
