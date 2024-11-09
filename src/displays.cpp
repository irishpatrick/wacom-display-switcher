#include "displays.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <iostream>
#include <format>
#include <string>
#include <algorithm>
#include <cctype>

namespace displays
{

    static std::string gpuVendor = "";
    static std::vector<DisplayMetrics> cachedMetrics;

    const std::string &GetGPUVendor()
    {
        if (gpuVendor.length() < 1)
        {
            gpuVendor = "unknown";

            FILE *lspciOut = popen("lspci | grep VGA", "r");
            size_t len = 0;
            ssize_t read = 0;
            char *line = nullptr;
            while ((read = getline(&line, &len, lspciOut)) != -1)
            {
                auto work = std::string(line);
                std::transform(work.begin(), work.end(), work.begin(), [](unsigned char c){ return std::tolower(c); });
                if (work.find("nvidia") != std::string::npos)
                {
                    gpuVendor = "nvidia";
                }
            }
            if (line)
            {
                free(line);
            }
        }

        return gpuVendor;
    }

    std::string DisplayMetrics::str() const
    {
        return std::format("{}: {},{},{},{}", this->name, this->offsetX, this->offsetY, this->width, this->height);
    }

    std::string DisplayMetrics::GetName() const
    {
        if (this->nvidia)
        {
            return std::format("HEAD-{}", this->index);
        }

        return this->name;
    }

    std::vector<DisplayMetrics> QueryDisplays()
    {
        std::vector<DisplayMetrics> metrics;

        const auto displaysToTry = 10;
        for (auto i = 0; i < displaysToTry; ++i)
        {
            Display *disp = XOpenDisplay(std::format(":{}", i).c_str());
            if (disp == nullptr)
            {
                continue;
            }

            const auto screenCount = XScreenCount(disp);
            for (auto j = 0; j < screenCount; ++j)
            {
                Screen *screen = XScreenOfDisplay(disp, j);

                Window window = XRootWindowOfScreen(screen);

                int numMonitors;
                XRRMonitorInfo *monitorInfo = XRRGetMonitors(disp, window, 1, &numMonitors);
                for (auto k = 0; k < numMonitors; ++k)
                {
                    const auto info = monitorInfo[k];

                    metrics.emplace_back(k, XGetAtomName(disp, info.name), info.width, info.height, info.x, info.y);
                    metrics.back().SetNvidia(GetGPUVendor() == "nvidia");
                }

                free(monitorInfo);
            }

            XCloseDisplay(disp);
        }

        return std::move(metrics);
    }

    const std::vector<DisplayMetrics> &GetDisplays()
    {
        if (cachedMetrics.size() == 0)
        {
            cachedMetrics = QueryDisplays();
        }

        return cachedMetrics;
    }

    int EstimateHeight(int width)
    {
        const auto displays = GetDisplays();

        std::pair<int, int> min;
        std::pair<int, int> max;
        for (const auto &monitor : displays)
        {
            min = std::pair<int, int>(std::min(min.first, monitor.offsetX), std::min(min.second, monitor.offsetY));
            max = std::pair<int, int>(std::max(max.first, monitor.offsetX + monitor.width), std::max(max.second, monitor.offsetY + monitor.height));
        }
        const double scaleX = double(width) / double(max.second - min.second);
        return int(max.second * scaleX * (9.0 / 16.0));
    }
}