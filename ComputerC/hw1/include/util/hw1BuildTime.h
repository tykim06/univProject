#ifndef D_hw1BuildTime_H
#define D_hw1BuildTime_H

///////////////////////////////////////////////////////////////////////////////
//
//  hw1BuildTime is responsible for recording and reporting when
//  this project library was built
//
///////////////////////////////////////////////////////////////////////////////

class hw1BuildTime
  {
  public:
    explicit hw1BuildTime();
    virtual ~hw1BuildTime();
    
    const char* GetDateTime();

  private:
      
    const char* dateTime;

    hw1BuildTime(const hw1BuildTime&);
    hw1BuildTime& operator=(const hw1BuildTime&);

  };

#endif  // D_hw1BuildTime_H
