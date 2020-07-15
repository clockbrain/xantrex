ESPHome custom component to read Xantrex GT2.8 solar inverter values via RS232.

![esp8266 with RS232 module directly connected](images/xantrex rs232.jpg?raw=true "esp8266 with RS232")

Standard use of the UART component within esphome but uses regular component rather than pollingcomponent due to Xantrex solar inverter taking just over 2 seconds to respond to each command. So the esphome custom component implements 2 timers: (i) delay between command cycles (5 minutes) and (ii) delay after issuing each command before attempting to read the response (2.5 seconds). Subsequent commands are issues immediately after a response is read unless it is the last command in the group.

If you align the headers on the Wemos d1 mini with the RS232 module you can plug them directly without using jumper wires. Hence using D4 for tx and D3 for rx.
