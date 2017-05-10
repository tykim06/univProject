#ifndef D_hw3BuildTime_H
#define D_hw3BuildTime_H

///////////////////////////////////////////////////////////////////////////////
//
//  hw3BuildTime is responsible for recording and reporting when
//  this project library was built
//
///////////////////////////////////////////////////////////////////////////////

class hw3BuildTime
  {
  public:
    explicit hw3BuildTime();
    virtual ~hw3BuildTime();
    
    const char* GetDateTime();

  private:
      
    const char* dateTime;

    hw3BuildTime(const hw3BuildTime&);
    hw3BuildTime& operator=(const hw3BuildTime&);

  };

#endif  // D_hw3BuildTime_H
