#ifndef LOGGER_H
#define LOGGER_H

#include <sstream>

    class logger
    {
    public:
        typedef enum {
            LOG_DBG,
            LOG_INF,
            LOG_WRN,
            LOG_ERR,
        } LOG_LEVEL;
         logger(LOG_LEVEL level);
        ~logger();

//          logger& operator<<(const char *s);
         logger& operator<<(const std::string &s);

         template <typename T>
         logger& operator<<(T v)
         {
             mStream << v;

             return *this;
         };

    private:
        LOG_LEVEL mLevel;
        std::stringstream mStream;
    };

    // Shortcut classes for particular log levels

    class log_dbg: public logger
    {
    public:
        log_dbg(): logger(LOG_DBG) {};
    };

    class log_wrn: public logger
    {
    public:
        log_wrn(): logger(LOG_WRN) {};
    };

    class log_err: public logger
    {
    public:
        log_err(): logger(LOG_ERR) {};
    };

    class log_inf: public logger
    {
    public:
        log_inf(): logger(LOG_INF) {};
    };

#endif // LOGGER_H