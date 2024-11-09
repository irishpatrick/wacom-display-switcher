#include "wacom.hpp"

#include <cstdlib>
#include <cstdio>
#include <format>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <locale>

inline void ltrim(std::string &s)
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch)
                                    { return !std::isspace(ch); }));
}

inline void rtrim(std::string &s)
{
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch)
                         { return !std::isspace(ch); })
                .base(),
            s.end());
}

namespace wacom
{

    std::vector<std::string> GetDevices()
    {
        std::vector<std::string> devices;

        FILE *wacomOut = popen("xsetwacom --list devices", "r");
        size_t len = 0;
        ssize_t read = 0;
        char *line = nullptr;
        while ((read = getline(&line, &len, wacomOut)) != -1)
        {
            const auto work = std::string(line);
            auto name = work.substr(0, work.find("id: "));
            ltrim(name);
            rtrim(name);
            devices.push_back(name);
        }
        if (line)
        {
            free(line);
        }
        fclose(wacomOut);

        return std::move(devices);
    }

    void SetDisplay(const std::string &displayName)
    {
        const auto devices = GetDevices();
        for (const auto &deviceName : devices)
        {
            const auto result = system(std::format("xsetwacom --set \"{}\" MapToOutput {}", deviceName, displayName).c_str());
            if (result != 0)
            {
                std::cout << "todo handle error" << std::endl;
            }
        }
    }

} // namespace wacom
