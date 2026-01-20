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

    log_set_time(current_time);
    log_stdout_level = LOG_INFO; // Set minimum level for stdout

    log_trace("This is a trace message with timestamp: %d", 7);
    log_debug("This is a debug message with timestamp: %d", 8);
    log_info("This is an info message with timestamp: %d", 9);
    log_warn("This is a warning message with timestamp: %d", 10);
    log_error("This is an error message with timestamp: %d", 11);
    log_fatal("This is a fatal message with timestamp: %d", 12);

    if (log_file)
    {
        fclose(log_file);
    }

    return 0;
}