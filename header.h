// To make sure you don't declare the function more than once by including the header multiple times.
#ifndef header_H
#define header_H

#include "imgui/lib/imgui.h"
#include "imgui/lib/backend/imgui_impl_sdl.h"
#include "imgui/lib/backend/imgui_impl_opengl3.h"
#include <stdio.h>
#include <dirent.h>
#include <vector>
#include <iostream>
#include <cmath>
// lib to read from file
#include <fstream>
// for the name of the computer and the logged in user
#include <unistd.h>
#include <limits.h>
// this is for us to get the cpu information
// mostly in unix system
// not sure if it will work in windows
#include <cpuid.h>
// this is for the memory usage and other memory visualization
// for linux gotta find a way for windows
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
// for time and date
#include <ctime>
// ifconfig ip addresses
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <map>

// add for the network
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

struct CPUStats
{
    long long int user;
    long long int nice;
    long long int system;
    long long int idle;
    long long int iowait;
    long long int irq;
    long long int softirq;
    long long int steal;
    long long int guest;
    long long int guestNice;
};

// processes `stat`
struct Proc
{
    int pid;
    string name;
    char state;
    long long int vsize;
    long long int rss;
    long long int utime;
    long long int stime;
};

struct IP4
{
    char *name;
    char addressBuffer[INET_ADDRSTRLEN];
};

struct Networks
{
    vector<IP4> ip4s;
};

struct TX
{
    int bytes;
    int packets;
    int errs;
    int drop;
    int fifo;
    int frame;
    int compressed;
    int multicast;
};

struct RX
{
    int bytes;
    int packets;
    int errs;
    int drop;
    int fifo;
    int colls;
    int carrier;
    int compressed;
};

// student TODO : system stats
string CPUinfo();
const char *getOsName();
const char *getHostName();
const char *getUserName();
const char *getCPUName();
const char* NumberofWorking();
int get_cpu_temperature();
const char* is_fan_enabled();
const char* get_fan_level();
int get_fan_speed();

// student TODO : memory and processes
double getPhysicalMemoryUsedInGB();
double get_total_ram_memory();
double getSwapSpaceInGB();
double getUsedSwapSpaceInGB();
double getDiskSizeInGB(const std::string& path);
double getUsedDiskSpaceInGB(const std::string& path);
void listProcesses(const char* searchFilter);
std::vector<double> getCPUUsage();
double getProcessMemoryUsage(int pid);

// student TODO : network
const char *getCurrentDateTimeStr();
const char *getIPv4Address(const char* iface);
TX getTXData(const std::string& interfaceName);
RX getRXData(const std::string& interfaceName);


#endif
