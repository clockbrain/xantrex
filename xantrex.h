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

  bool pending = false;
  unsigned long pollTime, responseTime;
  const int responseWait = 2500; // Wait for each rs232 reply
  const int pollWait = 300000;    // How frequently to poll for sensor values
  int elapsedTime;

  const char *queries[11] = { "KWHLIFE?\r", "KWHTODAY?\r", "PIN?\r", "POUT?\r",
    "VIN?\r", "VOUT?\r", "IIN?\r", "IOUT?\r",
    "FREQ?\r", "TIME?\r", "MEASTEMP?\r" };
  int queryNum = 0;

  void setup() override {
    // Start the timer
    pollTime = millis()-pollWait;
    responseTime = pollTime;
    pending = false;
    // So the readStringUntil doesn't block if no response
    setTimeout(30);
  }

  void loop() override {

    String line = "";

    if (pending) {
      // wait after command before checking for a response
      elapsedTime = (millis() - responseTime);
      if (elapsedTime > responseWait) {
        write_str("done waiting\r");
        flush();
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
              space = line.indexOf(" ");
              celsius = line.substring(2,space-2);
              temp_sensor->publish_state(celsius.toFloat());
              break;
            default:
              break;
          }
          line = "";
        }
        queryNum++;
        if (queryNum>10) {
          queryNum=0;
        }
        pending = false;
      }
    }

    // check if polling wait time has elapsed for 1st command or 
    // subsequent command is due to be issued
    elapsedTime = (millis() - pollTime);
    if (elapsedTime > pollWait || (queryNum > 0 && !pending)) {
      // Issue next command
      write_str(queries[queryNum]);
      flush();
      pending = true;
      // Reset poll timer and start response timer
      responseTime = millis();
      // Keep the overall cycle timer aligned to the 1st command
      if (queryNum == 0) {
        pollTime = millis();
      }
    }

  }
};