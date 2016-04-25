#include <NTPClient.h>
#include <Clock.h>
#include <WiFiUdp.h>

// TODO: determine byte ordering by platform
#define HTONL(x) ((((x) & 0xff) << 24) | \
                     (((x) & 0xff00) << 8) | \
                     (((x) & 0xff0000UL) >> 8) | \
                     (((x) & 0xff000000UL) >> 24))
#define NTOHL(x) HTONL(x)

NTPClient::NTPClient() {
}

void NTPClient::begin(int localNTPPort) {
  udp.begin(localNTPPort);
}

// TODO: handle kiss of death
// TODO: verify delay/dispersion, stratum, version, mode
// TODO: verify upstream sync status
void NTPClient::parse_ntp(byte *ResponseBuffer, bool ActuallySetTime) {
  struct ntp_packet *response = (struct ntp_packet *)ResponseBuffer;
  struct timems local_ts;

  // Unix time starts on Jan 1 1970. In NTP seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t epoch = NTOHL(response->trans_time) - seventyYears;
  double epoch_ms = NTOHL(response->trans_time_fb) / 4294967.2960;
  if(ActuallySetTime) {
    local_ts.tv_sec = epoch;
    local_ts.tv_msec = epoch_ms;
    setTime_ms(&local_ts);
  }
  now_ms(&lastLocalTS);

  lastRemoteTS.tv_sec = epoch;
  lastRemoteTS.tv_msec = epoch_ms;
  lastRemoteTS.raw_millis = lastLocalTS.raw_millis - (received - sent)/2;
}

void NTPClient::sendNTPpacket(IPAddress& address) {
  struct ntp_packet *request = (struct ntp_packet *)packetBuffer;
  
  // set all bytes in the buffer to 0
  memset(request, 0, sizeof(struct ntp_packet));
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  request->byte1 = 0b11100011;   // LI=unsync, Version=4, Mode=3(client)
  request->stratum = 0; // TODO: 0 - unspecified or invalid
  request->poll = 6; // TODO: interval in 2^x seconds
  request->precision = -10;  // Peer Clock Precision in 2^x seconds
  // 8 bytes of zero for Root Delay & Root Dispersion
  request->ident = 827207988; // TODO: 52.49.78.49
  // TODO: set ref_time / trans_time?

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write((byte *)request, sizeof(struct ntp_packet));
  udp.endPacket();

  sent = millis();
}

PollStatus NTPClient::poll_reply(bool ActuallySetTime) {
  int rec_length;

  if(millis()-sent > 800) { // TODO: allow changing the timeout parameter?
    return NTP_TIMEOUT;
  }

  rec_length = udp.parsePacket();
  if(rec_length == 0) {
    return NTP_NO_PACKET;
  }

  if(rec_length != NTP_PACKET_SIZE) {
    return NTP_NO_PACKET;
  }
  // TODO: check source IP

  received = millis();  
  udp.read(packetBuffer, NTP_PACKET_SIZE);
  parse_ntp(packetBuffer, ActuallySetTime);
  return NTP_PACKET;
}

int32_t NTPClient::getLastOffset() {
  return (lastRemoteTS.tv_sec - lastLocalTS.tv_sec) * 1000 +
         (lastRemoteTS.tv_msec - lastLocalTS.tv_msec);
}

uint32_t NTPClient::getLastRTT() {
  return received - sent;
}

int32_t NTPClient::getLastOffset_RTT() {
  return getLastOffset() - getLastRTT()/2;
}

void NTPClient::getRemoteTS(struct timems *ts) {
  ts->tv_sec = lastRemoteTS.tv_sec;
  ts->tv_msec = lastRemoteTS.tv_msec;
  ts->raw_millis = lastRemoteTS.raw_millis;
}
