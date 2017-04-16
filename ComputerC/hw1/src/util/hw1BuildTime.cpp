#include "hw1BuildTime.h"

hw1BuildTime::hw1BuildTime()
: dateTime(__DATE__ " " __TIME__)
{
}

hw1BuildTime::~hw1BuildTime()
{
}

const char* hw1BuildTime::GetDateTime()
{
    return dateTime;
}

