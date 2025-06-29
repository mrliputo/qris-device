#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "qrcode.h"


#define BUZZER_PIN 13
// Touchscreen pin
#define TOUCH_CS 33
#define TOUCH_IRQ 255  // tidak digunakan
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);
TFT_eSPI tft = TFT_eSPI();
QRCode qrcode;

const String STATIC_QRIS =
"";

const uint8_t QR_VER = 16;
String amount = "";
bool entering = true;

struct Button {
  int x, y, w, h;
  String label;
};

Button buttons[12] = {
  {20,  180, 80, 60, "1"},
  {120, 180, 80, 60, "2"},
  {220, 180, 80, 60, "3"},
  {20,  250, 80, 60, "4"},
  {120, 250, 80, 60, "5"},
  {220, 250, 80, 60, "6"},
  {20,  320, 80, 60, "7"},
  {120, 320, 80, 60, "8"},
  {220, 320, 80, 60, "9"},
  {20,  390, 80, 60, "C"},
  {120, 390, 80, 60, "0"},
  {220, 390, 80, 60, "OK"},
};

String calcCRC16(const String &data) {
  uint16_t crc = 0xFFFF;
  for (size_t c = 0; c < data.length(); c++) {
    crc ^= (uint16_t)data[c] << 8;
    for (uint8_t i = 0; i < 8; i++)
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : crc << 1;
  }
  crc &= 0xFFFF;
  char buf[5];
  sprintf(buf, "%04X", crc);
  return String(buf);
}

String buildDynamicQRIS(const String &nominal) {
  String base = STATIC_QRIS.substring(0, STATIC_QRIS.length() - 4);
  base.replace("010211", "010212");
  int pos = base.indexOf("5802ID");
  if (pos < 0) return "";
  String part1 = base.substring(0, pos);
  String part2 = base.substring(pos);
  String tag54 = "54";
  String len = String(nominal.length());
  if (len.length() == 1) len = "0" + len;
  tag54 += len + nominal;
  String withoutCRC = part1 + tag54 + part2;
  String crc = calcCRC16(withoutCRC);
  return withoutCRC + crc;
}
void playNoteForButton(const String& label) {
  int freq = 523;  // Default: Do'

  if (label == "1") freq = 262;     // Do
  else if (label == "2") freq = 294; // Re
  else if (label == "3") freq = 330; // Mi
  else if (label == "4") freq = 349; // Fa
  else if (label == "5") freq = 392; // Sol
  else if (label == "6") freq = 440; // La
  else if (label == "7") freq = 494; // Si
  else if (label == "8") freq = 523; // Do'
  else if (label == "9") freq = 587; // Re'
  else if (label == "C") freq = 659; // Mi'
  else if (label == "0") freq = 698; // Fa'
  else if (label == "OK") freq = 784; // Sol'

  tone(BUZZER_PIN, freq, 120);  // 120ms
}
void drawButtons() {
  for (int i = 0; i < 12; i++) {
    tft.fillRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, TFT_DARKGREY);
    tft.drawRect(buttons[i].x, buttons[i].y, buttons[i].w, buttons[i].h, TFT_WHITE);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextDatum(MC_DATUM);
    tft.setTextFont(2);
    tft.drawString(buttons[i].label, buttons[i].x + buttons[i].w / 2, buttons[i].y + buttons[i].h / 2);
  }
}

void drawWelcome() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 30);
  tft.println("Selamat Datang .");
  tft.setCursor(10, 60);
  tft.println("Masukkan nominal:");
  tft.drawBmp("/bg.bmp", 0, 0);
  redrawAmount();
  drawButtons();
}

void redrawAmount() {
  tft.fillRect(10, 100, 300, 40, TFT_BLACK);
  tft.setCursor(10, 100);
  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN);
  String tampil = formatRupiah(amount);
  tft.print("Rp." + tampil);
}

void showQRCode(const String &payload) {
  uint8_t qrbuff[qrcode_getBufferSize(QR_VER)];
  QRCode qr;
  qrcode_initText(&qr, qrbuff, QR_VER, 11, payload.c_str());

  tft.fillScreen(TFT_WHITE);
  const int scale = 3;
  const int qrPix = qr.size * scale;
  const int offsetX = (tft.width() - qrPix) / 2;
  const int offsetY = (tft.height() - qrPix) / 2;

  for (uint8_t y = 0; y < qr.size; y++) {
    for (uint8_t x = 0; x < qr.size; x++) {
      uint16_t color = qrcode_getModule(&qr, x, y) ? TFT_BLACK : TFT_WHITE;
      tft.fillRect(offsetX + x * scale, offsetY + y * scale, scale, scale, color);
    }
  }
    String tampil2 = "Rp." + formatRupiah(amount);
String info = tampil2;
tft.setTextDatum(TC_DATUM);              // Rata tengah atas
tft.setTextFont(1);                      // Ukuran kecil
tft.setTextColor(TFT_BLACK, TFT_WHITE);

// Potong teks jika terlalu panjang
if (info.length() > 20) {
  info = info.substring(0, 20) + "...";
}

int textY = offsetY + qrPix + 10;
tft.drawString(info, tft.width() / 2, textY);
}

// Fungsi untuk filter touch noise
bool isStableTouch() {
  if (ts.touched()) {
    delay(20);  // tahan sejenak
    if (ts.touched()) return true;
  }
  return false;
}
String formatRupiah(const String& angka) {
  String hasil = "";
  int len = angka.length();
  int count = 0;
  for (int i = len - 1; i >= 0; i--) {
    hasil = angka[i] + hasil;
    count++;
    if (count % 3 == 0 && i != 0) {
      hasil = '.' + hasil;
    }
  }
  return hasil;
}
void handleTouch() {
  if (isStableTouch()) {
    TS_Point p = ts.getPoint();

    int16_t x = map(p.x, 3900, 300, 0, 320);
    int16_t y = map(p.y, 3900, 300, 0, 480);

    Serial.printf("Mapped Touch: x=%d y=%d (raw: x=%d y=%d)\n", x, y, p.x, p.y);
    tft.fillCircle(x, y, 2, TFT_GREEN);

    for (int i = 0; i < 12; i++) {
      Button btn = buttons[i];
      if (x >= btn.x && x <= btn.x + btn.w && y >= btn.y && y <= btn.y + btn.h) {
        String label = btn.label;
        if (label == "C") {
          amount = "";
          redrawAmount();
        } else if (label == "OK") {
          if (amount.length()) {
            entering = false;
            String payload = buildDynamicQRIS(amount);
            Serial.println("QR Payload: " + payload);
            showQRCode(payload);
          }
        } else {
          if (amount.length() < 8)
            amount += label;
          redrawAmount();
        }
          playNoteForButton(label);
        delay(300);
        break;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  tft.begin();
  tft.setRotation(0);
  ts.begin();
  ts.setRotation(2);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);  // stabilisasi
  // Bersihkan sentuhan hantu
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    Serial.printf("Ghost touch cleaned: x=%d y=%d\n", p.x, p.y);
  }

  drawWelcome();
}

void loop() {

  if (entering) {
    handleTouch();
  } else {
    if (isStableTouch()) {
      playNoteForButton("OK");
      amount = "";
      entering = true;
      delay(300);
      drawWelcome();
    }
  }
}
