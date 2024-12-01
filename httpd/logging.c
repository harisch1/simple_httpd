/* logging.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <execinfo.h>
#include <string.h>
#include <stdarg.h>

char *get_time()
{
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);
    time_str[strlen(time_str) - 1] = '\0';
    return time_str;
}

// Show stacktrace till the previous function
void log_traceback()
{
    void *callstack[128];
    int i, frames = backtrace(callstack, 128);
    char **strs = backtrace_symbols(callstack, frames);
    for (i = 0; i < frames; ++i)
    {
        printf("%s\n", strs[i]);
    }
    free(strs);
}

void log_error(char* msg, ...)
{
    // red colors
    char *red = "\033[0;31m";
    char *reset = "\033[0m";
    char message[256];
    va_list args;
    va_start(args, msg);
    vsnprintf(message, sizeof(message), msg, args);
    va_end(args);
    fprintf(stderr, "%s - %s[Error]%s: %s\n", get_time(), red, reset, message);
    // log_traceback();
    fprintf(stderr, "Exiting...\n");
    // exit(1);
}

void log_info(char* msg)
{
    char *green = "\033[0;32m";
    char *reset = "\033[0m";
    fprintf(stdout, "%s - %s[INFO]%s: %s\n", get_time(), green, reset, msg);
    fflush(stdout);
}


// int main()
// {
//     char *error_args[] = {"arg1", "arg2", NULL};
//     log_info("This is an info message.");
//     log_error("This is an error message.", error_args);
//     return 0;
// }