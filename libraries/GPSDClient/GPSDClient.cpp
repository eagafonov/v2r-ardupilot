#include "GPSDClient.h"

#include <gps.h>
#include <errno.h>
#include <logger.h>
#include <unistd.h>

GPSDClient::GPSDClient()
{

}

void GPSDClient::init(const char* gpsd_host, const char* gpsd_port)
{
    _host = gpsd_host;
    _port = gpsd_port;
}

void GPSDClient::thread_proc()
{
    int ret;
    
    struct gps_data_t gps_data;

    ret = gps_open("127.0.0.1", "2947", &gps_data);
    
    printf("GPS open result:%d\n", ret);

    (void) gps_stream(&gps_data, WATCH_ENABLE | WATCH_JSON, NULL);

    while (1) {
        if (gps_waiting (&gps_data, 5000000)) {
            errno = 0;
            if (gps_read (&gps_data) == -1) {
                printf("read error\n");

            } else {
                /* Display data from the GPS receiver. */
                // if (gps_data.set & ..

                printf("status: %d satelites:%d/%d fix_mode:%d long:%lf(+-%lf) lat:%lf(+-%lf) alt:%lf(+-%lf) time:%lf\n",
                       gps_data.status,
                       gps_data.satellites_used,
                       gps_data.satellites_visible,
                       gps_data.fix.mode,
                       gps_data.fix.longitude, gps_data.fix.epx,
                       gps_data.fix.latitude, gps_data.fix.epy,
                       gps_data.fix.altitude, gps_data.fix.epv,
                       gps_data.fix.time
                      );
            }
        } else {
            log_dbg() << "[GPSD] Wait taimeout";
        }
    }

    /* When you are done... */
    (void) gps_stream(&gps_data, WATCH_DISABLE, NULL);
    (void) gps_close (&gps_data);
}

