#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <sys/time.h>
#endif

/**
 * Return the number of millseconds elapsed since
 * any arbitrary point in time. This method should
 * work for any Windows machine and any POSIX 
 * complient or semi-complient machine.
 *
 * @return  Time elapsed in milliseconds
 */
double getRealTime(){
#if defined(_WIN32) || defined(_WIN64)
    return (double)(GetTickCount());
#else
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv,&tz);
    
    double secsInMilli = ((double)tv.tv_sec) * 1000;
    double usecsInMilli = tv.tv_usec / 1000;
    
    return (secsInMilli + usecsInMilli);
#endif
}