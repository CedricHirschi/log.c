#include <time.h>
#include <stdio.h>

#define LOG_IMPLEMENTATION
#define LOG_USE_COLOR
#include "log.h"

uint32_t current_time()
{
    // Return time in ms since start of the program
    return (uint32_t)(clock() * 1000 / CLOCKS_PER_SEC);
}

int main()
{
    log_set_time(current_time);

    FILE *log_file = fopen("test.log", "w");
    if (log_file)
    {
        log_add_fp(log_file, LOG_TRACE);
    }

    log_trace("This is a trace message: %d", 1);
    log_debug("This is a debug message: %d", 2);
    log_info("This is an info message: %d", 3);
    log_warn("This is a warning message: %d", 4);
    log_error("This is an error message: %d", 5);
    log_fatal("This is a fatal message: %d", 6);

    return 0;
}