#include <regex>
#include <algorithm>

#include "timer_log.h"
#include "timer_rule_crontab.h"

namespace xg::timer {

//Field
std::map<RuleCrontab::FieldRule::RuleType, std::regex>
RuleCrontab::FieldRule::RegexTable = {
    {RuleCrontab::FieldRule::RuleType::SyntaxCheck, std::regex("([0-9]|[\\/]|[\\*]|[\\-]|[\\,])*")},
    {RuleCrontab::FieldRule::RuleType::Any, std::regex("(\\*)")},
    {RuleCrontab::FieldRule::RuleType::Frequency, std::regex("\\*[\\/]([0-9]+)")},
    {RuleCrontab::FieldRule::RuleType::Range, std::regex("[0-9]+\\-[0-9]+")},
    {RuleCrontab::FieldRule::RuleType::FrequencyRange, std::regex("[0-9]+\\-[0-9]+[\\/]([0-9]+)")},
    {RuleCrontab::FieldRule::RuleType::Value, std::regex("[0-9]+")},
};

RuleCrontab::FieldRule::FieldRule(Rule::RefTimePoint start_time, std::string rule, int max, int min)
        : _parsed(false), _raw_rule(rule), _start_time(start_time), _last_time(start_time), _last_value(-1),
         _field_max_value(max), _field_min_value(min)
{
    ParseRule();
    ValidRule();
}
RuleCrontab::FieldRule::FieldRule(RuleCrontab::FieldRule&& other)
{
    _parsed = other._parsed;
    _raw_rule = other._raw_rule;
    _start_time = other._start_time;
    _last_value = other._last_value;
    _rule_map = other._rule_map;
    _field_max_value = other._field_max_value;
    _field_min_value = other._field_min_value;
}

bool RuleCrontab::FieldRule::Valid()
{
    return _parsed;
}

void RuleCrontab::FieldRule::SetLastTime(Rule::RefTimePoint last_time)
{
    _last_time = last_time;
}

void RuleCrontab::FieldRule::Reset()
{
}

void RuleCrontab::FieldRule::Print()
{
}

void RuleCrontab::FieldRule::ParseRule()
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

void RuleCrontab::FieldRule::ValidRule()
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

std::tuple<Return, int> RuleCrontab::FieldRule::PeekNextValue()
{
    return PeekNextValue(_last_value);
}

std::tuple<Return, int> RuleCrontab::FieldRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                return {Return::E_RULE_INVALID, -1};
        }
        if (next_value_min == -1) {
            next_value_min = next_value_tmp;
        } else {
            next_value_min = next_value_tmp < next_value_min ? next_value_tmp : next_value_min;
        }
    }
    return {Return::SUCCESS, next_value_min};
}

std::tuple<Return, int> RuleCrontab::FieldRule::GetNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto ret = PeekNextValue();
        if (Return::SUCCESS == std::get<0>(ret)) {
            _last_value = std::get<1>(ret);
        }
        return ret;
    }
    return GetNextValue(_last_value);
}

std::tuple<Return, int> RuleCrontab::FieldRule::GetNextValue(int curr_value)
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    TIMER_RULE_INFO("--grh-- [", __LINE__, "]");
    auto ret = PeekNextValue(curr_value);
    TIMER_RULE_INFO("--grh-- [", __LINE__, "]");
    return ret;
}

//RuleCrontab::YearRule
RuleCrontab::YearRule::YearRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_YEAR, TIMER_MIN_YEAR)
{
}
RuleCrontab::YearRule::YearRule(RuleCrontab::YearRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::YearRule::~YearRule() { }

std::tuple<Return, int> RuleCrontab::YearRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_year = std::chrono::year_month_day(start_days).year();
        return {Return::SUCCESS, (int)start_year};
    }
    return PeekNextValue(_last_value);
}

std::tuple<Return, int> RuleCrontab::YearRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
                }
                if (curr_value < std::stoi(sm.str(1))) {
                    next_value_tmp = std::stoi(sm.str(1));
                } else if (curr_value >= std::stoi(sm.str(2))) {
                    ret_tmp = Return::E_RULE_REACH_LIMIT;
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
                }
                if (curr_value < std::stoi(sm.str(1))) {
                    next_value_tmp = std::stoi(sm.str(1));
                } else if (curr_value >= std::stoi(sm.str(2))) {
                    ret_tmp = Return::E_RULE_REACH_LIMIT;
                } else {
                    next_value_tmp = curr_value + std::stoi(sm.str(3));
                    if (next_value_tmp > std::stoi(sm.str(2))) {
                        ret_tmp = Return::E_RULE_REACH_LIMIT;
                    }
                }
            }
            break;
            case RuleType::Value:
                if (curr_value < std::stoi(rule.second)) {
                    next_value_tmp = std::stoi(rule.second);
                } else {
                    ret_tmp = Return::E_RULE_REACH_LIMIT;
                }
                break;
            default:
                return {Return::E_RULE_INVALID, -1};
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
        ret = Return::E_RULE_REACH_LIMIT;
    }
    return {ret, next_value_min};
}

//RuleCrontab::MonthRule
RuleCrontab::MonthRule::MonthRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_MONTH, TIMER_MIN_MONTH)
{
}
RuleCrontab::MonthRule::MonthRule(RuleCrontab::MonthRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::MonthRule::~MonthRule() { }

std::tuple<Return, int> RuleCrontab::MonthRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_month = std::chrono::year_month_day(start_days).month();
        return {Return::SUCCESS, (unsigned int)start_month};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//RuleCrontab::DayOfMonthRule
RuleCrontab::DayOfMonthRule::DayOfMonthRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_DAYOFMONTH, TIMER_MIN_DAYOFMONTH)
{
}
RuleCrontab::DayOfMonthRule::DayOfMonthRule(RuleCrontab::DayOfMonthRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::DayOfMonthRule::~DayOfMonthRule() { }

std::tuple<Return, int> RuleCrontab::DayOfMonthRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_day = std::chrono::year_month_day(start_days).day();
        _last_value = (unsigned int)start_day;
        return {Return::SUCCESS, (unsigned int)start_day};
    }
    return FieldRule::PeekNextValue(_last_value);
}

std::tuple<Return, int> RuleCrontab::DayOfMonthRule::PeekNextValue(int curr_value)
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    auto start_days = std::chrono::floor<std::chrono::days>(_last_time);
    auto start_year = std::chrono::year_month_day(start_days).year();
    auto start_month = std::chrono::year_month_day(start_days).month();
    _field_max_value = GetMonthMaxDays((int)start_year, (unsigned int)start_month);
    TIMER_RULE_ERROR("---grh--- max [", _field_max_value, "]");
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 2) {
                    TIMER_RULE_ERROR("Not found right frequency in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                    return {Return::E_RULE_INVALID, -1};
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return {Return::E_RULE_INVALID, -1};
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
                return {Return::E_RULE_INVALID, -1};
        }
        if (next_value_min == -1) {
            next_value_min = next_value_tmp;
        } else {
            next_value_min = next_value_tmp < next_value_min ? next_value_tmp : next_value_min;
        }
    }
    return {Return::SUCCESS, next_value_min};
    

    return {Return::SUCCESS, curr_value};
}

//RuleCrontab::DayOfWeekRule
RuleCrontab::DayOfWeekRule::DayOfWeekRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_DAYOFWEEK, TIMER_MIN_DAYOFWEEK)
{
}
RuleCrontab::DayOfWeekRule::DayOfWeekRule(RuleCrontab::DayOfWeekRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::DayOfWeekRule::~DayOfWeekRule() { }

std::tuple<Return, int> RuleCrontab::DayOfWeekRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        auto start_days = std::chrono::floor<std::chrono::days>(_start_time);
        auto start_weekday = std::chrono::year_month_weekday(start_days).weekday().c_encoding();
        TIMER_RULE_INFO("---grh--- week [", start_weekday, "]");
        return {Return::SUCCESS, start_weekday};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//RuleCrontab::HourRule
RuleCrontab::HourRule::HourRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_HOUR, TIMER_MIN_HOUR)
{
}
RuleCrontab::HourRule::HourRule(RuleCrontab::HourRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::HourRule::~HourRule() { }

std::tuple<Return, int> RuleCrontab::HourRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        std::chrono::hh_mm_ss start_time(_start_time- std::chrono::floor<std::chrono::days>(_start_time));
        return {Return::SUCCESS, start_time.hours().count()};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//RuleCrontab::MinuteRule
RuleCrontab::MinuteRule::MinuteRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_MINUTE, TIMER_MIN_MINUTE)
{
}
RuleCrontab::MinuteRule::MinuteRule(RuleCrontab::MinuteRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::MinuteRule::~MinuteRule() { }

std::tuple<Return, int> RuleCrontab::MinuteRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        std::chrono::hh_mm_ss start_time(_start_time- std::chrono::floor<std::chrono::days>(_start_time));
        return {Return::SUCCESS, start_time.minutes().count()};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//RuleCrontab::SecondRule
RuleCrontab::SecondRule::SecondRule(Rule::RefTimePoint start_time, std::string rule)
        : RuleCrontab::FieldRule(start_time, rule, TIMER_MAX_SECOND, TIMER_MIN_SECOND)
{
}
RuleCrontab::SecondRule::SecondRule(RuleCrontab::SecondRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::SecondRule::~SecondRule() { }

std::tuple<Return, int> RuleCrontab::SecondRule::PeekNextValue()
{
    if (!_parsed) return {Return::E_RULE_INVALID, -1};
    if (_last_value == -1) {
        std::chrono::hh_mm_ss start_time(_start_time- std::chrono::floor<std::chrono::days>(_start_time));
        return {Return::SUCCESS, start_time.seconds().count()};
    }
    return FieldRule::PeekNextValue(_last_value);
}

//RuleCrontab
RuleCrontab::RuleCrontab(std::string rule) : _parsed(false), _raw_rule(rule)
{
    _start_time = std::chrono::system_clock::now();
    parse_rule_();
}

RuleCrontab::RuleCrontab(RefTimePoint start_time, std::string rule)
        : _parsed(false), _raw_rule(rule), _start_time(start_time)
{
    parse_rule_();
}

RuleCrontab::~RuleCrontab()
{
    for (auto it : _crontab_rule) {
        delete it.second;
    }
}

bool RuleCrontab::Valid(WheelAccuracy& accuracy)
{
    if (!_parsed) return false;
    std::ignore = accuracy;
    return true;
}

std::tuple<Return, WheelScale>
RuleCrontab::GetNextExprieScale(RefTimePoint&& reftime, WheelAccuracy& accuracy)
{
    std::ignore = reftime;
    std::ignore = accuracy;
    return std::make_tuple(Return(Return::E_RULE_INVALID), WheelScale());
}

std::tuple<Return, RuleCrontab::RefTimePoint>
RuleCrontab::GetNextExprieTime(RefTimePoint&& reftime)
{
    auto curr_days = std::chrono::floor<std::chrono::days>(reftime);
    std::chrono::hh_mm_ss curr_time(reftime - curr_days);
//    int curr_year = (int)std::chrono::year_month_day(curr_days).year();
//    int curr_month = (unsigned int)std::chrono::year_month_day(curr_days).month();
//    int curr_day = (unsigned int)std::chrono::year_month_day(curr_days).day();
//    int curr_weekday = std::chrono::year_month_weekday(curr_days).weekday().c_encoding();

    RuleCrontab::RefTimePoint next_time;
    int second;
    int curr_second = curr_time.seconds().count();
    auto second_ret = _crontab_rule[Field::Second]->GetNextValue(curr_second);
    if (std::get<0>(second_ret) == Return::SUCCESS) {
        second = std::get<1>(second_ret);
    } else {
        return {std::get<0>(second_ret), next_time};
    }
    if (second < TIMER_MAX_SECOND) {
        next_time = std::chrono::floor<std::chrono::minutes>(reftime)
                    + std::chrono::seconds(second);
        return {Return::SUCCESS, next_time};
    }

    int minute;
    int curr_minute = curr_time.minutes().count();
    auto minute_ret = _crontab_rule[Field::Minute]->GetNextValue(curr_minute);
    if (std::get<0>(minute_ret) == Return::SUCCESS) {
        minute = std::get<1>(minute_ret);
    } else {
        return {std::get<0>(minute_ret), next_time};
    }
    if (minute < TIMER_MAX_MINUTE) {
        if (minute - curr_minute >= second / TIMER_SECOND_COUNT) {
            next_time = std::chrono::floor<std::chrono::hours>(reftime)
                        + std::chrono::minutes(minute)
                        + std::chrono::seconds(second % TIMER_SECOND_COUNT);
            return {Return::SUCCESS, next_time};
        } else {
            return {Return::E_RULE_CONFLICT, next_time};
        }
    }

    int hour;
    int curr_hour = curr_time.hours().count();
    auto hour_ret = _crontab_rule[Field::Hour]->GetNextValue(curr_hour);
    if (std::get<0>(hour_ret) == Return::SUCCESS) {
        hour = std::get<1>(hour_ret);
    } else {
        return {std::get<0>(hour_ret), next_time};
    }
    if (hour < TIMER_MAX_HOUR) {
        if (hour - curr_hour >= minute / TIMER_HOUR_COUNT) {
            next_time = std::chrono::floor<std::chrono::days>(reftime)
                        + std::chrono::hours(hour)
                        + std::chrono::minutes(minute % TIMER_HOUR_COUNT)
                        + std::chrono::seconds(second % TIMER_SECOND_COUNT);
            return {Return::SUCCESS, next_time};
        } else {
            return {Return::E_RULE_CONFLICT, next_time};
        }
    }

    return {Return::SUCCESS, next_time};
}

std::tuple<Return, RuleCrontab::RefTimePoint>
RuleCrontab::GetNextExprieTime()
{
    if (_last_time.time_since_epoch().count() == 0) {
        _last_time = _start_time;
        return {Return::SUCCESS, _last_time};
    }
    return GetNextExprieTime(std::move(_last_time));
}

int RuleCrontab::GetMonthMaxDays(int year, int month)
{
    switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            return 31;
        case 2:
            if ((((int)year) % 400 == 0) || (((int)year) % 4 == 0 && ((int)year) % 100 != 0)) {
                return 29;
            } else {
                return 28;
            }
            break;
        default:
            return 30;
    }
}

void RuleCrontab::parse_rule_()
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

RuleCrontab::FieldRule* RuleCrontab::parse_field_rule_(int field, std::string rule)
{
    TIMER_RULE_INFO("field rule[", rule, "]");
    
    switch (field) {
        case Field::Year:
            return new RuleCrontab::YearRule(_start_time, rule);
        case Field::Month:
            return new RuleCrontab::MonthRule(_start_time, rule);
        case Field::DayOfMonth:
            return new RuleCrontab::DayOfMonthRule(_start_time, rule);
        case Field::DayOfWeek:
            return new RuleCrontab::DayOfWeekRule(_start_time, rule);
        case Field::Hour:
            return new RuleCrontab::HourRule(_start_time, rule);
        case Field::Minute:
            return new RuleCrontab::MinuteRule(_start_time, rule);
        case Field::Second:
            return new RuleCrontab::SecondRule(_start_time, rule);
        default:
            break;
    }
    return nullptr;
}

}
