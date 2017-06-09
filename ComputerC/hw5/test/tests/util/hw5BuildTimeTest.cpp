#include "CppUTest/TestHarness.h"
#include "hw5BuildTime.h"

TEST_GROUP(hw5BuildTime)
{
  hw5BuildTime* projectBuildTime;

  void setup()
  {
    projectBuildTime = new hw5BuildTime();
  }
  void teardown()
  {
    delete projectBuildTime;
  }
};

TEST(hw5BuildTime, Create)
{
  CHECK(0 != projectBuildTime->GetDateTime());
}

