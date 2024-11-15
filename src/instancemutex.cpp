#include "instancemutex.hpp"
#include <fstream>

#include <cstring>
#include <format>
#include <cstdlib>
#include <iostream>
#include <sys/file.h>
#include <unistd.h>

void InstanceMutex::Acquire() {
    if (acquired) {
        return;
    }

    const char *home = std::getenv("HOME");

    {
        std::ofstream out("/tmp/dump", std::ios_base::out);
        out << "hi\n";
    }

    fn = std::format("{}/.wds.lock", home);
    fd = open(fn.c_str(), O_CREAT | O_RDWR | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        std::cout << "error " << strerror(errno) << std::endl;
        return;
    }
    const int lockErr = flock(fd, LOCK_EX | LOCK_NB);
    if (lockErr == -1) {
        if (errno == EAGAIN) {
            return;
        }
        std::cout << "error " << strerror(errno) << std::endl;
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
        std::cout << "error " << strerror(errno) << std::endl;
        return;
    }
    close(fd);

    unlink(fn.c_str());

    acquired = false;
}

bool InstanceMutex::IsHeld() const {
    return acquired;
}