#include "nextion_hal.h"
#include "NextionX2.h"
#include "map"

#define TX 27
#define RX 26

NextionComPort next;
NextionComponent prevName(next, 0, 20);
NextionComponent nextName(next, 0, 22);
NextionComponent prevNo(next, 0, 21);
NextionComponent nextNo(next, 0, 23);
NextionComponent nameText(next, 0, 5);
NextionComponent customNameTxt(next, 0, 28);
NextionComponent noText(next, 0, 44);
NextionComponent nameId(next, 0, 43);
NextionComponent ip(next, 0, 14);
NextionComponent sender_ip(next, 0, 15);
NextionComponent led1(next, 0, 16);
NextionComponent led2(next, 0, 19);
NextionComponent broad_ip(next, 0, 17);
NextionComponent temp(next, 0, 18);
NextionComponent dev_id(next, 0, 33);
NextionComponent ka_name_remote(next, 0, 32);
NextionComponent ka_no_remote(next, 0, 34);
NextionComponent temp_remote(next, 0, 36);

std::map<const char *, NextionComponent *> objMap;

NEXTION::NEXTION() : uploadTime(0),
                     upload(false),
                     baud(0),
                     baudCmd(false),
                     connectEditor(false),
                     loopTime(0) {}

void NEXTION::setCallback(const char *var, void (*cb)() = nullptr)
{
    if (cb && objMap[var] && (!this->upload && !this->connectEditor))
    {
        Serial.print("callback '");
        Serial.println(var);
        objMap[var]->release(cb);
    }
}

// void NEXTION::setCallbackNextName(void (*cb)() = nullptr)
// {
//     if (cb)
//     {
//         nextName.release(cb);
//     }
// }

// void NEXTION::setCallbackPrevNo(void (*cb)() = nullptr)
// {
//     if (cb)
//     {
//         prevNo.release(cb);
//     }
// }

// void NEXTION::setCallbackNextNo(void (*cb)() = nullptr)
// {
//     if (cb)
//     {
//         nextNo.release(cb);
//     }
// }

String NEXTION::getText(const char *var)
{
    if (objMap[var] && (!this->upload && !this->connectEditor))
    {
        return objMap[var]->text();
    }
    else
    {
        return "";
    }
}

int32_t NEXTION::getVal(const char *var)
{
    if (objMap[var] && (!this->upload && !this->connectEditor))
    {
        return objMap[var]->value();
    }
    else
    {
        return -1;
    }
}

void NEXTION::setName(const char *txt, int32_t val)
{
    nameText.attribute("txt", txt);
    nameId.attribute("val", val);
}

void NEXTION::setText(const char *var, const char *txt)
{
    if (objMap[var] && (!this->upload && !this->connectEditor))
    {
        objMap[var]->attribute("txt", txt);
    }
}

void NEXTION::setText(const char *var, int32_t val)
{
    if (objMap[var] && (!this->upload && !this->connectEditor))
    {
        objMap[var]->attribute("val", val);
    }
}

void NEXTION::init()
{
    next.begin(Serial2, 9600);
    // Serial2.begin(9600, SERIAL_8N1, RX, TX);
    Serial2.setPins(RX, TX);
    Serial2.setRxFIFOFull(112);
    // delay(100);
    // next.command("");
    // next.command("bkcmd=0");
    next.debug(Serial, 9600);
    Serial.setRxFIFOFull(112);
    delay(100);
    next.command("");
    next.command("bkcmd=3");
    this->uploadTime = millis();
    this->upload = false;
    this->baud = 0;
    this->connectEditor = false;
    this->loopTime = millis();
    objMap["nameText"] = &nameText;
    objMap["nameId"] = &nameId;
    objMap["prevName"] = &prevName;
    objMap["nextName"] = &nextName;
    objMap["prevNo"] = &prevNo;
    objMap["nextNo"] = &nextNo;
    objMap["noText"] = &noText;
    objMap["ip"] = &ip;
    objMap["sender_ip"] = &sender_ip;
    objMap["led1"] = &led1;
    objMap["led2"] = &led2;
    objMap["broad_ip"] = &broad_ip;
    objMap["temp"] = &temp;
    objMap["dev_id"] = &dev_id;
    objMap["ka_name_remote"] = &ka_name_remote;
    objMap["ka_no_remote"] = &ka_no_remote;
    objMap["temp_remote"] = &temp_remote;
}

void NEXTION::update()
{
    // if (!this->upload)
    // {
    //     if (millis() - this->loopTime >= 00)
    //         this->loopTime = millis();
    //     else
    //         return;
    // }

    if (Serial.available())
    {
        if (this->upload)
            this->uploadTime = millis();

        int length = Serial.available();
        int dly = 0;
        char buf[length];
        Serial.readBytes(buf, length);
        // Serial.print(length);

        if (strstr(buf, "connect") != NULL)
        {
            this->connectEditor = true;
            this->uploadTime = millis();
            // Serial.println("Connect to editor");
            // dly = 100;
            // return;
        }

        if (strstr(buf, "whmi-wri") != NULL || strstr(buf, "whmi-wris") != NULL)
        {
            auto idx1 = strchr(buf, ',');
            auto idx2 = strrchr(buf, ',');
            if (idx1 != NULL && idx2 != NULL)
            {
                int iidx1 = (int)(idx1 - buf + 1);
                int iidx2 = (int)(idx2 - buf);
                char baudChar[iidx2 - iidx1];
                strncpy(baudChar, reinterpret_cast<const char *>(buf) + iidx1, iidx2 - iidx1);
                this->baud = atoi(baudChar);
                if (this->baud)
                {
                    this->baudCmd = true;
                    dly = 100;
                    // Serial.println("SetBaud");
                }
            }
        }
        if (strstr(buf, "changeBaud") != NULL)
        {
            Serial.print("Change Baudrate -> ");
            auto idx = strchr(buf, '=');
            if (idx != NULL)
            {
                int iidx = (int)(idx - buf + 1);
                char baudChar[length];
                strncpy(baudChar, reinterpret_cast<const char *>(buf) + iidx, length - iidx);
                this->baud = atoi(baudChar);
                Serial.println(this->baud);
                Serial.updateBaudRate(this->baud);
                Serial2.updateBaudRate(this->baud);
                this->baud = 0;
                return;
            }
        }
        // for(int i = 0; i<= length; i++){
        //     Serial2.write(buf[i]);
        //     delay(1);
        // }
        Serial2.flush();
        Serial2.write(buf, length);
        // Serial2.write(buf, length);
        delay(dly);
    }

    if (!this->upload && !this->connectEditor)
    {
        if (millis() - this->loopTime >= 100)
        {
            this->loopTime = millis();
            next.update();
        }
    }
    else
    {
        while (Serial2.available())
        {
            char c = Serial2.read();
            Serial.write(c);
            if (/*(c == 0xFF || c == 0x05) &&*/this->baudCmd)
            {
                this->upload = true;
                this->uploadTime = millis();
                Serial.updateBaudRate(this->baud);
                Serial2.updateBaudRate(this->baud);
                this->baudCmd = 0;
                this->baud = 0;
                // delay(200);
            }
        }
    }

    if (millis() - this->uploadTime >= 5000 && (this->upload || this->connectEditor))
    {
        this->upload = false;
        this->baudCmd = false;
        this->connectEditor = false;
        this->baud = 0;
        Serial2.updateBaudRate(9600);
        Serial.updateBaudRate(9600);
        // delay(200);
    }
}