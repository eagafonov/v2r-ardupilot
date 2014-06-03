#ifndef _GPSD_CLIENT_H_
#define _GPSD_CLIENT_H_
#include <string>

class GPSDClient {
public:
    GPSDClient();

    void init(const char *gpsd_host, const char *gpsd_port);

    void thread_proc();

private:
    std::string _host;
    std::string _port;
};

#endif