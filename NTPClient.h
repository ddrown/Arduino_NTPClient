#ifndef _NTPClient_
#define _NTPClient_

#include "Arduino.h"
//#include <inttypes.h>
#include <WiFiUdp.h>
#include <Clock.h>

extern "C" {
  struct ntp_packet {
    uint8_t byte1;
    uint8_t stratum;
    uint8_t poll;
    int8_t precision;
    uint16_t root_delay;
    uint16_t root_delay_fb;
    uint16_t dispersion;
    uint16_t dispersion_fb;
    uint32_t ident;
    uint32_t ref_time;
    uint32_t ref_time_fb;
    uint32_t org_time;
    uint32_t org_time_fb;
    uint32_t recv_time;
    uint32_t recv_time_fb;
    uint32_t trans_time;
    uint32_t trans_time_fb;
  };

  enum PollStatus {
    NTP_NO_PACKET,
    NTP_TIMEOUT,
    NTP_PACKET
  };
};

#define NTP_PACKET_SIZE sizeof(struct ntp_packet)

class NTPClient {
  public:
    NTPClient();
    void begin(int localNTPPort);
    void sendNTPpacket(IPAddress& address);
    PollStatus poll_reply(bool ActuallySetTime);
    int32_t getLastOffset();
    int32_t getLastOffset_RTT();
    uint32_t getLastRTT();
    void getRemoteTS(struct timems *ts);
  
  private:
    WiFiUDP udp;
    uint32_t sent, received;
    struct timems lastRemoteTS, lastLocalTS;

    byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

    void parse_ntp(byte *ResponseBuffer, bool ActuallySetTime);
};

#endif
