#include "CppUTest/TestHarness.h"
#include "hw4BuildTime.h"

TEST_GROUP(hw4BuildTime)
{
  hw4BuildTime* projectBuildTime;

  void setup()
  {
    projectBuildTime = new hw4BuildTime();
  }
  void teardown()
  {
    delete projectBuildTime;
  }
};

TEST(hw4BuildTime, Create)
{
  CHECK(0 != projectBuildTime->GetDateTime());
}

