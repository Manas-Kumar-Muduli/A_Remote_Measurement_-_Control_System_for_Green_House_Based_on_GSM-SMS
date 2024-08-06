#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11   // DHT 11


DHT dht(DHTPIN, DHTTYPE);

const char simPIN[]   = "";

// Your phone number to send SMS: + (plus sign) and country code, for Portugal +351, followed by phone number
// SMS_TARGET Example for Portugal +351XXXXXXXXX
#define SMS_TARGET  "+918763253866"

// Configure TinyGSM library
#define TINY_GSM_MODEM_SIM800      // Modem is SIM800
#define TINY_GSM_RX_BUFFER   1024  // Set RX buffer to 1Kb

#include <Wire.h>
#include <TinyGsmClient.h>

// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define I2C_SDA              21
#define I2C_SCL              22

// Set serial for debug console (to Serial Monitor, default speed 115200)
#define SerialMon Serial
// Set serial for AT commands (to SIM800 module)
#define SerialAT  Serial1

// Define the serial console for debug prints, if needed
//#define DUMP_AT_COMMANDS

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#define IP5306_ADDR          0x75
#define IP5306_REG_SYS_CTL0  0x00

bool setPowerBoostKeepOn(int en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) {
    Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  } else {
    Wire.write(0x35); // 0x37 is default reg value
  }
  return Wire.endTransmission() == 0;
}

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  Serial.println(F("DHTxx test!"));
  pinMode(13, OUTPUT);
  pinMode(2, OUTPUT);
  dht.begin();
  // Keep power when running from battery
  Wire.begin(I2C_SDA, I2C_SCL);
  bool isOk = setPowerBoostKeepOn(1);
  SerialMon.println(String("IP5306 KeepOn ") + (isOk ? "OK" : "FAIL"));

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, HIGH);

  // Set GSM module baud rate and UART pins
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(3000);

  // Restart SIM800 module, it takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // use modem.init() if you don't need the complete restart

  // Unlock your SIM card with a PIN if needed
  if (strlen(simPIN) && modem.getSimStatus() != 3 ) {
    modem.simUnlock(simPIN);
  }
  digitalWrite(13, HIGH);
  int analog_read = analogRead(A6);
  int  sm = (100 - ((analog_read / 4095.00) * 100));

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f))
  {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  //  Serial.print("Moisture: ");
  //  Serial.print(sm);
  //  Serial.println("%");
  //  Serial.print(F("Humidity: "));
  //  Serial.print(h);
  //  Serial.print(F("%  Temperature: "));
  //  Serial.print(t);
  //  Serial.print(F("°C "));
  //  Serial.println("");


  //String smsMessage = "Temp->" + String(t) + "Humid->" + String(h) + "SoilMoist->" + String(sm);

  String smsMessage = "Temp->" + String(t) + "\n" + "Humid->" + String(h) + "\n" + "SoilMoist->" + String(sm) + "\n";

  if (modem.sendSMS(SMS_TARGET, smsMessage))
  {
    digitalWrite(2, HIGH);
    SerialMon.println("Message Sent");

  }
  else
  {
    digitalWrite(2, LOW);
    SerialMon.println("SMS failed to send");
  }
}

void loop() {
  delay(1);
}
