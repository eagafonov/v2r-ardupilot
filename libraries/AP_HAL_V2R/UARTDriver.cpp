#include <AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_V2R

#include "UARTDriver.h"

#include <stdio.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <assert.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>

#include <logger.h>

extern const AP_HAL::HAL& hal;

using namespace V2R;

V2RUARTDriver::V2RUARTDriver(bool default_console) :
    device_path(NULL),
    _rd_fd(-1),
    _wr_fd(-1)
{
    if (default_console) {
        _rd_fd = 0;
        _wr_fd = 1;
        _console = true;
    }
}

/*
  set the tty device to use for this UART
 */
void V2RUARTDriver::set_device_path(const char *path)
{
    device_path = path;
}

/*
  open the tty
 */
void V2RUARTDriver::begin(uint32_t b)
{
    begin(b, 0, 0);
}

bool parse_host_port(const char *host_port, std::string &out_host, int& out_port) {
    std::string s(host_port);

    size_t pos = s.find(':');

    if (pos == std::string::npos || pos == 0) {
        return false;
    }

    std::string s_port(s.substr(pos+1));
    std::stringstream ss(s_port);
    int port = 0;

    ss >> port;

    if (port <= 0 || port > 65535) {
        log_err() << "Invalid port value " << ss.str();
        return false;
    }

    out_port = port;
    out_host = s.substr(0, pos);

    return true;
};

void V2RUARTDriver::begin(uint32_t b, uint16_t rxS, uint16_t txS)
{
    if (device_path == NULL && _console) {
        _rd_fd = 0;
        _wr_fd = 1;
        rxS = 512;
        txS = 512;
        fcntl(_rd_fd, F_SETFL, fcntl(_rd_fd, F_GETFL, 0) | O_NONBLOCK);
        fcntl(_wr_fd, F_SETFL, fcntl(_wr_fd, F_GETFL, 0) | O_NONBLOCK);
    } else if (!_initialised) {
        if (device_path == NULL) {
            return;
        }
        uint8_t retries = 0;

        while (retries < 5) {

            // Parse string 'udp:addr:port'

            if (strncmp(device_path, "udp:", 4) == 0) {
                const char *ip_addr_port = device_path + 4;

                std::string remote_host;
                int remote_port;

                if (!parse_host_port(ip_addr_port, remote_host, remote_port)) {
                    log_err() << "Failed to parse IP host:port from string '" << ip_addr_port << "'";
                    return;
                }


                log_dbg() << "Open UDP link host:" << remote_host << " port:" << remote_port;

                _rd_fd = socket(AF_INET, SOCK_DGRAM, 0);

                if (_rd_fd < 0) {
                    continue;
                };

                struct sockaddr_in serv_addr;
                memset(&serv_addr, '0', sizeof(serv_addr));

                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(remote_port);

                if(inet_pton(AF_INET, remote_host.c_str(), &serv_addr.sin_addr)<=0)
                {
                    log_err() << "Can't get IP from " << remote_host;  
                    return;
                }

                if( connect(_rd_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
                {
                    log_wrn() << "Faile to connect UDP";
                    continue;
                }
            } else {
                log_dbg() << "Open file " << device_path;

                _rd_fd = open(device_path, O_RDWR);
            }

            if (_rd_fd != -1) {
                break;
            }
            // sleep a bit and retry. There seems to be a NuttX bug
            // that can cause ttyACM0 to not be available immediately,
            // but a small delay can fix it
            hal.scheduler->delay(100);
            retries++;
        }
        _wr_fd = _rd_fd;
        if (_rd_fd == -1) {
            fprintf(stdout, "Failed to open UART device %s - %s\n",
                    device_path, strerror(errno));
            return;
        }
        if (retries != 0) {
            fprintf(stdout, "WARNING: took %u retries to open UART %s\n",
                    (unsigned)retries, device_path);
            return;
        }

        // always run the file descriptor non-blocking, and deal with
        // blocking IO in the higher level calls
        fcntl(_rd_fd, F_SETFL, fcntl(_rd_fd, F_GETFL, 0) | O_NONBLOCK);

        if (rxS < 1024) {
            rxS = 1024;
        }

        // we have enough memory to have a larger transmit buffer for
        // all ports. This means we don't get delays while waiting to
        // write GPS config packets
        if (txS < 1024) {
            txS = 1024;
        }
    }

    _initialised = false;
    while (_in_timer) hal.scheduler->delay(1);

    if (b != 0 && _rd_fd == _wr_fd) {
        // set the baud rate
        struct termios t;
        tcgetattr(_rd_fd, &t);
        cfsetspeed(&t, b);
        // disable LF -> CR/LF
        t.c_oflag &= ~ONLCR;
        tcsetattr(_rd_fd, TCSANOW, &t);
    }

    /*
      allocate the read buffer
    */
    if (rxS != 0 && rxS != _readbuf_size) {
        _readbuf_size = rxS;
        if (_readbuf != NULL) {
            free(_readbuf);
        }
        _readbuf = (uint8_t *)malloc(_readbuf_size);
        _readbuf_head = 0;
        _readbuf_tail = 0;
    }

    /*
      allocate the write buffer
    */
    if (txS != 0 && txS != _writebuf_size) {
        _writebuf_size = txS;
        if (_writebuf != NULL) {
            free(_writebuf);
        }
        _writebuf = (uint8_t *)malloc(_writebuf_size+16);
        _writebuf_head = 0;
        _writebuf_tail = 0;
    }

    if (_writebuf_size != 0 && _readbuf_size != 0) {
        _initialised = true;
    }
}

/*
  shutdown a UART
 */
void V2RUARTDriver::end()
{
    _initialised = false;
    while (_in_timer) hal.scheduler->delay(1);
    if (_rd_fd == _wr_fd && _rd_fd != -1) {
        close(_rd_fd);
    }
    _rd_fd = -1;
    _wr_fd = -1;
    if (_readbuf) {
        free(_readbuf);
        _readbuf = NULL;
    }
    if (_writebuf) {
        free(_writebuf);
        _writebuf = NULL;
    }
    _readbuf_size = _writebuf_size = 0;
    _writebuf_head = 0;
    _writebuf_tail = 0;
    _readbuf_head = 0;
    _readbuf_tail = 0;
}


void V2RUARTDriver::flush()
{
    // we are not doing any buffering, so flush is a no-op
}


/*
  return true if the UART is initialised
 */
bool V2RUARTDriver::is_initialized()
{
    return _initialised;
}


/*
  enable or disable blocking writes
 */
void V2RUARTDriver::set_blocking_writes(bool blocking)
{
    _nonblocking_writes = !blocking;
}


/*
  buffer handling macros
 */
#define BUF_AVAILABLE(buf) ((buf##_head > (_tail=buf##_tail))? (buf##_size - buf##_head) + _tail: _tail - buf##_head)
#define BUF_SPACE(buf) (((_head=buf##_head) > buf##_tail)?(_head - buf##_tail) - 1:((buf##_size - buf##_tail) + _head) - 1)
#define BUF_EMPTY(buf) (buf##_head == buf##_tail)
#define BUF_ADVANCETAIL(buf, n) buf##_tail = (buf##_tail + n) % buf##_size
#define BUF_ADVANCEHEAD(buf, n) buf##_head = (buf##_head + n) % buf##_size

/*
  do we have any bytes pending transmission?
 */
bool V2RUARTDriver::tx_pending()
{
    return !BUF_EMPTY(_writebuf);
}

/*
  return the number of bytes available to be read
 */
int16_t V2RUARTDriver::available()
{
    if (!_initialised) {
        return 0;
    }
    uint16_t _tail;
    return BUF_AVAILABLE(_readbuf);
}

/*
  how many bytes are available in the output buffer?
 */
int16_t V2RUARTDriver::txspace()
{
    if (!_initialised) {
        return 0;
    }
    uint16_t _head;
    return BUF_SPACE(_writebuf);
}

int16_t V2RUARTDriver::read()
{
    uint8_t c;
    if (!_initialised || _readbuf == NULL) {
        return -1;
    }
    if (BUF_EMPTY(_readbuf)) {
        return -1;
    }
    c = _readbuf[_readbuf_head];
    BUF_ADVANCEHEAD(_readbuf, 1);
    return c;
}

/* V2R implementations of Print virtual methods */
size_t V2RUARTDriver::write(uint8_t c)
{
    if (!_initialised) {
        return 0;
    }

    uint16_t _head;

    while (BUF_SPACE(_writebuf) == 0) {
        if (_nonblocking_writes) {
            return 0;
        }
        hal.scheduler->delay(1);
    }
    _writebuf[_writebuf_tail] = c;
    BUF_ADVANCETAIL(_writebuf, 1);
    return 1;
}

/*
  write size bytes to the write buffer
 */
size_t V2RUARTDriver::write(const uint8_t *buffer, size_t size)
{
    if (!_initialised) {
        return 0;
    }

    if (!_nonblocking_writes) {
        /*
          use the per-byte delay loop in write() above for blocking writes
         */
        size_t ret = 0;
        while (size--) {
            if (write(*buffer++) != 1) break;
            ret++;
        }
        return ret;
    }

    uint16_t _head, space;
    space = BUF_SPACE(_writebuf);
    if (space == 0) {
        return 0;
    }
    if (size > space) {
        size = space;
    }
    if (_writebuf_tail < _head) {
        // perform as single memcpy
        assert(_writebuf_tail+size <= _writebuf_size);
        memcpy(&_writebuf[_writebuf_tail], buffer, size);
        BUF_ADVANCETAIL(_writebuf, size);
        return size;
    }

    // perform as two memcpy calls
    uint16_t n = _writebuf_size - _writebuf_tail;
    if (n > size) n = size;
    assert(_writebuf_tail+n <= _writebuf_size);
    memcpy(&_writebuf[_writebuf_tail], buffer, n);
    BUF_ADVANCETAIL(_writebuf, n);
    buffer += n;
    n = size - n;
    if (n > 0) {
        assert(_writebuf_tail+n <= _writebuf_size);
        memcpy(&_writebuf[_writebuf_tail], buffer, n);
        BUF_ADVANCETAIL(_writebuf, n);
    }
    return size;
}

/*
  try writing n bytes, handling an unresponsive port
 */
int V2RUARTDriver::_write_fd(const uint8_t *buf, uint16_t n)
{
    int ret = 0;

    struct pollfd fds;
    fds.fd = _wr_fd;
    fds.events = POLLOUT;
    fds.revents = 0;

    if (poll(&fds, 1, 0) == 1) {
        ret = ::write(_wr_fd, buf, n);
    }

    if (ret > 0) {
        BUF_ADVANCEHEAD(_writebuf, ret);
        return ret;
    }

    return ret;
}

/*
  try reading n bytes, handling an unresponsive port
 */
int V2RUARTDriver::_read_fd(uint8_t *buf, uint16_t n)
{
    int ret;
    ret = ::read(_rd_fd, buf, n);
    if (ret > 0) {
        BUF_ADVANCETAIL(_readbuf, ret);
    }
    return ret;
}


/*
  push any pending bytes to/from the serial port. This is called at
  1kHz in the timer thread. Doing it this way reduces the system call
  overhead in the main task enormously.
 */
void V2RUARTDriver::_timer_tick(void)
{
    uint16_t n;

    if (!_initialised) return;

    _in_timer = true;

    // write any pending bytes
    uint16_t _tail;
    n = BUF_AVAILABLE(_writebuf);
    if (n > 0) {
        if (_tail > _writebuf_head) {
            // do as a single write
            _write_fd(&_writebuf[_writebuf_head], n);
        } else {
            // split into two writes
            uint16_t n1 = _writebuf_size - _writebuf_head;
            int ret = _write_fd(&_writebuf[_writebuf_head], n1);
            if (ret == n1 && n != n1) {
                _write_fd(&_writebuf[_writebuf_head], n - n1);
            }
        }
    }

    // try to fill the read buffer
    uint16_t _head;
    n = BUF_SPACE(_readbuf);
    if (n > 0) {
        if (_readbuf_tail < _head) {
            // one read will do
            assert(_readbuf_tail+n <= _readbuf_size);
            _read_fd(&_readbuf[_readbuf_tail], n);
        } else {
            uint16_t n1 = _readbuf_size - _readbuf_tail;
            assert(_readbuf_tail+n1 <= _readbuf_size);
            int ret = _read_fd(&_readbuf[_readbuf_tail], n1);
            if (ret == n1 && n != n1) {
                assert(_readbuf_tail+(n-n1) <= _readbuf_size);
                _read_fd(&_readbuf[_readbuf_tail], n - n1);
            }
        }
    }

    _in_timer = false;
}

#endif // CONFIG_HAL_BOARD
