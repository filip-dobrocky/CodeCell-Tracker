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
OSC_send_msg send_msg_ypr("/ypr");
OSC_send_msg send_msg_proximity("/proximity");
OSC_send_msg send_msg_mag("/magnetometer");
OSC_send_msg send_msg_battery("/battery");

// === Global sensor values ===
float roll = 0.0;
float pitch = 0.0;
float yaw = 0.0;
float x = 0.0;  // Magnetic field on X-axis
float y = 0.0;  // Magnetic field on Y-axis
float z = 0.0;  // Magnetic field on Z-axis
uint16_t proximity = 0;

// === Function prototypes ===
void resetup();
void resetup_attempt();
void send_messages();
void read_battery();

void setup() {
  Serial.begin(115200); // Ensure Tools/USB_CDC_On_Boot is enabled if using Serial.

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
  myCodeCell.Init(MOTION_ROTATION | LIGHT | MOTION_MAGNETOMETER);
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
    proximity = myCodeCell.Light_ProximityRead();
    myCodeCell.Motion_MagnetometerRead(x, y, z);

    ESP_LOGI(TAG, "Roll: %.2f°, Pitch: %.2f°, Yaw: %.2f°\n", roll, pitch, yaw);
    ESP_LOGI(TAG, "Magnet X: %.2f, Y: %.2f, Z: %.2f\n", x, y, z);
    ESP_LOGI(TAG, "Proximity: %d\n", proximity);
  }

  send_messages();
  read_battery();
}

void resetup()
{
  ESP_LOGI(TAG, "Resetup attempt\n");
  if (!osc_net_osc_resetup(osc_net_udp)) // setup udep and OSC messages for networking
    return;                              // try again later if network is responding

  // add custom parsed messages: receive_messages and get_messages
  send_msg_ypr.init(osc_net_config->osc_baseaddress);
  send_msg_proximity.init(osc_net_config->osc_baseaddress);
  send_msg_mag.init(osc_net_config->osc_baseaddress);

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
  unsigned int send_interval = 30u; // 10ms
  if (millis() - last_send < send_interval)
    return;
  last_send = millis();

  send_msg_ypr.m.add(yaw);
  send_msg_ypr.m.add(pitch);
  send_msg_ypr.m.add(roll);
  send_msg_proximity.m.add(proximity);
  send_msg_mag.m.add(x);
  send_msg_mag.m.add(y);
  send_msg_mag.m.add(z);

  send_msg_ypr.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
  send_msg_proximity.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
  send_msg_mag.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
}

void read_battery()
{
  static unsigned long last_battery_read = 0ul;
  unsigned long battery_read_interval = 30000ul; // 60 seconds
  if (millis() - last_battery_read < battery_read_interval)
    return;
  last_battery_read = millis();

  uint8_t battery_level = myCodeCell.BatteryLevelRead();
  ESP_LOGI(TAG, "Battery status: %d\n", battery_level);
  
  send_msg_battery.m.add(battery_level);
  send_msg_battery.send(osc_net_udp, osc_net_udp.remoteIP(), networker_config.osc_port);
}

