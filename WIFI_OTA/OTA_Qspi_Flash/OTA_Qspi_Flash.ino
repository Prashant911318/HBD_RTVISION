#include <Arduino_Portenta_OTA.h>
#include <PortentaEthernet.h>
#include <Ethernet.h>
#include <SPI.h>


IPAddress ip(192, 168, 2, 177);
IPAddress myDns(192, 168, 2, 1);

static char const SSID[] = "Rai_Sahab";  /* your network SSID (name) */
static char const PASS[] = "00000000";  /* your network password (use for WPA, or use as key for WEP) */

static char const OTA_FILE_LOCATION[] = "https://downloads.arduino.cc/ota/OTA_Usage_Portenta.ino.PORTENTA_H7_M7.ota";

void setup()
{
  Serial.begin(115200);
  while (!Serial) {}

    while (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("Attempting to connect to the network ...");
    Ethernet.begin();
  }
  Serial.println("Connected");

  // if (WiFi.status() == WL_NO_SHIELD)
  // {
  //   Serial.println("Communication with WiFi module failed!");
  //   return;
  // }

  // int status = WL_IDLE_STATUS;
  // while (status != WL_CONNECTED)
  // {
  //   Serial.print  ("Attempting to connect to '");
  //   Serial.print  (SSID);
  //   Serial.println("'");
  //   status = WiFi.begin(SSID, PASS);
  //   delay(10000);
  // }
  Serial.print  ("You're connected to '");
  Serial.print  (WiFi.SSID());
  Serial.println("'");

  Arduino_Portenta_OTA_QSPI ota(QSPI_FLASH_FATFS_MBR, 2);
  Arduino_Portenta_OTA::Error ota_err = Arduino_Portenta_OTA::Error::None;

  if (!ota.isOtaCapable())
  {
    Serial.println("Higher version bootloader required to perform OTA.");
    Serial.println("Please update the bootloader.");
    Serial.println("File -> Examples -> STM32H747_System -> STM32H747_updateBootloader");
    return;
  }

  Serial.println("Initializing OTA storage");
  if ((ota_err = ota.begin()) != Arduino_Portenta_OTA::Error::None)
  {
    Serial.print  ("Arduino_Portenta_OTA::begin() failed with error code ");
    Serial.println((int)ota_err);
    return;
  }

  Serial.println("Starting download to QSPI ...");
  int const ota_download = ota.download(OTA_FILE_LOCATION, false /* is_https */);
  if (ota_download <= 0)
  {
    Serial.print  ("Arduino_Portenta_OTA_QSPI::download failed with error code ");
    Serial.println(ota_download);
    return;
  }
  Serial.print  (ota_download);
  Serial.println(" bytes stored.");


  Serial.println("Decompressing LZSS compressed file ...");
  int const ota_decompress = ota.decompress();
  if (ota_decompress < 0)
  {
    Serial.print("Arduino_Portenta_OTA_QSPI::decompress() failed with error code");
    Serial.println(ota_decompress);
    return;
  }
  Serial.print(ota_decompress);
  Serial.println(" bytes decompressed.");


  Serial.println("Storing parameters for firmware update in bootloader accessible non-volatile memory ...");
  if ((ota_err = ota.update()) != Arduino_Portenta_OTA::Error::None)
  {
    Serial.print  ("ota.update() failed with error code ");
    Serial.println((int)ota_err);
    return;
  }

  Serial.println("Performing a reset after which the bootloader will update the firmware.");
  Serial.println("Hint: Portenta H7 LED will blink Red-Blue-Green.");
  delay(1000); /* Make sure the serial message gets out before the reset. */
  ota.reset();
}

void loop()
{
}