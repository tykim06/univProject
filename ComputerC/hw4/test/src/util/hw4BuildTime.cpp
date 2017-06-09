#include "hw4BuildTime.h"

hw4BuildTime::hw4BuildTime()
: dateTime(__DATE__ " " __TIME__)
{
}

hw4BuildTime::~hw4BuildTime()
{
}

const char* hw4BuildTime::GetDateTime()
{
    return dateTime;
}

