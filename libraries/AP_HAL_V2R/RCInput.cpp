
#include "RCInput.h"
#include <logger.h>

#include <arpa/inet.h>

// MAVLINK stuff
#include <include/mavlink/v1.0/mavlink_types.h>

using namespace V2R;

extern const AP_HAL::HAL& hal;

V2RRCInput::V2RRCInput():
mavlink_fd(-1),
_valid_channels(0)
{
    for(int i = 0; i < sizeof(_periods) / sizeof(_periods[0]); i++) {
        _periods[i] = MID_CHANNEL_VALUE;
    }

    _periods[CHANNEL_THR] = MIN_CHANNEL_VALUE;

    // channels 5-8
    for(int i = 4; i < sizeof(_periods) / sizeof(_periods[0]); i++) {
        _periods[i] = MIN_CHANNEL_VALUE;
    }
}

void V2RRCInput::init(void* machtnichts)
{
    log_inf() << "[RC] Init";

    // UDP socket at port 14551
        /*
     * Mavlink UDP client socket
     */

    mavlink_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (mavlink_fd < 0) {
        log_err() << "Failed to creat UDP socket";
        return;
    };

    struct sockaddr_in serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(14551);

    if(inet_pton(AF_INET, "0.0.0.0", &serv_addr.sin_addr) <= 0)
    {
        log_err() << "Failed to parse IP address";
        return;
    }

    if (bind(mavlink_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
        log_err() << "Failed to bind UDP socket";
        return;
    }
}

uint8_t V2RRCInput::valid_channels() {
    return _valid_channels;
}

uint16_t V2RRCInput::read(uint8_t ch) {
    if (ch > MAX_CHANNELS) {
        return 0;
    }

    return _periods[ch];
}

uint8_t V2RRCInput::read(uint16_t* periods, uint8_t len) {
    if (len > MAX_CHANNELS) {
        len = MAX_CHANNELS;
    }
    for (uint8_t i = 0; i < len; i++){
        periods[i] = _periods[i];
    }

    _valid_channels = 0;

    return len;
}

bool V2RRCInput::set_overrides(int16_t *overrides, uint8_t len) {
    if (len > MAX_CHANNELS) {
        len = MAX_CHANNELS;
    }

    for (int i = 0; i < len; i++) {
        set_override(i, overrides[i]);
    }

    return true;
}

bool V2RRCInput::set_override(uint8_t channel, int16_t override) {
    if (channel < MAX_CHANNELS) {
        if ((override >= MIN_CHANNEL_VALUE) && (override <= MAX_CHANNEL_VALUE)) {
            _valid_channels++;
            _periods[channel] = override;
        }
        return true;
    } else {
        return false;
    }
}

void V2RRCInput::clear_overrides()
{
    // Overrides is the only supporeted fro V2R yet
}

static std::string ip4tostr(uint32_t addr) {
    std::stringstream ss;

    union {
        uint32_t addr_host;

        uint8_t ip4bytes[4];
    };

    addr_host = addr;

    ss << (unsigned)ip4bytes[0] << '.'
       << (unsigned)ip4bytes[1] << '.' 
       << (unsigned)ip4bytes[2] << '.'
       << (unsigned)ip4bytes[3];

    return ss.str();
};

extern mavlink_system_t mavlink_system;

static void v2r_rc_input_comm_send_buffer(mavlink_channel_t chan, const uint8_t *buf, uint8_t len)
{
    log_dbg() << "[RCIN] Want to send " << len << "bytes";
/*    
    static std::vector<uint8_t> packet_buffer;
    static std::vector<uint8_t>::value_type *write_packet_ptr;

    if (packet_buffer.capacity() < 1000) {
        packet_buffer.reserve(1000);
        write_packet_ptr = packet_buffer.data();
    }

//     fprintf(stderr, " send:%d", len);
//     fprintf(stdout, "%.*s", len, buf);
//     if (len <= 2) { // flush after packet is finalized 
//         fflush(stdout);
//     }

    if (target_udp_fd > 0) {
        // Append data to buffer
        packet_buffer.resize(packet_buffer.size() + len);
        memcpy(write_packet_ptr, buf, len);
        write_packet_ptr += len;
        
//         cerr << " sending:" << (int) len << " to " << ip4tostr(mavlink_udp_dst_addr.sin_addr.s_addr) << ":" << ntohs(mavlink_udp_dst_addr.sin_port);
//         fprintf(stderr, " send:%d", len);

        if (len <= 2) {
//             fprintf(stderr, " flush\n");
            int result = sendto(target_udp_fd, packet_buffer.data(), packet_buffer.size(), 0, (const sockaddr*)&mavlink_udp_dst_addr, sizeof(mavlink_udp_dst_addr));

            if (result != packet_buffer.size()) {
                cerr << "UDP Write error:" << result << endl;
            }

            write_packet_ptr = packet_buffer.data();
            packet_buffer.resize(0);
        }
    }*/
};


// Override transport functions
#define MAVLINK_SEND_UART_BYTES(chan, buf, len) v2r_rc_input_comm_send_buffer(chan, buf, len)

#define MAVLINK_USE_CONVENIENCE_FUNCTIONS
#include <include/mavlink/v1.0/common/mavlink.h>
#include <AP_Notify.h>

void V2RRCInput::process_input()
{
//     log_dbg() << "[RCIN] waiting for input frame";

    uint8_t udp_buf[3000];

    struct sockaddr_in src_addr;
    socklen_t addrlen = sizeof(src_addr); 

    int r = recvfrom(mavlink_fd, udp_buf, sizeof(udp_buf), 0, (struct sockaddr*)&src_addr, &addrlen);

    if (r < 0) {
        log_err() << "[MAVLINK] Failed to read UDP packet";
        return;
    } else {
//         log_dbg() << "[MAVLINK] Got packet from " << ip4tostr(src_addr.sin_addr.s_addr) << ":" << ntohs(src_addr.sin_port);
    }

    // Send data to MAVlink parser

    mavlink_message_t msg;
    mavlink_status_t status;
    status.packet_rx_drop_count = 0;

    uint8_t *p = udp_buf;

    for (int i = 0; i < r; p++, i++) {
        // Try to get a new message
        if (mavlink_parse_char(MAVLINK_COMM_0, *p, &msg, &status)) {
            // we exclude radio packets to make it possible to use the
            // CLI over the radio
//                         cerr << "[MAVLINK] Got message from " << (int)msg.sysid << ":" << (int)msg.compid
//                              << " type:" << (int)msg.msgid
//                              << " seq:" << (int)msg.seq << endl;

            switch (msg.msgid) {
                case MAVLINK_MSG_ID_RC_CHANNELS_OVERRIDE:
                {
                    
                    mavlink_rc_channels_override_t rc_channels_override;
                    
                    // static inline void mavlink_msg_rc_channels_override_decode(const mavlink_message_t* msg, mavlink_rc_channels_override_t* rc_channels_override)
                    
                    mavlink_msg_rc_channels_override_decode(&msg, &rc_channels_override);

//                     log_dbg() << "[RCIN] RC Channel override " 
//                         << " sysid:" << rc_channels_override.target_system
//                         << " compid:" << rc_channels_override.target_component
//                         << " " << rc_channels_override.chan1_raw
//                         << " " << rc_channels_override.chan2_raw
//                         << " " << rc_channels_override.chan3_raw
//                         << " " << rc_channels_override.chan4_raw
//                         << " " << rc_channels_override.chan5_raw
//                         << " " << rc_channels_override.chan6_raw
//                         << " " << rc_channels_override.chan7_raw
//                         << " " << rc_channels_override.chan8_raw;

                        // TODO protect with semaphore ?
                        set_overrides((int16_t*)&rc_channels_override.chan1_raw, 8);
                        AP_Notify::flags.rc_input = 1;

                    break;
                }

                case MAVLINK_MSG_ID_HEARTBEAT:
                {
                    log_dbg() << "[RCIN] Heartbeat from " << (int)msg.sysid << ":" << (int)msg.compid;
                    // Save

//                                 if ((mavlink_udp_dst_addr.sin_addr.s_addr != src_addr.sin_addr.s_addr) || 
//                                     (mavlink_udp_dst_addr.sin_port != src_addr.sin_port))
//                                 {
//                                     target_udp_fd = mavlink_fd;
// 
//                                     memcpy(&mavlink_udp_dst_addr, &src_addr, sizeof(src_addr));
//                                 }

                    break;
                }

                default:
                    log_inf() << "[RCIN] Unknown message id:" << (int)msg.msgid << " from " << (int)msg.sysid << ":" << (int)msg.compid;
            }
        }
    }

    if (status.packet_rx_drop_count) {
        log_inf() << "[RCIN] Some bytes are dropped: " << status.packet_rx_drop_count;
    }
    
}


