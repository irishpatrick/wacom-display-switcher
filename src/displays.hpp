#ifndef DISPLAYS_HPP
#define DISPLAYS_HPP

#include <string>
#include <vector>

namespace displays {

    class DisplayMetrics {
    public:
        DisplayMetrics(int index, std::string name, int width, int height, int offsetX, int offsetY)
                : index(index), name(name), width(width), height(height), offsetX(offsetX), offsetY(offsetY) {}

        void SetNvidia(bool val) {
            nvidia = val;
        }

        int index;
        std::string name;
        int width;
        int height;
        int offsetX;
        int offsetY;
        bool nvidia = true;
    };

    const std::string &GetGPUVendor();

    std::vector<DisplayMetrics> QueryDisplays();

    std::pair<int, int> QueryMousePosition();

    const std::vector<DisplayMetrics> &GetDisplays();

    int EstimateHeight(int);

}

#endif /* DISPLAYS_HPP */