#include <ESP8266WiFi.h>
// requires the clock library: https://github.com/ddrown/Arduino_Clock
#include <Clock.h>
#include <NTPClient.h>

// WiFi settings
const char ssid[] = "SSID";  // your network SSID (name)
const char pass[] = "PASS";   // your network password

// NTP settings
#define LOCAL_NTP_PORT 2390
const char* ntpServerName = "ntp.example.com"; // NTP server hostname - see http://www.pool.ntp.org/en/vendors.html
#define NTP_INTERVAL 1024  // seconds between polling the NTP server - check with your ntp operator before lowering this

IPAddress timeServerIP;
NTPClient ntp;

void wifi_setup() {
  Serial.println("Connecting to SSID");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void ntp_setup() {
  Serial.println("Starting NTP/UDP");
  ntp.begin(LOCAL_NTP_PORT);

  // note: this only looks up the hostname once, on boot.
  // you might want to look it up more frequently
  WiFi.hostByName(ntpServerName, timeServerIP);

  Serial.print("NTP IP: ");
  Serial.println(timeServerIP.toString());
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  wifi_setup();
  ntp_setup();
}

struct timems startTS;
void ntp_loop(bool ActuallySetTime) {
  PollStatus NTPstatus;
  
  ntp.sendNTPpacket(timeServerIP);

  while((NTPstatus = ntp.poll_reply(ActuallySetTime)) == NTP_NO_PACKET) { // wait until we get a response
    delay(1); // allow the background threads to run
  }
  
  if (NTPstatus == NTP_TIMEOUT) {
    Serial.println("NTP client timeout");
  } else if(NTPstatus == NTP_PACKET) {
    struct timems nowTS;
    int32_t ms_delta;
    int32_t ppm_error;
    int32_t offset = ntp.getLastOffset();
    uint32_t rtt = ntp.getLastRTT();
    
    now_ms(&nowTS);
    Serial.print("now=");
    Serial.print(nowTS.tv_sec); // current timestamp in unix form
    Serial.print(" rtt=");
    Serial.print(rtt); // round trip time

    ms_delta = (nowTS.tv_sec - startTS.tv_sec) * 1000 + (nowTS.tv_msec - startTS.tv_msec);
    if(ms_delta == 0) {
      ms_delta = 1;
    }
    ppm_error = offset * 1000000 / ms_delta;
    Serial.print(" offset=");
    Serial.print(offset); // offset between the local clock and the remote clock in ms
    Serial.print(" ppm=");
    Serial.print(ppm_error); // error in the local clock in parts per million
    Serial.print(" delta=");
    Serial.println(ms_delta); // milliseconds since the ntp loop started
  }
}

void loop() {
  time_t now_t, next_ntp;

  // first run, set the time
  ntp_loop(true);
  // schedule the next run
  next_ntp = now() + NTP_INTERVAL;
  // keep a timestamp of when we started
  now_ms(&startTS);

  while(1) {
    delay(1000);
    now_t = now();
    if(now_t >= next_ntp) {
      ntp_loop(false); // don't set the time - let the clocks drift to see the error rate
      next_ntp = now_t + NTP_INTERVAL;
    }
  }
}
