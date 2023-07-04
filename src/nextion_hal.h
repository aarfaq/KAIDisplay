#ifndef _nextion_hal_h_
#define _nextion_hal_h_

#include <Arduino.h>

class NEXTION
{
public:
    NEXTION();
    void init();
    void update();
    void setCallback(const char* var, void (*cb)());
    String getText(const char* var);
    int32_t getVal(const char* var);
    void setName(const char* txt, int32_t val);
    void setText(const char* var, const char* txt);
    void setText(const char* var, int32_t val);

protected:
    uint32_t uploadTime, loopTime;
    bool upload, baudCmd, connectEditor;
    int baud;
};

#endif