#ifndef WACOM_HPP
#define WACOM_HPP

#include "displays.hpp"

#include <string>
#include <vector>

namespace wacom {

    std::vector<std::string> GetDevices();

    void SetDisplay(const displays::DisplayMetrics &);

}

#endif /* WACOM_HPP */