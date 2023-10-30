#include "header.h"
#include <sys/sysinfo.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/times.h>

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
     std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        while (std::getline(meminfo, line)) {
            if (line.find("MemAvailable:") != std::string::npos) {
                unsigned long long availableMemoryKB;
                if (sscanf(line.c_str(), "MemAvailable: %llu", &availableMemoryKB) == 1) {
                    double availableMemoryGB = static_cast<double>(availableMemoryKB) / (1024 * 1024);
                    return availableMemoryGB;
                }
            }
        }
        meminfo.close();
    }

    // If we can't read the value, return a default or error value
    return -1.0;
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
  std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        unsigned long long totalSwapKB = 0;
        unsigned long long freeSwapKB = 0;

        while (std::getline(meminfo, line)) {
            if (line.find("SwapTotal:") != std::string::npos) {
                sscanf(line.c_str(), "SwapTotal: %llu", &totalSwapKB);
            } else if (line.find("SwapFree:") != std::string::npos) {
                sscanf(line.c_str(), "SwapFree: %llu", &freeSwapKB);
            }
        }
        meminfo.close();

        if (totalSwapKB > 0) {
            double usedSwapGB = static_cast<double>(totalSwapKB - freeSwapKB) / (1024 * 1024);
            return usedSwapGB;
        }
    }

    // If we can't read the value, return a default or error value
    return -1.0;
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

double getProcessCPUUsage(pid_t processID) {
    std::string statFilePath = "/proc/" + std::to_string(processID) + "/stat";
    std::ifstream statFile(statFilePath.c_str());
    
    if (!statFile) {
        std::cerr << "Impossible d'ouvrir le fichier /proc/" << processID << "/stat." << std::endl;
        return -1.0;
    }

    std::string line;
    std::getline(statFile, line);
    std::istringstream iss(line);
    std::string token;
    for (int i = 1; i <= 13; i++) {
        iss >> token;
    }

    long utime = 0, stime = 0;
    iss >> utime >> stime;

    long totalTime = utime + stime;
    long hertz = sysconf(_SC_CLK_TCK); // Correction : multiplication par 100
    double cpuUsage = 100.0 * (totalTime / static_cast<double>(hertz));
    
    return cpuUsage;
}  

double getProcessMemoryUsage(int pid) {
    // Paths to the process's status and system's memory info
    std::string statusFile = "/proc/" + std::to_string(pid) + "/status";
    std::string meminfoFile = "/proc/meminfo";

    std::ifstream processStatus(statusFile);
    std::ifstream meminfo(meminfoFile);
    std::string line;

    long long processMemory = 0;
    long long totalMemory = 0;

    // Read the process's memory from its status file
    while (std::getline(processStatus, line)) {
        if (line.find("VmRSS:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            long long value;
            iss >> key >> value;
            processMemory = value * 1024; // Convert from kB to bytes
            break;
        }
    }

    // Read the total system memory from /proc/meminfo
    while (std::getline(meminfo, line)) {
        if (line.find("MemTotal:") != std::string::npos) {
            std::istringstream iss(line);
            std::string key;
            long long value;
            iss >> key >> value;
            totalMemory = value * 1024; // Convert from kB to bytes
            break;
        }
    }

    if (totalMemory > 0) {
        // Calculate memory usage percentage
        double memoryUsagePercentage = (static_cast<double>(processMemory) / totalMemory) * 100.0;
        return memoryUsagePercentage;
    }

    // Return -1 to indicate an error
    return -1.0;
}


void listProcesses(const char* searchFilter) {
    DIR* dir;
    struct dirent* entry;
    std::string statusFile, line;

    dir = opendir("/proc");
    if (!dir) {
        std::cerr << "Error opening /proc directory." << std::endl;
        return;
    }

    ImGui::Columns(5, "ProcessListColumns", true);

    ImGui::Text("PID");
    ImGui::NextColumn();
    ImGui::Text("Name");
    ImGui::NextColumn();
    ImGui::Text("State");
    ImGui::NextColumn();
    ImGui::Text("CPU Usage");
    ImGui::NextColumn();
    ImGui::Text("Memory Usage");
    ImGui::NextColumn();
    ImGui::Separator();

    while ((entry = readdir(dir)) != NULL) {
        if (isdigit(entry->d_name[0])) {
            statusFile = "/proc/" + std::string(entry->d_name) + "/status";
            std::ifstream file(statusFile);
            if (file.is_open()) {
                std::string processName;
                int processID = std::stoi(entry->d_name);
                double cpuUsage = getProcessCPUUsage(processID); // You need to implement this function
                double memoryUsageKB = getProcessMemoryUsage(processID); // You need to implement this function

                while (std::getline(file, line)) {
                    if (line.find("Name:") == 0) {
                        processName = line.substr(6); // Extract process name
                    }
                    else if (line.find("State:") == 0) {
                        std::string state = line.substr(7);
                        state = state.substr(1, state.size() - 2);
                        ImGui::Text("%d", processID);
                        ImGui::NextColumn();
                        ImGui::Text("%s", processName.c_str());
                        ImGui::NextColumn();
                        ImGui::Text("%s", state.c_str()+2);
                        ImGui::NextColumn();
                        ImGui::Text("%.2f%%", cpuUsage); // Display CPU usage
                        ImGui::NextColumn();
                        ImGui::Text("%.2f%%", memoryUsageKB); // Display memory usage in KB
                        ImGui::NextColumn();
                    }
                }

                file.close();
            }
        }
    }

    ImGui::Columns(1);
    closedir(dir);
}
