#include <MFRC522.h>
#include <tusb.h>
#include <Adafruit_NeoPixel.h>
#include <hardware/pwm.h>

#include <config/io.h>
#include <config/led.h>
#include <config/hid.h>

MFRC522 mfrc522(CS_PIN, RESET_PIN);

uint8_t report[6] = {0};
Adafruit_USBD_HID usb_hid;
uint8_t const desc_hid_report[] = { TUD_HID_REPORT_DESC_KEYBOARD() };

uint slice_num;

void buzzer_init() {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_enabled(slice_num, true);
}

void buzzer_tone(uint freq, uint ms) {
    if (freq == 0) {
        pwm_set_gpio_level(BUZZER_PIN, 0);
        sleep_ms(ms);
        return;
    }
    uint32_t clock = 125000000;
    uint32_t wrap = clock / freq;
    pwm_set_wrap(slice_num, wrap);
    pwm_set_gpio_level(BUZZER_PIN, wrap / 2);
    sleep_ms(ms);
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

Adafruit_NeoPixel pixels(PIXEL_CNT, PIXEL_PIN, PIXEL_ORDER + PIXEL_FREQ);

uint8_t hexCharToKeycode(char c) {
  switch (c) {
    case '0': return 0x27;
    case '1': return 0x1E;
    case '2': return 0x1F;
    case '3': return 0x20;
    case '4': return 0x21;
    case '5': return 0x22;
    case '6': return 0x23;
    case '7': return 0x24;
    case '8': return 0x25;
    case '9': return 0x26;
    default: return 0;
  }
}

void sendChar(char c) {
  memset(report, 0, sizeof(report));
  report[2] = hexCharToKeycode(c);

  while (!usb_hid.ready()) delay(1);
  usb_hid.sendReport(0, report, sizeof(report));

  memset(report, 0, sizeof(report));
  while (!usb_hid.ready()) delay(1);
  usb_hid.sendReport(0, report, sizeof(report));

  delay(HID_DELAY);
}

void sendEnter() {
  memset(report, 0, sizeof(report));
  report[2] = HID_EOL_KEY;

  while (!usb_hid.ready()) delay(1);
  usb_hid.sendReport(0, report, sizeof(report));

  memset(report, 0, sizeof(report));
  while (!usb_hid.ready()) delay(1);
  usb_hid.sendReport(0, report, sizeof(report));

  delay(HID_DELAY);
}


void setup() {
  pixels.begin();
  pixels.fill(COLOR_INIT);
  pixels.show();

  Serial.begin(115200);

  gpio_set_function(SCK_PIN, GPIO_FUNC_SPI);
  gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
  gpio_set_function(MISO_PIN, GPIO_FUNC_SPI);
  spi_init(spi0, 1000 * 1000);

  mfrc522.PCD_Init();
  
  if (!TinyUSBDevice.isInitialized()) TinyUSBDevice.begin(0);
  usb_hid.setPollInterval(2);
  usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usb_hid.begin();
  
  if (TinyUSBDevice.mounted()) {
    TinyUSBDevice.detach();
    delay(10);
    TinyUSBDevice.attach();
  }

  buzzer_init();

  pixels.fill(COLOR_IDLE);
  pixels.show();

  buzzer_tone(2000, 100);
  buzzer_tone(3000, 100);
  buzzer_tone(4000, 100);
  buzzer_tone(2300, 100);
}

void loop() {
  #ifdef TINYUSB_NEED_POLLING_TASK
  TinyUSBDevice.task();
  #endif

  if (!TinyUSBDevice.mounted() || !usb_hid.ready()) return;

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(10);
    return;
  }

  pixels.fill(COLOR_SENDING);
  pixels.show();

  Serial.print("UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  unsigned long long uidDecimal = 0;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidDecimal = (uidDecimal << 8) | mfrc522.uid.uidByte[i];
  }

  char uidStr[18];
  sprintf(uidStr, "%llu", uidDecimal);

  buzzer_tone(2000, 100);
  buzzer_tone(3000, 100);

  for (char* p = uidStr; *p; p++) {
      sendChar(*p);
  }

  sendEnter();

  pixels.fill(COLOR_IDLE);
  pixels.show();

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  while (mfrc522.PICC_IsNewCardPresent() || mfrc522.PICC_ReadCardSerial()) {
    delay(10);
  }
}