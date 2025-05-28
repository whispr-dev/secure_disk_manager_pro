#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

class SystemMonitor {
public:
    static double getCpuUsage();
    static void getMemoryStats(unsigned long long& total, unsigned long long& available);
};

#endif
