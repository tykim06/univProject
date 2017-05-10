#include "CppUTest/TestHarness.h"
#include "hw3BuildTime.h"

TEST_GROUP(hw3BuildTime)
{
  hw3BuildTime* projectBuildTime;

  void setup()
  {
    projectBuildTime = new hw3BuildTime();
  }
  void teardown()
  {
    delete projectBuildTime;
  }
};

TEST(hw3BuildTime, Create)
{
  CHECK(0 != projectBuildTime->GetDateTime());
}

