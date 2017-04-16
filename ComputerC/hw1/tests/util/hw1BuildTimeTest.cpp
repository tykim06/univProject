#include "CppUTest/TestHarness.h"
#include "hw1BuildTime.h"

TEST_GROUP(hw1BuildTime)
{
  hw1BuildTime* projectBuildTime;

  void setup()
  {
    projectBuildTime = new hw1BuildTime();
  }
  void teardown()
  {
    delete projectBuildTime;
  }
};

TEST(hw1BuildTime, Create)
{
  CHECK(0 != projectBuildTime->GetDateTime());
}

