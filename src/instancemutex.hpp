#ifndef INSTANCEMUTEX_HPP
#define INSTANCEMUTEX_HPP

#include <optional>
#include <string>
#include <utility>

class InstanceMutexError {
public:
    explicit InstanceMutexError(std::string_view m = "", std::string dt = "") : message(m), detail(std::move(dt)) {}

    std::string message;
    std::string detail;

    explicit operator bool() const {
        return !message.empty();
    }

    constexpr static std::string_view CREATE_LOCKFILE_FAIL = "create lockfile failed";
    constexpr static std::string_view ALREADY_HELD = "already held";
    constexpr static std::string_view UNLOCK_FAIL = "unlock failed";
    constexpr static std::string_view LOCK_FAIL = "lock failed";
};

inline bool operator==(const InstanceMutexError &lhs, const std::string_view &rhs) {
    return lhs.message == rhs;
}

class InstanceMutex {
public:
    InstanceMutex() {
        Acquire();
    }

    ~InstanceMutex() {
        Release();
    }

    void Acquire();

    void Release();

    [[nodiscard]] std::optional<InstanceMutexError> IsHeld() const;

private:
    int fd{-1};
    bool acquired{false};
    InstanceMutexError error;
    std::string fn;
};

#endif /* INSTANCEMUTEX */