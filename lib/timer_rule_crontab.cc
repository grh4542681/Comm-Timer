#include <regex>
#include <algorithm>

#include "timer_log.hh"
#include "timer_rule_crontab.hh"

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

RuleCrontab::FieldRule::FieldRule(std::string rule, int max, int min)
        : _parsed(false), _raw_rule(rule), _field_max_value(max), _field_min_value(min)
{
    ParseRule();
    ValidRule();
}
RuleCrontab::FieldRule::FieldRule(RuleCrontab::FieldRule&& other)
{
    _parsed = other._parsed;
    _raw_rule = other._raw_rule;
    _last_time = other._last_time;
    _rule_map = other._rule_map;
    _field_max_value = other._field_max_value;
    _field_min_value = other._field_min_value;
}

bool RuleCrontab::FieldRule::Valid()
{
    return _parsed;
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

void RuleCrontab::FieldRule::SetLastTime(Rule::RefTimePoint&& last_time)
{
    _last_time = last_time;
}

bool RuleCrontab::FieldRule::CheckValue(int curr_value)
{
    if (!_parsed) return false;
    for (auto rule : _rule_map) {
        switch (rule.first) {
            case RuleType::Any:
                return true;
            case RuleType::Frequency:
                return true;
            case RuleType::Range:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found range in rule[" , rule.second, "]");
                    _parsed = false;
                    return false;
                }
                if (sm.size() != 3) {
                    TIMER_RULE_ERROR("Not found right range in rule[" , rule.second, "]");
                    _parsed = false;
                    return false;
                }
                if (curr_value >= std::stoi(sm.str(1)) || curr_value <= std::stoi(sm.str(2))) {
                    return true;
                }
            }
            break;
            case RuleType::FrequencyRange:
            {
                std::smatch sm;
                if (!std::regex_search(rule.second, sm, std::regex("([0-9]+)\\-([0-9]+)\\/([0-9]+)"))) {
                    TIMER_RULE_ERROR("Not found frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return false;
                }
                if (sm.size() != 4) {
                    TIMER_RULE_ERROR("Not found right frequency & range in rule[" , rule.second, "]");
                    _parsed = false;
                    return false;
                }
                if (curr_value >= std::stoi(sm.str(1)) || curr_value <= std::stoi(sm.str(2))) {
                    return true;
                }
            }
            break;
            case RuleType::Value:
                if (curr_value == std::stoi(rule.second)) {
                    return true;
                }
                break;
            default:
                return false;
        }
    }
    return false;
}

std::tuple<Return, int> RuleCrontab::FieldRule::GetNextValue(int curr_value)
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

//RuleCrontab::YearRule
RuleCrontab::YearRule::YearRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_YEAR, TIMER_MIN_YEAR)
{
}
RuleCrontab::YearRule::YearRule(RuleCrontab::YearRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::YearRule::~YearRule() { }

std::tuple<Return, int> RuleCrontab::YearRule::GetNextValue(int curr_value)
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

//RuleCrontab::MonthRule
RuleCrontab::MonthRule::MonthRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_MONTH, TIMER_MIN_MONTH)
{
}
RuleCrontab::MonthRule::MonthRule(RuleCrontab::MonthRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::MonthRule::~MonthRule() { }

//RuleCrontab::DayOfMonthRule
RuleCrontab::DayOfMonthRule::DayOfMonthRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_DAYOFMONTH, TIMER_MIN_DAYOFMONTH)
{
}
RuleCrontab::DayOfMonthRule::DayOfMonthRule(RuleCrontab::DayOfMonthRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::DayOfMonthRule::~DayOfMonthRule() { }

std::tuple<Return, int> RuleCrontab::DayOfMonthRule::GetNextValue(int curr_value)
{
    if (!_parsed) return {Return::ESCHEDULE_RULE_INVALID, -1};
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
    

    return {Return::SUCCESS, curr_value};
}

//RuleCrontab::DayOfWeekRule
RuleCrontab::DayOfWeekRule::DayOfWeekRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_DAYOFWEEK, TIMER_MIN_DAYOFWEEK)
{
}
RuleCrontab::DayOfWeekRule::DayOfWeekRule(RuleCrontab::DayOfWeekRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::DayOfWeekRule::~DayOfWeekRule() { }

//RuleCrontab::HourRule
RuleCrontab::HourRule::HourRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_HOUR, TIMER_MIN_HOUR)
{
}
RuleCrontab::HourRule::HourRule(RuleCrontab::HourRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::HourRule::~HourRule() { }

//RuleCrontab::MinuteRule
RuleCrontab::MinuteRule::MinuteRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_MINUTE, TIMER_MIN_MINUTE)
{
}
RuleCrontab::MinuteRule::MinuteRule(RuleCrontab::MinuteRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::MinuteRule::~MinuteRule() { }

//RuleCrontab::SecondRule
RuleCrontab::SecondRule::SecondRule(std::string rule)
        : RuleCrontab::FieldRule(rule, TIMER_MAX_SECOND, TIMER_MIN_SECOND)
{
}
RuleCrontab::SecondRule::SecondRule(RuleCrontab::SecondRule&& other)
    : RuleCrontab::FieldRule(std::move(other)) { }

RuleCrontab::SecondRule::~SecondRule() { }

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
    return std::make_tuple(Return(Return::ESCHEDULE_RULE_INVALID), WheelScale());
}

int RuleCrontab::get_field_value_(RefTimePoint&& time, Field&& field)
{
    auto curr_days = std::chrono::floor<std::chrono::days>(time);
    std::chrono::hh_mm_ss curr_time(time - curr_days);
    switch (field) {
        case Year:
            return ((int)std::chrono::year_month_day(curr_days).year());
        case Month:
            return ((unsigned int)std::chrono::year_month_day(curr_days).month());
        case DayOfMonth:
            return ((unsigned int)std::chrono::year_month_day(curr_days).day());
        case DayOfWeek:
            return std::chrono::year_month_weekday(curr_days).weekday().c_encoding();
        case Hour:
            return curr_time.hours().count();
        case Minute:
            return curr_time.minutes().count();
        case Second:
            return curr_time.seconds().count();
        default:
            return -1;
    }
}
void RuleCrontab::set_field_value_(RefTimePoint&& time, Field&& field, int value)
{
    auto curr_days = std::chrono::floor<std::chrono::days>(time);
    std::chrono::hh_mm_ss curr_time(time - curr_days);
    switch (field) {
        case Year:
        {
            int year;
            if (value < (int)std::chrono::year_month_day(curr_days).year()) {
                year = 0;
            } else {
                year = value - (int)std::chrono::year_month_day(curr_days).year();
            }
            time += std::chrono::years(year);
        }
        break;
        case Month:
        {
            int month;
            if ((unsigned int)value < (unsigned int)std::chrono::year_month_day(curr_days).month()) {
                month = value + TIMER_MONTH_COUNT - (unsigned int)std::chrono::year_month_day(curr_days).month();
            } else {
                month = value - (unsigned int)std::chrono::year_month_day(curr_days).month();
            }
            time += std::chrono::months(month);
        }
        break;
        case DayOfMonth:
        {
            int monthdays;
            if ((unsigned int)value < (unsigned int)std::chrono::year_month_day(curr_days).day()) {
                monthdays = value + TIMER_DAYOFMONTH_COUNT - (unsigned int)std::chrono::year_month_day(curr_days).day();
            } else {
                monthdays = value - (unsigned int)std::chrono::year_month_day(curr_days).day();
            }
            time += std::chrono::days(monthdays);
        }
        break;
        case DayOfWeek:
        {
            int weekdays;
            if ((unsigned int)value < std::chrono::year_month_weekday(curr_days).weekday().c_encoding()) {
                weekdays = value + TIMER_DAYOFWEEK_COUNT - std::chrono::year_month_weekday(curr_days).weekday().c_encoding();
            } else {
                weekdays = value - std::chrono::year_month_weekday(curr_days).weekday().c_encoding();
            }
            time += std::chrono::days(weekdays);
        }
        break;
        case Hour:
        {
            int hours;
            if (value < curr_time.hours().count()) {
                hours = value + TIMER_HOUR_COUNT - curr_time.hours().count();
            } else {
                hours = value - curr_time.hours().count();
            }
            time += std::chrono::hours(hours);
        }
        break;
        case Minute:
        {
            int minutes;
            if (value < curr_time.minutes().count()) {
                minutes = value + TIMER_MINUTE_COUNT - curr_time.minutes().count();
            } else {
                minutes = value - curr_time.minutes().count();
            }
            time += std::chrono::minutes(minutes);
        }
        break;
        case Second:
        {
            int seconds;
            if (value < curr_time.seconds().count()) {
                seconds = value + TIMER_SECOND_COUNT - curr_time.seconds().count();
            } else {
                seconds = value - curr_time.seconds().count();
            }
            time += std::chrono::seconds(seconds);
        }
        break;
        default:
            break;
    }
}

Return RuleCrontab::gen_next_time_(RefTimePoint& next_time, Field&& field)
{
    TIMER_RULE_ERROR("--grh-- [", __LINE__,"]");
    int field_value = get_field_value_(std::move(next_time), std::forward<Field>(field));
    if (!_crontab_rule[field]->CheckValue(field_value)) {
        _crontab_rule[field]->SetLastTime(std::move(next_time));
        auto ret = _crontab_rule[field]->GetNextValue(field_value);

        if (std::get<0>(ret) != Return::SUCCESS) {
            return std::get<0>(ret);
        }
        set_field_value_(std::move(next_time), std::forward<Field>(field), std::get<1>(ret));
    }
    if (field > Field::Begin) {
        return gen_next_time_(next_time, (Field)(field - 1));
    }
    return Return::SUCCESS;
}

std::tuple<Return, RuleCrontab::RefTimePoint>
RuleCrontab::GetNextExprieTime(RefTimePoint&& reftime)
{
    RuleCrontab::RefTimePoint next_time = reftime;
    gen_next_time_(next_time, Field::Second);
#if 0
    auto curr_days = std::chrono::floor<std::chrono::days>(reftime);
    std::chrono::hh_mm_ss curr_time(reftime - curr_days);
//    int curr_month = (unsigned int)std::chrono::year_month_day(curr_days).month();

    int second;
    int curr_second = curr_time.seconds().count();
    auto second_ret = _crontab_rule[Field::Second]->GetNextValue(curr_second);
    if (std::get<0>(second_ret) == Return::SUCCESS) {
        second = std::get<1>(second_ret);
    } else {
        return {std::get<0>(second_ret), next_time};
    }
    if (second <= TIMER_MAX_SECOND) {
        next_time = std::chrono::floor<std::chrono::minutes>(reftime)
                    + std::chrono::seconds(second);
        return {Return::SUCCESS, next_time};
    }

    int minute;
    int curr_minute = curr_time.minutes().count();
    int tmp_minute = curr_minute;
    int minute_len = 0;
    do {
        auto minute_ret = _crontab_rule[Field::Minute]->GetNextValue(tmp_minute);
        if (std::get<0>(minute_ret) == Return::SUCCESS) {
            minute = std::get<1>(minute_ret);
        } else {
            return {std::get<0>(minute_ret), next_time};
        }
        minute_len += minute - tmp_minute;
        tmp_minute = minute % TIMER_MINUTE_COUNT;
    } while (minute - curr_minute < second / TIMER_SECOND_COUNT);
    if (minute_len % (second / TIMER_SECOND_COUNT) == 0) {
        if (minute <= TIMER_MAX_MINUTE) {
            next_time = std::chrono::floor<std::chrono::hours>(reftime)
                        + std::chrono::minutes(minute)
                        + std::chrono::seconds(second % TIMER_SECOND_COUNT);
            return {Return::SUCCESS, next_time};
        }
    } else {
        return {Return::ESCHEDULE_RULE_CONFLICT, next_time};
    }

    int hour;
    TIMER_RULE_ERROR("--grh-- [", __LINE__,"]");
    int curr_hour = curr_time.hours().count();
    int tmp_hour = curr_hour;
    int hour_len = 0;
    do {
        auto hour_ret = _crontab_rule[Field::Hour]->GetNextValue(tmp_hour);
        if (std::get<0>(hour_ret) == Return::SUCCESS) {
            hour = std::get<1>(hour_ret);
        } else {
            return {std::get<0>(hour_ret), next_time};
        }
        hour_len += hour - tmp_hour;
        tmp_hour = hour % TIMER_HOUR_COUNT;
    } while (hour - curr_hour < minute / TIMER_MINUTE_COUNT);
    if (hour_len % (minute / TIMER_MINUTE_COUNT) == 0) {
        if (hour <= TIMER_MAX_HOUR) {
            next_time = std::chrono::floor<std::chrono::days>(reftime)
                        + std::chrono::hours(hour)
                        + std::chrono::minutes(minute % TIMER_MINUTE_COUNT)
                        + std::chrono::seconds(second % TIMER_SECOND_COUNT);
            return {Return::SUCCESS, next_time};
        }
    } else {
        return {Return::ESCHEDULE_RULE_CONFLICT, next_time};
    }

    int curr_year = (int)std::chrono::year_month_day(curr_days).year();
    int curr_month = (unsigned int)std::chrono::year_month_day(curr_days).month();
    int curr_day = (unsigned int)std::chrono::year_month_day(curr_days).day();
    int curr_weekday = std::chrono::year_month_weekday(curr_days).weekday().c_encoding();

    int weekday;
    int monthday;
    int tmp_weekday = curr_weekday;
    int tmp_monthday = curr_day;
    int tmp_day_len = 0;
    do {
        auto tmp_last_time = reftime + std::chrono::days(tmp_day_len);
        _crontab_rule[Field::DayOfMonth]->SetLastTime(std::move(tmp_last_time));
        auto weekday_ret = _crontab_rule[Field::DayOfWeek]->GetNextValue(tmp_weekday);
        if (std::get<0>(weekday_ret) == Return::SUCCESS) {
            weekday = std::get<1>(weekday_ret);
        } else {
            return {std::get<0>(weekday_ret), next_time};
        }
        int weekday_len = weekday - tmp_weekday;
    TIMER_RULE_ERROR("--grh-- [", __LINE__,"] weekday [", weekday, "] len [", weekday_len, "]");

        auto monthday_ret = _crontab_rule[Field::DayOfMonth]->GetNextValue(tmp_monthday);
        if (std::get<0>(monthday_ret) == Return::SUCCESS) {
            monthday = std::get<1>(monthday_ret);
        } else {
            return {std::get<0>(monthday_ret), next_time};
        }
        int monthday_len = monthday - tmp_monthday;
    TIMER_RULE_ERROR("--grh-- [", __LINE__,"] monthday [", monthday, "] len [", monthday_len, "]");
        int day_len = weekday_len < monthday_len ? weekday_len : monthday_len;
        tmp_day_len += day_len;

        tmp_weekday = (tmp_weekday + day_len) % TIMER_DAYOFWEEK_COUNT;
        tmp_monthday += day_len;
        int tmp_curr_month = (unsigned int)(std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tmp_last_time)).month());
        int tmp_curr_year = (int)(std::chrono::year_month_day(std::chrono::floor<std::chrono::days>(tmp_last_time)).year());
        while(tmp_monthday > GetMonthMaxDays(tmp_curr_year, tmp_curr_month)) {
            tmp_monthday -= GetMonthMaxDays(tmp_curr_year, tmp_curr_month);
            tmp_curr_month++;
            tmp_curr_year += tmp_curr_month / TIMER_MONTH_COUNT;
            tmp_curr_month = tmp_curr_month % TIMER_MONTH_COUNT;
        }
    } while (tmp_day_len < hour / TIMER_HOUR_COUNT);
    TIMER_RULE_ERROR("--grh-- [", __LINE__,"] tmp_day_len [", tmp_day_len, "]");
    TIMER_RULE_ERROR("--grh-- [", __LINE__,"] hour [", hour % TIMER_HOUR_COUNT, "]");
    if (tmp_day_len % (hour / TIMER_HOUR_COUNT) == 0) {
        if ((curr_day + tmp_day_len) < GetMonthMaxDays(curr_year, curr_month)) {
            next_time = std::chrono::floor<std::chrono::days>(reftime)
                        + std::chrono::days(tmp_day_len)
                        + std::chrono::hours(hour % TIMER_HOUR_COUNT)
                        + std::chrono::minutes(minute % TIMER_MINUTE_COUNT)
                        + std::chrono::seconds(second % TIMER_SECOND_COUNT);
            return {Return::SUCCESS, next_time};
        }
    } else {
        return {Return::ESCHEDULE_RULE_CONFLICT, next_time};
    }

    int exceed_month = 0;
    int tmp_exceed_curr_month = curr_month;
    int tmp_exceed_curr_year = curr_year;
    int tmp_exceed_monthday = curr_day + tmp_day_len;
    while(tmp_exceed_monthday > GetMonthMaxDays(tmp_exceed_curr_year, tmp_exceed_curr_month)) {
        tmp_exceed_monthday -= GetMonthMaxDays(tmp_exceed_curr_year, tmp_exceed_curr_month);
        tmp_exceed_curr_month++;
        tmp_exceed_curr_year += tmp_exceed_curr_month / TIMER_MONTH_COUNT;
        tmp_exceed_curr_month = tmp_exceed_curr_month % TIMER_MONTH_COUNT;
        exceed_month++;
    }

    int month;
    int tmp_month = curr_month;
    int month_len = 0;
    do {
        auto month_ret = _crontab_rule[Field::Month]->GetNextValue(tmp_month);
        if (std::get<0>(month_ret) == Return::SUCCESS) {
            month = std::get<1>(month_ret);
        } else {
            return {std::get<0>(month_ret), next_time};
        }
        month_len += (month - tmp_month);
        tmp_month = month;
    } while (month_len < exceed_month);
    if (month % exceed_month == 0) {
        if (month < TIMER_MAX_MONTH) {
            next_time = std::chrono::floor<std::chrono::months>(reftime)
                        + std::chrono::months(month)
                        + std::chrono::days(tmp_exceed_monthday)
                        + std::chrono::hours(hour % TIMER_HOUR_COUNT)
                        + std::chrono::minutes(minute % TIMER_MINUTE_COUNT)
                        + std::chrono::seconds(second % TIMER_SECOND_COUNT);
            return {Return::SUCCESS, next_time};
        }
    }
#endif
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
        if (field_index > Field::End) {
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
            return new RuleCrontab::YearRule(rule);
        case Field::Month:
            return new RuleCrontab::MonthRule(rule);
        case Field::DayOfMonth:
            return new RuleCrontab::DayOfMonthRule(rule);
        case Field::DayOfWeek:
            return new RuleCrontab::DayOfWeekRule(rule);
        case Field::Hour:
            return new RuleCrontab::HourRule(rule);
        case Field::Minute:
            return new RuleCrontab::MinuteRule(rule);
        case Field::Second:
            return new RuleCrontab::SecondRule(rule);
        default:
            break;
    }
    return nullptr;
}

}
