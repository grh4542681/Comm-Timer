#ifndef __LOG_INTERFACE_H__
#define __LOG_INTERFACE_H__

#include <map>
#include <utility>
#include <chrono>
#include <thread>
#include <sstream>
#include <ctime>
#include <iomanip>

#include "object.h"

#include "log_priority.h"
#include "log_format.h"
#include "log_device.h"

namespace infra::log {

class Interface : virtual public base::Object {
public:
    Interface() : log_device_(Device::Instance()) { }
    virtual ~Interface() { }

    template <typename ... Args> void Emergency(Args&& ... args) {
        Log(Priority::Emergency, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Alert(Args&& ... args) {
        Log(Priority::Alert, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Critical(Args&& ... args) {
        Log(Priority::Critical, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Error(Args&& ... args) {
        Log(Priority::Error, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Warning(Args&& ... args) {
        Log(Priority::Warning, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Notice(Args&& ... args) {
        Log(Priority::Notice, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Info(Args&& ... args) {
        Log(Priority::Info, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Debug(Args&& ... args) {
        Log(Priority::Debug, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Debug2(Args&& ... args) {
        Log(Priority::Debug2, std::forward<Args>(args)...);
    }
    template <typename ... Args> void Debug3(Args&& ... args) {
        Log(Priority::Debug3, std::forward<Args>(args)...);
    }

    template <typename ... Args> void Log(Priority&& priority, Args&& ... args) {
        std::stringstream log_stream;
        _build_format(log_stream, std::move(priority));
        _build_log(log_stream, std::forward<Args>(args)...);
        log_device_.GetInterface()->write(app_name_, std::move(priority), log_stream.str());
    }

protected:
    void _build_priority(std::stringstream& log_stream, Priority&& priority) {
        log_stream << app_name_;
        switch (priority) {
            case Priority::Emergency:
                log_stream << "-EMERG";
                break;
            case Priority::Alert:
                log_stream << "-ALERT";
                break;
            case Priority::Critical:
                log_stream << "-CRIT";
                break;
            case Priority::Error:
                log_stream << "-ERR";
                break;
            case Priority::Warning:
                log_stream << "-WARN";
                break;
            case Priority::Notice:
                log_stream << "-NOTICE";
                break;
            case Priority::Info:
                log_stream << "-INFO";
                break;
            case Priority::Debug:
                log_stream << "-DEBUG";
                break;
            case Priority::Debug2:
                log_stream << "-DEBUG2";
                break;
            case Priority::Debug3:
                log_stream << "-DEBUG3";
                break;
            default:
                log_stream << "-UNKNOW";
                break;
        }
    }
    void _build_format(std::stringstream& log_stream, Priority&& priority) {

        auto timepoint = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now());
        std::time_t timestamp = std::chrono::system_clock::to_time_t(timepoint);
        std::tm* tm = std::localtime(&timestamp);

        Format& format = format_default_;
        auto rule_map_it = rule_map_.find(priority);
        if (rule_map_it != rule_map_.end()) {
            format = rule_map_it->second;
        }

        for (auto format_it : format) {
            switch(format_it) {
                case Format::Field::Pid:
                {
                    log_stream << std::this_thread::get_id();
                    break;
                }
                case Format::Field::Tid:
                {
                    log_stream << std::this_thread::get_id();
                    break;
                }
                case Format::Field::Function:
                {
                    log_stream << __func__;
                    break;
                }
                case Format::Field::File:
                {
                    log_stream << __FILE__;
                    break;
                }
                case Format::Field::LineNo:
                {
                    log_stream << __LINE__;
                    break;
                }
                case Format::Field::TimeStamp:
                {
                    log_stream << timestamp;
                    break;
                }
                case Format::Field::Year:
                {
                    log_stream << (tm->tm_year + 1900);
                    break;
                }
                case Format::Field::Month:
                {
                    log_stream << std::setw(2) << std::setfill('0') << (tm->tm_mon + 1);
                    break;
                }
                case Format::Field::DayOfMonth:
                {
                    log_stream << std::setw(2) << std::setfill('0') << tm->tm_mday;
                    break;
                }
                case Format::Field::DayOfWeek:
                {
                    log_stream << std::setw(2) << std::setfill('0') << ((tm->tm_wday == 0) ? 7 : tm->tm_wday);
                    break;
                }
                case Format::Field::Hour:
                {
                    log_stream << std::setw(2) << std::setfill('0') << tm->tm_hour;
                    break;
                }
                case Format::Field::Minute:
                {
                    log_stream << std::setw(2) << std::setfill('0') << tm->tm_min;
                    break;
                }
                case Format::Field::Second:
                {
                    log_stream << std::setw(2) << std::setfill('0') << tm->tm_sec;
                    break;
                }
                case Format::Field::Millisecond:
                {
                    log_stream << std::setw(3) << std::setfill('0') << timepoint.time_since_epoch().count() % 1000000000 /1000000;
                    break;
                }
                case Format::Field::Microsecond:
                {
                    log_stream << std::setw(6) << std::setfill('0') << timepoint.time_since_epoch().count() % 1000000000 /1000;
                    break;
                }
                case Format::Field::Nanosecond:
                {
                    log_stream << std::setw(9) << std::setfill('0') << timepoint.time_since_epoch().count() % 1000000000;
                    break;
                }
                case Format::Field::Blank:
                {
                    log_stream << " ";
                    break;
                }
                case Format::Field::Dot:
                {
                    log_stream << ".";
                    break;
                }
                case Format::Field::Colon:
                {
                    log_stream << ":";
                    break;
                }
                case Format::Field::LeftBigBrackets:
                {
                    log_stream << "{";
                    break;
                }
                case Format::Field::RightBigBrackets:
                {
                    log_stream << "}";
                    break;
                }
                case Format::Field::LeftMidBrackets:
                {
                    log_stream << "[";
                    break;
                }
                case Format::Field::RightMidBrackets:
                {
                    log_stream << "]";
                    break;
                }
                case Format::Field::LeftSmallBrackets:
                {
                    log_stream << "(";
                    break;
                }
                case Format::Field::RightSmallBrackets:
                {
                    log_stream << ")";
                    break;
                }
                case Format::Field::HorizontalLine:
                {
                    log_stream << "-";
                    break;
                }
                case Format::Field::VerticalLine:
                {
                    log_stream << "|";
                    break;
                }
                case Format::Field::Logschema:
                {
                    _build_priority(log_stream, std::move(priority));
                    break;
                }
                default:
                    break;
            }
        }
    }

    template <typename T, typename ... Args> void _build_log(std::stringstream& log_stream, T&& arg, Args&& ... args) {
        log_stream << arg;
        if constexpr (sizeof...(args) > 0) {
            _build_log(log_stream, std::forward<Args>(args)...);
        }
    }
protected:
    std::string app_name_ = "LOG";
    std::map<Priority, Format> rule_map_ = {
        {Priority::Emergency, LOG_FORMAT_DEFAULT},
        {Priority::Alert,     LOG_FORMAT_DEFAULT},
        {Priority::Critical,  LOG_FORMAT_DEFAULT},
        {Priority::Error,     LOG_FORMAT_DEFAULT},
        {Priority::Warning,   LOG_FORMAT_DEFAULT},
        {Priority::Notice,    LOG_FORMAT_DEFAULT},
        {Priority::Info,      LOG_FORMAT_DEFAULT},
        {Priority::Debug,     LOG_FORMAT_DEFAULT},
        {Priority::Debug2,    LOG_FORMAT_DEFAULT},
        {Priority::Debug3,    LOG_FORMAT_DEFAULT},
    };
    Device& log_device_;
    Format format_default_ = LOG_FORMAT_DEFAULT;
};

}

#endif
