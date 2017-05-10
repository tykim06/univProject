#include "hw3BuildTime.h"

hw3BuildTime::hw3BuildTime()
: dateTime(__DATE__ " " __TIME__)
{
}

hw3BuildTime::~hw3BuildTime()
{
}

const char* hw3BuildTime::GetDateTime()
{
    return dateTime;
}

