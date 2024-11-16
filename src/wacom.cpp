#include "wacom.hpp"

#include <X11/extensions/XInput.h>

#include <cassert>
#include <cstdlib>
#include <format>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <libudev.h>

inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) { return !std::isspace(ch); })
                    .base(),
            s.end());
}

namespace wacom {

    std::vector<std::string> GetDevices() {
        std::vector<std::string> deviceIds;
        std::vector<std::string> deviceNames;

        auto udevCtx = udev_new();
        if (udevCtx == nullptr) {
            // todo err
        }
        auto deviceIterator = udev_enumerate_new(udevCtx);
        if (!deviceIterator) {
            // todo err
        }
        udev_enumerate_add_match_subsystem(deviceIterator, "input");
        udev_enumerate_add_match_sysname(deviceIterator, "event*");
        const auto numDevices = udev_enumerate_scan_devices(deviceIterator);
        if (numDevices < 0) {
            std::cout << "scan err" << std::endl;
            // todo err
        }

        udev_list_entry *deviceEntry;
        udev_list_entry_foreach(deviceEntry, udev_enumerate_get_list_entry(deviceIterator)) {
            auto curDevice = udev_device_new_from_syspath(udevCtx, udev_list_entry_get_name(deviceEntry));

            const auto isTablet = udev_device_get_property_value(curDevice, "ID_INPUT_TABLET") != nullptr;
            const auto isTabletPad = udev_device_get_property_value(curDevice, "ID_INPUT_TABLET_PAD") != nullptr;
            const auto isTouchpad = udev_device_get_property_value(curDevice, "ID_INPUT_TOUCHPAD") != nullptr;
            if (!isTablet && !isTabletPad && !isTouchpad) {
                udev_device_unref(curDevice);
                continue;
            }

            const auto name = udev_device_get_property_value(curDevice, "NAME");
            const auto parentDevice = udev_device_get_parent(curDevice);
            const auto parentName = udev_device_get_property_value(parentDevice, "NAME");
            std::string nameStr;
            if (name == nullptr) {
                nameStr = std::string(parentName);
                nameStr = nameStr.substr(1, nameStr.length() - 2);
            } else {
                nameStr = std::string(parentName);
                nameStr = nameStr.substr(1, nameStr.length() - 2);;
            }
            udev_device_unref(curDevice);

            deviceNames.emplace_back(nameStr + " stylus"); // xsetwacom hack
        }

        Display *disp = XOpenDisplay(nullptr);
        int numInputDevices;
        auto inputDevices = XListInputDevices(disp, &numInputDevices);
        for (auto i = 0; i < numInputDevices; ++i)
        {
            const auto xdev = inputDevices + i;
            assert(xdev != nullptr);
            if (std::string(xdev->name) == deviceNames[0])
            {
                deviceIds.emplace_back(std::format("{}", xdev->id));
                break;
            }
        }

        XCloseDisplay(disp);

        return deviceIds;
    }

    void SetDisplay(const std::string &displayName) {
        const auto devices = GetDevices();
        for (const auto &deviceName: devices) {
            const auto result = system(
                    std::format("xsetwacom --set {} MapToOutput {}", deviceName, displayName).c_str());
            if (result != 0) {
                std::cout << "todo handle error" << std::endl;
            }
        }
    }

} // namespace wacom
