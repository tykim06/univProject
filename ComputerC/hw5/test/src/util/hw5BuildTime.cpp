#include "hw5BuildTime.h"

hw5BuildTime::hw5BuildTime()
: dateTime(__DATE__ " " __TIME__)
{
}

hw5BuildTime::~hw5BuildTime()
{
}

const char* hw5BuildTime::GetDateTime()
{
    return dateTime;
}

