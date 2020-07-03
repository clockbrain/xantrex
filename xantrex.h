#include "esphome.h"


class Xantrex : public Component, public UARTDevice {

 public:
  Sensor *kwhlife_sensor = new Sensor();
  Sensor *kwhtoday_sensor = new Sensor();
  Sensor *pin_sensor = new Sensor();
  Sensor *pout_sensor = new Sensor();
  Sensor *vin_sensor = new Sensor();
  Sensor *vout_sensor = new Sensor();
  Sensor *iin_sensor = new Sensor();
  Sensor *iout_sensor = new Sensor();
  Sensor *freq_sensor = new Sensor();
  Sensor *time_sensor = new Sensor();
  Sensor *temp_sensor = new Sensor();

  Xantrex(UARTComponent *parent) : Component(), UARTDevice(parent) {}

  bool response_pending = false;
  unsigned long poll_time, response_time;
  const int RESPONSE_WAIT = 2500; // Wait for reply to each rs232 command
  const int POLL_WAIT = 300000;   // Overall poll cycle for Xantrex sensor values
                                  // Should be at least as long as RESPONSE_WAIT * number of sensors
  int elapsed_time;

  const char *queries[11] = { "KWHLIFE?\r", "KWHTODAY?\r", "PIN?\r", "POUT?\r",
    "VIN?\r", "VOUT?\r", "IIN?\r", "IOUT?\r",
    "FREQ?\r", "TIME?\r", "MEASTEMP?\r" };
  int queryNum = 0;

  void setup() override {
    // Start the timers
    poll_time = millis()-POLL_WAIT;
    response_time = poll_time;
    response_pending = false;
    // So the readStringUntil doesn't block if no response
    setTimeout(30);
  }

  void loop() override {

    String line = "";

    if (response_pending) {
      // wait after command before checking for a response
      elapsed_time = (millis() - response_time);
      if (elapsed_time > RESPONSE_WAIT) {
        if (available()>0) {
          line = readStringUntil('\r');
          int space = 0;
          String celsius = "";
          switch(queryNum) {
            case 0:
              kwhlife_sensor->publish_state(line.toFloat());
              break;
            case 1:
              kwhtoday_sensor->publish_state(line.toFloat());
              break;
            case 2:
              pin_sensor->publish_state(line.toFloat());
              break;
            case 3:
              pout_sensor->publish_state(line.toFloat());
              break;
            case 4:
              vin_sensor->publish_state(line.toFloat());
              break;
            case 5:
              vout_sensor->publish_state(line.toFloat());
              break;
            case 6:
              iin_sensor->publish_state(line.toFloat());
              break;
            case 7:
              iout_sensor->publish_state(line.toFloat());
              break;
            case 8:
              freq_sensor->publish_state(line.toFloat());
              break;
            case 9:
              time_sensor->publish_state(line.toFloat());
              break;
            case 10:
              // Temp response format is: C:0.0 F:32.0
              // Extract the celsius part
              space = line.indexOf(" ");
              celsius = line.substring(2, space-2);
              temp_sensor->publish_state(celsius.toFloat());
              break;
            default:
              break;
          }
          line = "";
        }
        queryNum++;
        if (queryNum > 10) {
          queryNum=0;
        }
        response_pending = false;
      }
    }

    // check if polling wait time has elapsed for 1st command or 
    // subsequent command is due to be issued
    elapsed_time = (millis() - poll_time);
    if (elapsed_time > POLL_WAIT || (queryNum > 0 && !response_pending)) {
      // Issue next command
      write_str(queries[queryNum]);
      flush();
      response_pending = true;
      // Reset poll timer and start response timer
      response_time = millis();
      // Keep the overall cycle timer aligned to the 1st command
      if (queryNum == 0) {
        poll_time = millis();
      }
    }

  }
};