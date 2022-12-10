#ifndef __UTIL
#define __UTIL
#include<iostream>

void warn(const std::string& mes)
{
    std::cerr <<" warning "<< mes<<std::endl;
}

void error(const std::string& mes)
{
    std::cerr <<" error "<< mes<<std::endl;
    exit(1);
}

#endif
