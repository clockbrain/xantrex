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

  bool command_issued = false;
  unsigned long cycle_start_time, command_issued_time;
  const int RESPONSE_WAIT = 2500; // Wait for reply to each rs232 command
  const int CYCLE_WAIT = 5*60*1000;   // Overall poll cycle for Xantrex sensor values
                                  // Should be at least as long as RESPONSE_WAIT * number of sensors
  const char *queries[11] = { 
    "KWHLIFE?\r", "KWHTODAY?\r", "PIN?\r", "POUT?\r",
    "VIN?\r", "VOUT?\r", "IIN?\r", "IOUT?\r",
    "FREQ?\r", "TIME?\r", "MEASTEMP?\r" };
  int queryNum = 0;

  void setup() override {
    // Start the timers
    cycle_start_time = millis()-CYCLE_WAIT;
    command_issued_time = cycle_start_time;
    command_issued = false;
    // Probably not necessary
    Serial.setTimeout(30);
  }

  int readline(char *buffer, int len) {
    static int pos = 0;
    int rpos;
    int readch;

    while (available() >0) {
      readch = read();
      switch (readch) {
        case '\n': // Ignore new-lines
          break;
        case '\r': // Return on CR
          rpos = pos;
          pos = 0;  // Reset position index ready for next time
          return rpos;
        default:
          if (pos < len-1) {
            buffer[pos++] = readch;
            buffer[pos] = 0;
          }
      }
    }
    // No end of line has been found, so return -1.
    return -1;
  }

  void stripfarenheit(char *buffer, int len) {
    static int pos = 0;

    // Temp response format is: C:0.0 F:32.0
    for (pos = 0; buffer[pos] != '\0' && pos < len-1; pos++) {
      switch (buffer[pos]) {
        case 'C': // Replace with space
        case ':': // Replace with space
          buffer[pos] = ' ';
          break;
        case ' ': // Terminate at space
          buffer[pos] = 0;
          pos = len-1;  // Force end of loop
          break;
        default:
          break;
      }
    }
  }


  void loop() override {
    const int max_line_length = 80;
    static char buffer[max_line_length];

    if (command_issued) {
      // wait after command before checking for a response
      if (millis() > command_issued_time + RESPONSE_WAIT) {
        ESP_LOGD("Xantrex", "RESPONSE_WAIT elapsed, available (%d)", available());
        if (readline(buffer, max_line_length)) {
          switch(queryNum) {
            case 0:
              kwhlife_sensor->publish_state(atof(buffer));
              break;
            case 1:
              kwhtoday_sensor->publish_state(atof(buffer));
              break;
            case 2:
              pin_sensor->publish_state(atof(buffer));
              break;
            case 3:
              pout_sensor->publish_state(atof(buffer));
              break;
            case 4:
              vin_sensor->publish_state(atof(buffer));
              break;
            case 5:
              vout_sensor->publish_state(atof(buffer));
              break;
            case 6:
              iin_sensor->publish_state(atof(buffer));
              break;
            case 7:
              iout_sensor->publish_state(atof(buffer));
              break;
            case 8:
              freq_sensor->publish_state(atof(buffer));
              break;
            case 9:
              time_sensor->publish_state(atof(buffer));
              break;
            case 10:
              // Temp response format is: C:0.0 F:32.0
              // Extract the celsius part
              stripfarenheit(buffer, max_line_length);
              temp_sensor->publish_state(atof(buffer));
              break;
            default:
              break;
          }
        }
        queryNum++;
        if (queryNum > 10) {
          queryNum=0;
        }
        buffer[0] = 0; // Clear the buffer
        command_issued = false;
      }
    }

    // check if polling wait time has elapsed for 1st command or 
    // subsequent command is due to be issued
    if (millis() > cycle_start_time + CYCLE_WAIT || (queryNum > 0 && !command_issued)) {
      ESP_LOGD("Xantrex", "CYCLE_WAIT elapsed");
      // Issue next command
      write_str(queries[queryNum]);
      flush();

      command_issued = true;

      // Reset poll timer and start response timer
      command_issued_time = millis();

      if (queryNum == 0) {
        // Increment the cycle timer by CYCLE_WAIT
        cycle_start_time = cycle_start_time + CYCLE_WAIT;
      }
    }

  }
};