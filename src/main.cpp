#include <Arduino.h>
#include <CodeCell.h>
#include "osc_control.h"
#include "osc_networking.h"

#ifndef ID // overwrite ID within flags eg. in platformio.ini -DID=7
#define ID 0
#endif
#define BASENAME "tracker"

static const char *TAG = "tracker"; // for logging service

OSC_NET_CONFIG networker_config;
WiFiUDP osc_net_udp;
bool osc_ready = false;

CodeCell myCodeCell;

// === OSC message definitions ===
OSC_send_msg send_msg_yaw("/yaw");
OSC_send_msg send_msg_pitch("/pitch");
OSC_send_msg send_msg_roll("/roll");
OSC_send_msg send_msg_proximity("/proximity");


// === Global sensor values ===
float roll = 0.0;
float pitch = 0.0;
float yaw = 0.0;
uint16_t proximity = 0;

// === Function prototypes ===
void resetup();
void resetup_attempt();
void send_messages();

void setup() {
  Serial.begin(115200); // Set Serial baud rate to 115200. Ensure Tools/USB_CDC_On_Boot is enabled if using Serial.

  {
    ESP_LOGD(TAG, "Set Config Defaults\n");

    networker_config.id = ID;
    strncpy(networker_config.base_name, BASENAME, OSC_NET_MAX_NAME);
    networker_config.osc_port = OSC_PORT_SEND;
    networker_config.osc_port_broadcast = OSC_PORT_BROADCAST;
    networker_config.ssid = NULL; // start with AP, then set SSID from application
    networker_config.passwd = NULL;
  }

  osc_net_init(&networker_config);

  ESP_LOGD(TAG, "CodeCell Init\n");
  myCodeCell.Init(MOTION_ROTATION | LIGHT);
}

void loop() {
  osc_net_loop();

  if (!osc_ready) {
    resetup_attempt();
    return;
  }

  osc_control_loop(osc_net_udp, networker_config.osc_baseaddress);

  if (myCodeCell.Run(10)) { 
    myCodeCell.Motion_RotationRead(roll, pitch, yaw);
    uint16_t proximity = myCodeCell.Light_ProximityRead();
    ESP_LOGI(TAG, "Roll: %.2f°, Pitch: %.2f°, Yaw: %.2f°\n", roll, pitch, yaw);
    ESP_LOGI(TAG, "Proximity: %d\n", proximity);
  }

  // send_messages();
}

void resetup()
{
  Serial.println("Resetup attempt");
  if (!osc_net_osc_resetup(osc_net_udp)) // setup udep and OSC messages for networking
    return;                              // try again later if network is responding

  // add custom parsed messages: receive_messages and get_messages
  send_msg_yaw.init(osc_net_config->osc_baseaddress);
  send_msg_pitch.init(osc_net_config->osc_baseaddress);
  send_msg_roll.init(osc_net_config->osc_baseaddress);
  osc_ready = true;
}

void resetup_attempt()
{
  static unsigned long check_time = 0ul;
  unsigned long check_interval = 10000ul; // 10 seconds

  if (millis() - check_time < check_interval)
    return;

  resetup();
  check_time = millis();
}

void send_messages()
{
  static unsigned long last_send = 0ul;
  unsigned int send_interval = 10; // 10ms
  if (millis() - last_send < send_interval)
    return;
  last_send = millis();

  send_msg_yaw.m.add(yaw);
  send_msg_pitch.m.add(pitch);
  send_msg_roll.m.add(roll);
  send_msg_proximity.m.add(proximity);

  send_msg_yaw.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
  send_msg_pitch.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
  send_msg_roll.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
  send_msg_proximity.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
}

