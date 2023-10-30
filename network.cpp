#include "header.h"

const char* getCurrentDateTimeStr() {
    std::time_t t = std::time(nullptr);
    std::tm* tm = std::localtime(&t);
    
    static char buffer[50];
    std::strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", tm);
    
    return buffer;
}

const char* getIPv4Address(const char* iface) {
    struct ifaddrs* ifAddrStruct = nullptr;
    struct ifaddrs* ifa = nullptr;
    void* tmpAddrPtr = nullptr;
    static char addressBuffer[INET_ADDRSTRLEN];

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family == AF_INET) { // Vérifiez qu'il s'agit d'une adresse IP4
            // Est-ce l'interface que nous cherchons ?
            if (strcmp(ifa->ifa_name, iface) == 0) {
                tmpAddrPtr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
                return addressBuffer;
            }
        }
    }

    if (ifAddrStruct != nullptr) freeifaddrs(ifAddrStruct);
    return "Interface not found or no IP assigned";
}


TX getTXData(const std::string& interfaceName) {
    TX tx = {};

    std::ifstream netDevFile("/proc/net/dev");
    if(!netDevFile) {
        std::cerr << "Failed to open /proc/net/dev" << std::endl;
        return tx;
    }

    std::string line;
    while(std::getline(netDevFile, line)) {
        std::istringstream iss(line);
        std::string iface;
        RX rx;
        TX tx;

        iss >> iface;
        if(iface == interfaceName + ":") {
            iss >> rx.bytes >> rx.packets >> rx.errs >> rx.drop >> rx.fifo >> rx.colls >> rx.carrier >> rx.compressed;
            iss >> tx.bytes >> tx.packets >> tx.errs >> tx.drop >> tx.fifo >> tx.frame >> tx.compressed >> tx.multicast;
            return tx;
        }
    }
    return tx;
}

RX getRXData(const std::string& interfaceName) {
    RX rx = {};

    std::ifstream netDevFile("/proc/net/dev");
    if(!netDevFile) {
        std::cerr << "Failed to open /proc/net/dev" << std::endl;
        return rx;
    }

    std::string line;
    while(std::getline(netDevFile, line)) {
        std::istringstream iss(line);
        std::string iface;
        RX rx;
        TX tx;

        iss >> iface;
        if(iface == interfaceName + ":") {
            iss >> rx.bytes >> rx.packets >> rx.errs >> rx.drop >> rx.fifo >> rx.colls >> rx.carrier >> rx.compressed;
            iss >> tx.bytes >> tx.packets >> tx.errs >> tx.drop >> tx.fifo >> tx.frame >> tx.compressed >> tx.multicast;
            return rx;
        }
    }

    return {};
}
