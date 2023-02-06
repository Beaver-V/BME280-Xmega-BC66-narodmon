# BME280-Xmega-BC66-narodmon

Atmel (Microchip) Studio 7 проект для отправки погодных данных на сервер narodmon.com

Микроконтролер: ATXMEGA32A4U

Датчик:         BME280

Модем:          BC66NB-04-STD

Таймер RTC используется для периодического пробуждения микроконтроллера.
Проснувшись, микроконтроллер однократно запускает BME280 на измерение.
Затем программа включает BC66 и ожидает его регистрацию в NB-IoT сети.
Если регистрация в сети была успешной, производится подключение к серверу и отправка данных.
Кроме температуры, давления и влажности на сервер отправляются ещё напряжение питающей устройство батарейки и уровень сигнала сети.
После отправки данных производится отключение от сервера, выключение BC66 и уход микроконтроллера в сон.
Режимы PSM и eDRX модуля BC66 не используются.

===

Google translation:

Atmel (Microchip) Studio 7 project for sending weather data to the narodmon.com server

Microcontroller:  ATXMEGA32A4U

Sensor:           BME280

Modem:            BC66NB-04-STD

The RTC timer is used to periodically wake up the microcontroller.
After waking up, the microcontroller triggers the BME280 for measurement once.
Then the program turns on the BC66 and waits for it to register in the NB-IoT network.
If registration on the network was successful, a connection to the server is made and data is sent.
In addition to temperature, pressure and humidity, the voltage of the battery powering the device and the network signal level are also sent to the server.
After sending the data, the server is disconnected, the BC66 is turned off and the microcontroller goes to sleep.
The PSM and eDRX modes of the BC66 module are not used.
