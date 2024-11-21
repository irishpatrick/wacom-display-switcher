#include "instancemutex.hpp"

#include <cstdlib>
#include <cstring>
#include <format>
#include <sys/file.h>
#include <unistd.h>

void InstanceMutex::Acquire() {
    if (acquired) {
        return;
    }

    const char *home = std::getenv("HOME");

    fn = std::format("{}/.wds.lock", home);
    fd = open(fn.c_str(), O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        error = InstanceMutexError(InstanceMutexError::CREATE_LOCKFILE_FAIL, strerror(errno));
        return;
    }
    const int lockErr = flock(fd, LOCK_EX | LOCK_NB);
    if (lockErr == -1) {
        if (errno == EAGAIN) {
            return;
        }
        error = InstanceMutexError(InstanceMutexError::LOCK_FAIL, strerror(errno));
        return;
    }

    acquired = true;
}

void InstanceMutex::Release() {
    if (!acquired) {
        return;
    }

    const int lockErr = flock(fd, LOCK_UN);
    if (lockErr == -1) {
        error = InstanceMutexError(InstanceMutexError::UNLOCK_FAIL, strerror(errno));
        return;
    }
    close(fd);

    unlink(fn.c_str());

    acquired = false;
}

std::optional<InstanceMutexError> InstanceMutex::IsHeld() const {
    if (error) {
        return error;
    }

    if (!acquired) {
        return InstanceMutexError(InstanceMutexError::ALREADY_HELD);
    }

    return std::nullopt;
}