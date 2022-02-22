#ifndef __LOG_FORMAT_H__
#define __LOG_FORMAT_H__

#include <vector>
#include <iterator>

#include "object.h"

#define LOG_FORMAT_DEFAULT (infra::log::Format() << infra::log::Format::Field::LeftMidBrackets \
                                            << infra::log::Format::Field::Year \
                                            << infra::log::Format::Field::HorizontalLine \
                                            << infra::log::Format::Field::Month \
                                            << infra::log::Format::Field::HorizontalLine \
                                            << infra::log::Format::Field::DayOfMonth \
                                            << infra::log::Format::Field::Blank \
                                            << infra::log::Format::Field::Hour \
                                            << infra::log::Format::Field::Colon \
                                            << infra::log::Format::Field::Minute \
                                            << infra::log::Format::Field::Colon \
                                            << infra::log::Format::Field::Second \
                                            << infra::log::Format::Field::Dot \
                                            << infra::log::Format::Field::Microsecond \
                                            << infra::log::Format::Field::RightMidBrackets \
                                            << infra::log::Format::Field::Blank \
                                            << infra::log::Format::Field::Logschema \
                                            << infra::log::Format::Field::Colon \
                                            << infra::log::Format::Field::Blank)

namespace xg::timer::log {

class Format : virtual public base::Object {
public:
    enum class Field : int {
        Pid,
        Tid,
        Function,
        File,
        LineNo,
        TimeStamp,
        Year,
        Month,
        DayOfMonth,
        Week,
        DayOfWeek,
        Hour,
        Minute,
        Second,
        Millisecond,
        Microsecond,
        Nanosecond,

        Blank,
        Dot,
        Colon,
        LeftBigBrackets,
        RightBigBrackets,
        LeftMidBrackets,
        RightMidBrackets,
        LeftSmallBrackets,
        RightSmallBrackets,
        HorizontalLine,
        VerticalLine,

        Logschema,
    };
public:
    class iterator : public std::iterator<std::input_iterator_tag, std::vector<Field>::iterator> {
    public:
        friend class Format;
        iterator(std::vector<Field>::iterator&& vit) : ptr(vit) { }
        iterator() { }

        iterator(const iterator& other) : ptr(other.ptr) { }
        ~iterator() { }


        Field& operator*() { return *(ptr); }
        bool operator!=(const iterator& other) const { return (ptr != other.ptr); }
        bool operator==(const iterator& other) const { return (ptr == other.ptr); }
        const iterator& operator--() {
            ptr--;
            return *this;
        }
        const iterator operator--(int) {
            --ptr;
            return *this;
        }
        const iterator& operator++() {
            ptr++;
            return *this;
        }
        const iterator operator++(int) {
            ++ptr;
            return *this;
        }

    private:
        std::vector<Field>::iterator ptr;
    };
public:
    Format() { }
    ~Format() { }

    iterator begin () { return iterator(field_vec_.begin()); }
    iterator end () { return iterator(field_vec_.end()); }

    Format& operator<<(Field&& field) {
        field_vec_.push_back(field);
        return *this;
    }

    Format& operator>>(Field&& field) {
        field = field_vec_.back();
        field_vec_.pop_back();
        return *this;
    }
    Format& operator>>(Field& field) {
        field = field_vec_.back();
        field_vec_.pop_back();
        return *this;
    }

public:
    std::vector<Field> field_vec_;
};

}

#endif
