#ifndef D_hw5BuildTime_H
#define D_hw5BuildTime_H

///////////////////////////////////////////////////////////////////////////////
//
//  hw5BuildTime is responsible for recording and reporting when
//  this project library was built
//
///////////////////////////////////////////////////////////////////////////////

class hw5BuildTime
  {
  public:
    explicit hw5BuildTime();
    virtual ~hw5BuildTime();
    
    const char* GetDateTime();

  private:
      
    const char* dateTime;

    hw5BuildTime(const hw5BuildTime&);
    hw5BuildTime& operator=(const hw5BuildTime&);

  };

#endif  // D_hw5BuildTime_H
