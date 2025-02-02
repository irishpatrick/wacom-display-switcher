#include "wacom.hpp"

#include <X11/extensions/XInput.h>

#include <cassert>
#include <cstdlib>
#include <format>
#include <libudev.h>

namespace wacom {

    std::expected<std::vector<std::string>, Error> GetDevices() {
        std::vector<std::string> deviceIds;
        std::vector<std::string> deviceNames;

        auto udevCtx = udev_new();
        if (udevCtx == nullptr) {
            return std::unexpected(Error(Error::GET_DEVICES_ERROR, "cannot get udev context"));
        }
        auto deviceIterator = udev_enumerate_new(udevCtx);
        if (!deviceIterator) {
            return std::unexpected(Error(Error::GET_DEVICES_ERROR, "cannot iterate devices"));
        }
        udev_enumerate_add_match_subsystem(deviceIterator, "input");
        udev_enumerate_add_match_sysname(deviceIterator, "event*");
        const auto numDevices = udev_enumerate_scan_devices(deviceIterator);
        if (numDevices < 0) {
            return std::unexpected(Error(Error::GET_DEVICES_ERROR, "cannot scan devices"));
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
            } else {
                nameStr = std::string(name);
            }
            nameStr = nameStr.substr(1, nameStr.length() - 2);
            udev_device_unref(curDevice);

            deviceNames.emplace_back(nameStr + " stylus"); // xsetwacom hack
        }

        Display *disp = XOpenDisplay(nullptr);
        int numInputDevices;
        const auto inputDevices = XListInputDevices(disp, &numInputDevices);
        for (auto i = 0; i < numInputDevices; ++i) {
            const auto xDev = inputDevices + i;
            assert(xDev != nullptr);
            if (std::string(xDev->name) == deviceNames[0]) {
                deviceIds.emplace_back(std::format("{}", xDev->id));
                break;
            }
        }

        XCloseDisplay(disp);

        return deviceIds;
    }

    static std::optional<Error> setMatrix(const displays::DisplayMetrics &metrics, const std::string &devIdStr) {
        Display *dpy = XOpenDisplay(nullptr); // todo err
        Atom matrixProperty = XInternAtom(dpy, "Coordinate Transformation Matrix", True); // todo err

        char *end;
        const auto devId = strtol(devIdStr.c_str(), &end, 10);
        if (*end != '\0') {
            return Error(Error::SET_MATRIX_ERROR, "malformed dev id");
        }

        XDevice *dev = XOpenDevice(dpy, devId);

        const auto fullWidth = DisplayWidth(dpy, DefaultScreen(dpy));
        const auto fullHeight = DisplayHeight(dpy, DefaultScreen(dpy));

        const float x = float(metrics.offsetX) / float(fullWidth);
        const float y = float(metrics.offsetY) / float(fullHeight);
        const float w = float(metrics.width) / float(fullWidth);
        const float h = float(metrics.height) / float(fullHeight);

        float matrix[9] = {w, 0, x, 0, h, y, 0, 0, 1};
        long xMatrix[9] = {0};
        for (auto i = 0; i < 9; ++i) {
            *(float *) (xMatrix + i) = matrix[i];
        }
        unsigned long itemLen;
        unsigned long unreadBytes;
        float *existingMatrix;
        Atom type;
        int format;
        XGetDeviceProperty(dpy, dev, matrixProperty, 0, 9, False, AnyPropertyType, &type, &format, &itemLen,
                           &unreadBytes, (unsigned char **) &existingMatrix);
        if (itemLen != 9 || format != 32 || unreadBytes > 0) {
            XFree(existingMatrix);
            XFlush(dpy);
            return Error(Error::SET_MATRIX_ERROR, "unexpected matrix format");
        }

        XChangeDeviceProperty(dpy, dev, matrixProperty, type, format, PropModeReplace, (unsigned char *) xMatrix, 9);
        XFree(existingMatrix);
        XFlush(dpy);

        return std::nullopt;
    }

    std::optional<Error> SetDisplay(const displays::DisplayMetrics &metrics) {
        const auto devices = GetDevices();
        if (!devices) {
            return devices.error();
        }
        for (const auto &deviceName: *devices) {
            const auto err = setMatrix(metrics, deviceName);
            if (err) {
                return *err;
            }
        }

        return std::nullopt;
    }
} // namespace wacom