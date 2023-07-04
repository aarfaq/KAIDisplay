#include <Arduino.h>
#include "nextion_hal.h"
#include "map"
#include <AsyncUDP_ESP32_W5500.h>

#define trainTotal 96
#define trainLength 30
#define ESP_CHIP_ID ((uint32_t)ESP.getEfuseMac())

AsyncUDP udp;

struct broadcastData
{
  uint32_t dev_id;
  uint8_t trainID;
  uint8_t trainNo;
  float temp;
  char customText[30];
};

typedef std::map<uint32_t, broadcastData> deviceData;
deviceData deviceCollection;
broadcastData dataToBroadcast;

bool lastETHConnection;
float tempData;

const static char trainName[trainTotal][trainLength] PROGMEM = {
    // Jawa
    "PT.KERETA API INDONESIA  ",
    "ARGO BROMO ANGGREK  ",
    "ARGO WILIS  ",
    "ARGO LAWU  ",
    "ARGO DWIPANGGA  ",
    "ARGO SINDORO  ",
    "ARGO MURIA  ",
    "ARGO JATI  ",
    "ARGO PARAHYANGAN  ",
    "GAJAYANA  ",
    "BIMA  ",
    "SEMBRANI  ",
    "TURANGGA  ",
    "TAKSAKA  ",
    "BANGUNKARTA  ",
    "PURWOJAYA  ",
    "TEGAL BAHARI  ",
    "CIREBON EKSPRES  ",
    "HARINA  ",
    "GUMARANG  ",
    "LODAYA  ",
    "SANCAKA  ",
    "MUTIARA TIMUR  ",
    "MALABAR  ",
    "MALIOBORO EKSPRES  ",
    "CIREMAI  ",
    "RANGGAJATI  ",
    "MUTIARA SELATAN  ",
    "SENJA UTAMA  ",
    "FAJAR UTAMA  ",
    "SAWUNGGALIH  ",
    "SIDOMUKTI  ",
    "MAJAPAHIT  ",
    "JAYABAYA  ",
    "JAKA TINGKIR  ",
    "MENOREH  ",
    "BOGOWONTO  ",
    "GAJAHWONG  ",
    "SINGASARI  ",
    "JOGLOKERTO  ",
    "AMBARAWA EKSPRES  ",
    "MATARMAJA  ",
    "GAYA BARU SELATAN  ",
    "BRANTAS  ",
    "KERTAJAYA  ",
    "PASUNDAN  ",
    "KAHURIPAN  ",
    "BENGAWAN  ",
    "PROGO  ",
    "LOGAWA  ",
    "KUTOJAYA UTARA  ",
    "SRI TANJUNG  ",
    "TAWANG JAYA  ",
    "KUTOJAYA SELATAN  ",
    "TAWANG ALUN  ",
    "TEGAL EKSPRES  ",
    "MAHARANI  ",
    "PROBOWANGI  ",
    "SERAYU  ",
    "KAMANDAKA  ",
    "BLORA JAYA  ",
    "KEDUNG SEPUR  ",
    "PRAMBANAN EKSPRES  ",
    "KOMUTER(DELTA EKSPRES)  ",
    "KOMUTER SULAM  ",
    "JENGGALA  ",
    "BATHARA KRESNA  ",
    "WALAHAR EKSPRES  ",
    "JATILUHUR  ",
    "PATAS BANDUNG RAYA  ",
    "LOKAL BANDUNG RAYA  ",
    "PANGRANGO  ",
    "LOKAL CIBATU  ",
    "KIANSANTANG  ",
    "KALIGUNG  ",
    "KALIJAGA  ",
    "DHOHO  ",
    "PENATARAN  ",
    "LOKAL SIDOARJO-BOJONEGORO  ",
    "LOKAL SURABAYA-KERTOSONO  ",
    "PANDANWANGI  ",
    "SILIWANGI  ",
    "LOKAL MERAK  ",
    "MATARAM PREMIUM  ",
    "JAYAKARTA PREMIUM  ",
    "MANTAB PREMIUM  ",
    "WIJAYAKUSUMA  ",
    // SUMATERA
    "SRIBILAH  ",
    "PUTRI DELI  ",
    "SIANTAR EKSPRES  ",
    "SIBINUANG  ",
    "SRIWIJAYA  ",
    "SINDANG MARGA  ",
    "BUKIT SERELO  ",
    "RAJABASA  ",
    "KRAKATAU  "};

NEXTION nextion;
uint8_t trainID = 0;
uint8_t trainNo = 0;

void prevName()
{
  if (trainID == 0)
    trainID = 95;
  else
    trainID--;
  // nextion.setName(trainName[trainID], trainID);
  nextion.setText("nameText", trainName[trainID]);
  nextion.setText("nameId", trainID);
}

void nextName()
{
  if (trainID == 95)
    trainID = 0;
  else
    trainID++;
  // nextion.setName(trainName[trainID], trainID);
  nextion.setText("nameText", trainName[trainID]);
  nextion.setText("nameId", trainID);
}

void prevNo()
{
  trainNo = nextion.getVal("noText");
  while (trainNo == -1)
  {
    trainNo = nextion.getVal("noText");
  }
  Serial.println(trainNo);
}

void nextNo()
{
  trainNo = nextion.getVal("noText");
  while (trainNo == -1)
  {
    trainNo = nextion.getVal("noText");
  }
  Serial.println(trainNo);
}

void parsePacket(AsyncUDPPacket packet)
{
  // Serial.print("UDP Packet Type: ");
  // Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
  // Serial.print(", From: ");
  // Serial.print(packet.remoteIP());
  // Serial.print(":");
  // Serial.print(packet.remotePort());
  // Serial.print(", To: ");
  // Serial.print(packet.localIP());
  // Serial.print(":");
  // Serial.print(packet.localPort());
  // Serial.print(", Length: ");
  // Serial.print(packet.length());
  // Serial.print(", Data: ");
  // Serial.write(packet.(), packet.length());
  // Serial.println();
  // //reply to the client
  // packet.printf("Got %u bytes of data", packet.length());
  if (trainNo == 0)
  {
    broadcastData *data = reinterpret_cast<broadcastData *>(packet.data());
    decltype(deviceCollection)::iterator it = deviceCollection.find(data->dev_id);
    if (it != deviceCollection.end())
    {
      // Serial.println("Found data");
      deviceCollection.erase(it);
    }
    else
    {
      // Serial.println("Data not found");
    }
    std::pair<deviceData::iterator, bool> rc = deviceCollection.insert(std::make_pair(data->dev_id, *data));
    if (!rc.second)
    {
      // Serial.print(F("Failed to update tag : "));
    }

    nextion.setText("dev_id", String(data->dev_id, HEX).c_str());
    nextion.setText("ka_name_remote", trainName[data->trainID]);
    nextion.setText("ka_no_remote", String(data->trainNo).c_str());
    nextion.setText("temp_remote", String(data->temp, 1).c_str());
    nextion.setText("sender_ip", packet.remoteIP().toString().c_str());

    Serial.print(String(data->dev_id, HEX).c_str());
    Serial.print(" | ");
    Serial.print(trainName[data->trainID]);
    Serial.print(" | ");
    Serial.print(data->trainNo);
    Serial.print(" | ");
    Serial.println(String(data->temp, 1).c_str());
  }
  else
  {
    nextion.setText("dev_id", "-");
    nextion.setText("ka_name_remote", "-");
    nextion.setText("ka_no_remote", "-");
    nextion.setText("temp_remote", "-");
    nextion.setText("sender_ip", "-");
  }
}

void startUdp()
{
  Serial.print("Async_UDPClient started @ IP address: ");
  Serial.println(ETH.localIP());

  nextion.setText("ip", ETH.localIP().toString().c_str());
  nextion.setText("broad_ip", IPAddress(239, 255, 255, 255).toString().c_str());

  if (udp.listenMulticast(IPAddress(239, 255, 255, 255), 5500))
  {
    Serial.print("UDP Listening on IP: ");
    Serial.println(ETH.localIP());

    udp.onPacket([](AsyncUDPPacket packet)
                 { parsePacket(packet); });
    // udp.print("Hello!");
  }
}

void setup()
{
  nextion.init();
  nextion.setCallback("prevName", prevName);
  nextion.setCallback("nextName", nextName);
  nextion.setCallback("prevNo", prevNo);
  nextion.setCallback("nextNo", nextNo);
  nextion.setText("nameText", trainName[trainID]);
  nextion.setText("nameId", trainID);
  nextion.setText("noText", trainNo);

  ESP32_W5500_onEvent();

  ETH.begin(MISO, MOSI, SCK, SS, 4);
}

void loop()
{
  if (ESP32_W5500_eth_connected)
  {
    if (!lastETHConnection)
    {
      startUdp();
      lastETHConnection = ESP32_W5500_eth_connected;
    }
  }
  else
  {
    lastETHConnection = ESP32_W5500_eth_connected;
  }

  nextion.update();
  static uint32_t dt = millis();
  if (millis() - dt >= random(1000, 2000))
  {
    dt = millis();
    tempData = random(235, 268) / 10.0f;
    nextion.setText("temp", String(tempData, 1).c_str());
    if (lastETHConnection && ESP32_W5500_eth_connected)
    {
      if (trainNo != 0)
      {
        dataToBroadcast.dev_id = ESP_CHIP_ID;
        dataToBroadcast.temp = tempData;
        dataToBroadcast.trainID = trainID;
        dataToBroadcast.trainNo = trainNo;
        udp.broadcast(reinterpret_cast<uint8_t *>(&dataToBroadcast), sizeof(broadcastData));
        nextion.setText("dev_id", "-");
        nextion.setText("ka_name_remote", "-");
        nextion.setText("ka_no_remote", "-");
        nextion.setText("temp_remote", "-");
      }
    }
  }
}