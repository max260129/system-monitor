#include "header.h"
#include <sys/utsname.h>
#include <sstream>

// get cpu id and information, you can use `proc/cpuinfo`
string CPUinfo()
{
    char CPUBrandString[0x40];
    unsigned int CPUInfo[4] = {0, 0, 0, 0};

    // unix system
    // for windoes maybe we must add the following
    // __cpuid(regs, 0);
    // regs is the array of 4 positions
    __cpuid(0x80000000, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);
    unsigned int nExIds = CPUInfo[0];

    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    for (unsigned int i = 0x80000000; i <= nExIds; ++i)
    {
        __cpuid(i, CPUInfo[0], CPUInfo[1], CPUInfo[2], CPUInfo[3]);

        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    string str(CPUBrandString);
    return str;
}

// getOsName, this will get the OS of the current computer
const char *getOsName()
{
#ifdef _WIN32
    return "Windows 32-bit";
#elif _WIN64
    return "Windows 64-bit";
#elif __APPLE__ || __MACH__
    return "Mac OSX";
#elif __linux__
    return "Linux";
#elif __FreeBSD__
    return "FreeBSD";
#elif __unix || __unix__
    return "Unix";
#else
    return "Other";
#endif
}
const char *getHostName() {
    static char hostname[1024];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        return hostname;
    } else {
        return "";
    }
}
const char* getUserName() {
    static char username[1024];
    if (getlogin_r(username, sizeof(username)) == 0) {
        return username;
    } else {
        return "";
    }
}

const char* getCPUName() {
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (!cpuinfo) {
        return "Unknown"; // Retourner "Unknown" si le fichier n'est pas accessible
    }

    std::string line;
    const std::string marker = "model name";
    while (std::getline(cpuinfo, line)) {
        if (line.find(marker) != std::string::npos) {
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string cpu_name = line.substr(pos + 2); // +2 pour sauter le caractère ':' et l'espace qui le suit
                static char buffer[256];
                strncpy(buffer, cpu_name.c_str(), sizeof(buffer) - 1);
                return buffer;
            }
        }
    }
    return "Unknown";
}


const char* NumberofWorking() {
    DIR* dir = opendir("/proc");
    if (dir == nullptr) {
        return "Error opening /proc";
    }

    int processCount = 0;
    struct dirent* entry;

    while ((entry = readdir(dir)) != nullptr) {
        // Si le nom du répertoire est numérique, nous avons trouvé un processus
        if (strspn(entry->d_name, "0123456789") == strlen(entry->d_name)) {
            processCount++;
        }
    }

    closedir(dir);

    // Convertir le nombre de processus en const char*
    static char buffer[50];
    snprintf(buffer, sizeof(buffer), "%d", processCount);
    return buffer;
}