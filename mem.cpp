#include "header.h"
#include <sys/sysinfo.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

//#include <unistd.h>

double get_total_ram_memory() {
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) != 0) {
        // Error handling
        return -1.0;
    }

    double totalRam = memInfo.totalram * memInfo.mem_unit / (1024.0 * 1024.0 * 1024.0);
    return totalRam;
}

double getPhysicalMemoryUsedInGB() {
#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    if (GlobalMemoryStatusEx(&memoryStatus)) {
        return static_cast<double>(memoryStatus.ullTotalPhys) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<double>(info.totalram) * info.mem_unit / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}


double getSwapSpaceInGB() {
#ifdef _WIN32
    MEMORYSTATUSEX memoryStatus;
    memoryStatus.dwLength = sizeof(memoryStatus);

    if (GlobalMemoryStatusEx(&memoryStatus)) {
        return static_cast<double>(memoryStatus.ullTotalPageFile) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<double>(info.totalswap) * info.mem_unit / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

double getUsedSwapSpaceInGB() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
        return static_cast<double>(pmc.PagefileUsage) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        return static_cast<double>(info.freeswap) * info.mem_unit / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

double getDiskSizeInGB(const std::string& path) {
#ifdef _WIN32
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(path.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        return static_cast<double>(totalNumberOfBytes.QuadPart) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct statvfs vfs;

    if (statvfs(path.c_str(), &vfs) == 0) {
        return static_cast<double>(vfs.f_frsize * vfs.f_blocks) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

double getUsedDiskSpaceInGB(const std::string& path) {
#ifdef _WIN32
    ULARGE_INTEGER freeBytesAvailable;
    ULARGE_INTEGER totalNumberOfBytes;
    ULARGE_INTEGER totalNumberOfFreeBytes;

    if (GetDiskFreeSpaceExA(path.c_str(), &freeBytesAvailable, &totalNumberOfBytes, &totalNumberOfFreeBytes)) {
        ULONGLONG usedSpace = totalNumberOfBytes.QuadPart - totalNumberOfFreeBytes.QuadPart;
        return static_cast<double>(usedSpace) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#else
    struct statvfs vfs;

    if (statvfs(path.c_str(), &vfs) == 0) {
        unsigned long long totalSpace = vfs.f_frsize * vfs.f_blocks;
        unsigned long long freeSpace = vfs.f_frsize * vfs.f_bfree;
        unsigned long long usedSpace = totalSpace - freeSpace;

        return static_cast<double>(usedSpace) / (1024 * 1024 * 1024);
    } else {
        return -1.0; // Error
    }
#endif
}

void listProcesses() {
    DIR* dir;
    struct dirent* entry;
    std::string statusFile, line;

    dir = opendir("/proc");
    if (!dir) {
        std::cerr << "Error opening /proc directory." << std::endl;
        return;
    }

    std::cout << "List of running processes:" << std::endl;
    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            statusFile = "/proc/" + std::string(entry->d_name) + "/status";
            std::ifstream file(statusFile);
            if (file.is_open()) {
                while (std::getline(file, line)) {
                    if (line.find("Name:") == 0) {
                        std::istringstream iss(line);
                        std::string key, value;
                        iss >> key >> value;
                        std::cout << "Process ID: " << entry->d_name << " - Process Name: " << value << std::endl;
                        break;
                    }
                }
                file.close();
            }
        }
    }
    closedir(dir);
}
