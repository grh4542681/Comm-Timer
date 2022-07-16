#ifndef __LOG_FACILITY_HH__
#define __LOG_FACILITY_HH__

namespace xg::timer::log {

/**
* @brief - XG-timer log facility class
*/
class Facility {
public:
    Facility(std::string&& name) : name_(name) { }
    ~Facility() { }

    std::string& GetName() {
        return name_;
    }
private:
    std::string name_;
};

}

#endif
