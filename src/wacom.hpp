#ifndef WACOM_HPP
#define WACOM_HPP

#include "displays.hpp"

#include <expected>
#include <optional>
#include <string>
#include <vector>

namespace wacom {

    class Error {
    public:
        explicit Error(std::string_view m = "", std::string dt = "")
                : message(m), detail(std::move(dt)) {}

        explicit operator bool() const {
            return !message.empty();
        }

        std::string message;
        std::string detail;

        constexpr static std::string_view GET_DEVICES_ERROR = "failed to get devices";
        constexpr static std::string_view SET_MATRIX_ERROR = "failed to set matrix";
    };

    inline bool operator==(const Error &lhs, const std::string_view &rhs) {
        return lhs.message == rhs;
    }

    std::expected<std::vector<std::string>, Error> GetDevices();

    std::optional<Error> SetDisplay(const displays::DisplayMetrics &);

}

#endif /* WACOM_HPP */