#ifndef D_hw4BuildTime_H
#define D_hw4BuildTime_H

///////////////////////////////////////////////////////////////////////////////
//
//  hw4BuildTime is responsible for recording and reporting when
//  this project library was built
//
///////////////////////////////////////////////////////////////////////////////

class hw4BuildTime
  {
  public:
    explicit hw4BuildTime();
    virtual ~hw4BuildTime();
    
    const char* GetDateTime();

  private:
      
    const char* dateTime;

    hw4BuildTime(const hw4BuildTime&);
    hw4BuildTime& operator=(const hw4BuildTime&);

  };

#endif  // D_hw4BuildTime_H
