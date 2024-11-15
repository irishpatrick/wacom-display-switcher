#ifndef WACOM_HPP
#define WACOM_HPP

#include <string>
#include <vector>

namespace wacom {

    std::vector<std::string> GetDevices();

    void SetDisplay(const std::string &);

}

#endif /* WACOM_HPP */