#ifndef __LOG_FORMAT_HH__
#define __LOG_FORMAT_HH__

#include <vector>
#include <iterator>

#define LOG_FORMAT_DEFAULT (::xg::timer::log::Format() << ::xg::timer::log::Format::Field::LeftMidBrackets \
                                            << ::xg::timer::log::Format::Field::Year \
                                            << ::xg::timer::log::Format::Field::HorizontalLine \
                                            << ::xg::timer::log::Format::Field::Month \
                                            << ::xg::timer::log::Format::Field::HorizontalLine \
                                            << ::xg::timer::log::Format::Field::DayOfMonth \
                                            << ::xg::timer::log::Format::Field::Blank \
                                            << ::xg::timer::log::Format::Field::Hour \
                                            << ::xg::timer::log::Format::Field::Colon \
                                            << ::xg::timer::log::Format::Field::Minute \
                                            << ::xg::timer::log::Format::Field::Colon \
                                            << ::xg::timer::log::Format::Field::Second \
                                            << ::xg::timer::log::Format::Field::Dot \
                                            << ::xg::timer::log::Format::Field::Microsecond \
                                            << ::xg::timer::log::Format::Field::RightMidBrackets \
                                            << ::xg::timer::log::Format::Field::Blank \
                                            << ::xg::timer::log::Format::Field::Logschema \
                                            << ::xg::timer::log::Format::Field::Colon \
                                            << ::xg::timer::log::Format::Field::Blank)

namespace xg::timer::log {

/**
* @brief - XG-timer logger format class
*/
class Format {
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
