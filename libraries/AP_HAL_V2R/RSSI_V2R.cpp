#include <stdint.h>

#include <fstream>
#include <logger.h>

uint8_t read_rssi_v2r() {
    uint8_t retval = 0;

    std::ifstream f("/proc/net/wireless");

    // Sample output of `cat /proc/net/wireless`
    //
    //Inter-| sta-|   Quality        |   Discarded packets               | Missed | WE
    // face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22
    // wlan0: 0000   70.  -39.  -256        0      0      0      0      0        0

    // Parse first iface only assuming there is only one WiFi module attached to V2R
    std::string s;

    // Skip two lines of header
    std::getline(f, s);
    std::getline(f, s);

    f >> s;
//     log_dbg() << "iface:" << s;

    int status;
    f >> status;
//     log_dbg() << "status:" << status;

    int link;
    f >> link;
//     log_dbg() << "link:" << link;

    if (f.good()) {
        retval = (uint8_t) link;
    } else {
        log_wrn() << "[RSSI] Failed to parse iface status line";
    }

    return retval;
};