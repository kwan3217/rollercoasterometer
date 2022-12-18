//
// Created by jeppesen on 6/9/22.
//

#include <time.h>
#include <sys/time.h>
#include <cstdio>
#include <cerrno>

int main() {
  struct timeval tv;
  struct tm tm;
  tm.tm_year=2022-1900;
  tm.tm_mon=6-1;
  tm.tm_mday=9;
  tm.tm_hour=15;
  tm.tm_min=0;
  tm.tm_sec=0;
  tv.tv_sec=mktime(&tm);
  tv.tv_usec=0;
  struct timezone tz;
  tz.tz_minuteswest=0;
  tz.tz_dsttime=0;
  printf("%04d-%02d-%02dT%02d:%02d:%02d %ld\n",tm.tm_year+1900,tm.tm_mon,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,tv.tv_sec);
  int result=settimeofday(&tv,&tz);
  printf("%d ",result);
  perror(nullptr);
}