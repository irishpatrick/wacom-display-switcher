#include "displays.hpp"

#include <libudev.h>

#ifdef _WDS_X11

#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>

#endif

#include <iostream>
#include <format>
#include <string>
#include <algorithm>

namespace displays {

    static std::string gpuVendor;
    static std::vector<DisplayMetrics> cachedMetrics;

    const std::string &GetGPUVendor() {
        if (gpuVendor.empty()) {
            gpuVendor = "unknown";
            auto udevCtx = udev_new();
            if (udevCtx == nullptr) {
                std::cout << "error: cannot create udev context" << std::endl;
                // todo err
            }
            auto deviceIterator = udev_enumerate_new(udevCtx);
            if (deviceIterator == nullptr) {
                std::cout << "error: cannot create udev enumerator" << std::endl;
                // todo err
            }

            udev_enumerate_add_match_subsystem(deviceIterator, "drm");
            // udev_enumerate_add_match_property(deviceIterator, "DEVTYPE", "drm_minor");
            udev_enumerate_add_match_sysname(deviceIterator, "card*");
            const auto numDevices = udev_enumerate_scan_devices(deviceIterator);
            if (numDevices < 0) {
                std::cout << "error: cannot scan for devices" << std::endl;
                // todo err
            }

            udev_list_entry *deviceEntry;
            udev_list_entry_foreach(deviceEntry, udev_enumerate_get_list_entry(deviceIterator)) {
                udev_device *dev = udev_device_new_from_syspath(udevCtx, udev_list_entry_get_name(deviceEntry));
                deviceEntry = udev_list_entry_get_next(deviceEntry);

                const char *type = udev_device_get_devtype(dev);
                if (std::string(type) != "drm_minor") {
                    udev_device_unref(dev);
                    continue;
                }

                auto parent = udev_device_get_parent(dev);
                const char *bootVga = udev_device_get_sysattr_value(parent, "boot_vga");
                if (bootVga == nullptr) {
                    udev_device_unref(dev);
                    continue;
                }
                if (std::string(bootVga) != "1") {
                    udev_device_unref(dev);
                    continue;
                }

                const char *driver = udev_device_get_driver(parent);
                if (driver == nullptr) {
                    udev_device_unref(dev);
                    break;
                }

                gpuVendor = std::string(driver);
                udev_device_unref(dev);
                break;
            }

            udev_enumerate_unref(deviceIterator);
            udev_unref(udevCtx);
        }

        return gpuVendor;
    }

    std::vector<DisplayMetrics> QueryDisplays() {
        std::vector<DisplayMetrics> metrics;

        const auto displaysToTry = 10;
        for (auto i = 0; i < displaysToTry; ++i) {
            Display *disp = XOpenDisplay(std::format(":{}", i).c_str());
            if (disp == nullptr) {
                continue;
            }

            const auto screenCount = XScreenCount(disp);
            for (auto j = 0; j < screenCount; ++j) {
                Screen *screen = XScreenOfDisplay(disp, j);

                Window window = XRootWindowOfScreen(screen);

                int numMonitors;
                XRRMonitorInfo *monitorInfo = XRRGetMonitors(disp, window, 1, &numMonitors);
                for (auto k = 0; k < numMonitors; ++k) {
                    const auto info = monitorInfo[k];

                    const char *name = XGetAtomName(disp, info.name);
                    metrics.emplace_back(k, name, info.width, info.height, info.x, info.y);
                    metrics.back().SetNvidia(GetGPUVendor() == "nvidia");
                    XFree((void *) name);
                }
                XRRFreeMonitors(monitorInfo);
            }

            XCloseDisplay(disp);
        }

        return metrics;
    }

    std::pair<int, int> QueryMousePosition() {
        Display *display = XOpenDisplay(nullptr);
        const int numScreens = XScreenCount(display);
        int rootX, rootY, winX, winY;
        unsigned int maskReturn;
        bool result;
        for (auto i = 0; i < numScreens; ++i) {
            Screen *screen = XScreenOfDisplay(display, i);
            Window window = XRootWindowOfScreen(screen);
            result = XQueryPointer(display, window, &window, &window, &rootX, &rootY, &winX, &winY, &maskReturn);
            if (result) {
                break;
            }
        }
        if (!result) {
            return {0, 0};
        }

        return {rootX, rootY};
    }

    const std::vector<DisplayMetrics> &GetDisplays() {
        if (cachedMetrics.empty()) {
            cachedMetrics = QueryDisplays();
        }

        return cachedMetrics;
    }

    int EstimateHeight(int width) {
        const auto &displays = GetDisplays();

        std::pair<int, int> min;
        std::pair<int, int> max;
        for (const auto &monitor: displays) {
            min = std::pair<int, int>(std::min(min.first, monitor.offsetX), std::min(min.second, monitor.offsetY));
            max = std::pair<int, int>(std::max(max.first, monitor.offsetX + monitor.width),
                                      std::max(max.second, monitor.offsetY + monitor.height));
        }
        const double scaleX = double(width) / double(max.second - min.second);
        return int(max.second * scaleX * (9.0 / 16.0));
    }

    DisplayMetrics GetFusedDisplay() {
        const auto& displays = GetDisplays();

        int offsetX = 0;
        int offsetY = 0;
        int width = 0;
        int height = 0;
        for (const auto& disp : displays) {
            const auto trueWidth = disp.offsetX + disp.width;
            const auto trueHeight = disp.offsetY + disp.height;
            width = std::max(trueWidth, width);
            height = std::max(trueHeight, height);
            offsetX = std::min(offsetX, disp.offsetX);
            offsetY = std::min(offsetY, disp.offsetY);
        }

        return {-1, "fused", width, height, offsetX, offsetY};
    }
}