/* INFO
  ---------------------------------------------------------------------
                        "KONUŞAN DHT DEMO"                          
          YouTube >>> Dr.TRonik ; TR/İzmir/EKİM/2022               
     2 ADET TEK RENK P10 16*32 PANEL + DHT SENSOR + DFPLAYER MİNİ   
     ÖLÇÜLEN DERECE DEĞİŞTİĞİNDE SESLİ OLARAK YENİ DERECEYİ SÖYLER  
     BELİRLİ SÜRELERDE MAX VE MİN DEĞERLERİ GÖSTERİR VE SÖYLER      
     İKİ ADET RÖLE + 3 BUTON İLE :                                  
     AYARLANAN DERECE AŞILDIĞINDA SOĞUTUCU RÖLE,                    
     AYARLANAN DERECE ALTINA DÜŞÜNCE ISITICI RÖLE ÇALIŞIR.     
     e-posta: bilgi@ronaer.com                                      
     https://www.instagram.com/dr.tronik2022/                       
     YouTube: https://www.youtube.com/@DrTRonik                     
  ---------------------------------------------------------------------
*/

/********************************************************************
  GLOBALS___GLOBALS___GLOBALS___GLOBALS___GLOBALS___GLOBALS___
 ********************************************************************/
#include <SPI.h>  //DMD kütüphanesi ile P10 panel sürmek için gerekli: https://github.com/PaulStoffregen/SPI
#include <DMD.h>  //https://github.com/ronaer/P10-Led-Tabela-RTC-DHT/raw/master/DMD-master.zip

DMD dmd(1, 2);  // Wide:1 , Height:2 ...

#include "TimerOne.h"       //https://github.com/PaulStoffregen/TimerOne
#include <angka6x13.h>      //Font kütüphanesi, DMD kütüphanesi içinde olmalı
#include <SystemFont5x7.h>  //Font kütüphanesi, DMD kütüphanesi içinde olmalı

#include <SoftwareSerial.h>               //ses çalıcımızın işlemci ile haberleşmesi için gerekli kütüphane...
SoftwareSerial mySoftwareSerial(A1, A2);  // RX, TX Pinlerimizi tanımlayalım...
//Arduino A1 >>> dfrobot 3.pin , Arduino A2 >>> dfrobot 2.pin 1K direnç ile

#include <DFRobotDFPlayerMini.h>  //Ses çalıcımızın arduino nano ile uyumlu kütüphanesi...
DFRobotDFPlayerMini myDFPlayer;   // ses çalıcımızın ismi "myDFPlayer" oldu

#include <DHT.h>           //DHT kütüphanesi https://github.com/adafruit/DHT-sensor-library EK KÜTÜPHANE İLE BİRLİKTE KURULMALIDIR...
#define DHTPIN 2           //DHT 2.pine bağlı...
#define DHTTYPE DHT11      //  DHT22 veya DHT11 tipinde sensör kullanılabilir...
DHT dht(DHTPIN, DHTTYPE);  //DHT'yi pin ve tip olarak kütüphaneye tanıtalım
int temp, hum;             //sıcaklık ve nem değerlerini tutacağımız değişkenlerimiz...

#define COOLER_ROLE 4  //Ayarlanan sıcaklık üzerinde çalışacak soğutucu bağlı Röle 1 PIN NO...
#define HEATER_ROLE 5  //Ayarlanan sıcaklık altında çalışacak ısıtıcı bağlı Röle 2 PIN NO...


int t_sound, t_max, t_min, h_max, h_min, bar;

int t_high = 35;  //Röle değerlerinin referans aldığı başlangıç değerleri
int t_low = 16;

#define menuPin 12   //OK BUTTON PIN NUMBER
#define higherPin 3  // COOLER BUTTON PIN NUMBER
#define lowerPin 0   //HEATER BUTTON PIN NUMBER

boolean buttonState_m = HIGH;  //buttonState_menu OK
boolean buttonState_h = HIGH;  //buttonState_high Cooler
boolean buttonState_l = HIGH;  //buttonState_low  Heater

bool flag;

char t_low_s[3];  //Globalde tanımlayarak tüm program boyunca kullanılması sağlandı...
char t_high_s[3];

/*--------------------------------------------------------------------------------------
  Timer1 (TimerOne) tarafından yönlendirilen DMD yenileme taraması için kesme işleyicisi,
  Timer1.initialize() içinde ayarlanan periyotta çağrılır;
  --------------------------------------------------------------------------------------*/
void ScanDMD() {
  dmd.scanDisplayBySPI();
}

/********************************************************************
  SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___SETUP___
 ********************************************************************/
void setup() {
  pinMode(menuPin, INPUT_PULLUP);
  pinMode(higherPin, INPUT_PULLUP);
  pinMode(lowerPin, INPUT_PULLUP);
  pinMode(COOLER_ROLE, OUTPUT);
  pinMode(HEATER_ROLE, OUTPUT);

  digitalWrite(menuPin, HIGH);
  digitalWrite(higherPin, HIGH);
  digitalWrite(lowerPin, HIGH);
  digitalWrite(COOLER_ROLE, LOW);
  digitalWrite(HEATER_ROLE, LOW);


  mySoftwareSerial.begin(9600);        // yazılımsal serialimiz başlasın
  Serial.begin(115200);                // nano ile seri iletişimi başlatalım...
  myDFPlayer.begin(mySoftwareSerial);  // mp3 çalıcımız, software serialimiz dahilinde başlasın
  myDFPlayer.volume(12);               // mp3 çalarımızın volüm seviyesi x/30
  myDFPlayer.setTimeOut(500);
  delay(100);

  dht.begin();
  delay(100);

  temp = dht.readTemperature();
  hum = dht.readHumidity();

  t_sound = temp;
  t_max = temp;
  t_min = temp;
  h_max = hum;
  h_min = hum;


  Timer1.initialize(3000);
  Timer1.attachInterrupt(ScanDMD);  //Timer kesmesini ScanDMD fonksiyonu ile attach edelim...
  dmd.clearScreen(true);            //P10 ledlerin hepsini bir söndürelim

  dmd.selectFont(SystemFont5x7);  //Font seçimi
  dmd.drawString(4, 1, "TEMP", 4, GRAPHICS_NORMAL);
  dmd.drawString(1, 9, "* * *", 5, GRAPHICS_NORMAL);
  dmd.drawString(8, 17, "HUM", 3, GRAPHICS_NORMAL);
  dmd.drawString(1, 25, "* * *", 5, GRAPHICS_NORMAL);
  myDFPlayer.play(1);  //Açılışta her şey yolunda ise 13.sıradaki ses dosyasını çalsın...
  delay(5000);
  dmd.clearScreen(true);
  dmd_yaz();
}

/********************************************************************
  LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__LOOP__
 ********************************************************************/
void loop() {

  static unsigned long timer = millis();
  static unsigned long timer_1 = millis();

  //Dakikada bir defa, sıcaklık ve nem bilgisini sensörden alalım...
  if (millis() - timer > 1 * 60000) {
    timer = millis();
    read_data();
    delay(50);
    //    Serial.println(temp);



    /*------------------------------------------------------------
      Eğer veri okunamazsa tekrar okunsun, ve başa dönülsün...
      isnan(temp) DHT11 ; (temp ==0) DHT22 için...
      t_max ve t_min değerleri atanmadan önce olmalı...
      -----------------------------------------------------------*/

    if ((isnan(temp)) || (temp == 0)) {
      read_data();
      delay(50);
      return;
    }

    dmd_yaz();

    //_____Ölçülen en üst ve en alt dereceleri belirlemek için______
    if (temp >= t_max) {
      t_max = temp;  //Max değer
    } else {
      t_max = t_max;
    }

    if (temp <= t_min) {
      t_min = temp;  //Min değer
    } else {
      t_min = t_min;
    }

    //

    if (hum >= h_max) {
      h_max = hum;  //Max değer
    } else {
      h_max = h_max;
    }

    if (hum <= h_min) {
      h_min = hum;  //Min değer
    } else {
      h_min = h_min;
    }

    //

    //Eğer ölçülen sıcaklık değişirse, sesli olarak bildirilsin:
    if (t_sound != temp) {
      ses();
      t_sound = temp;
    }

    //Şartlar sağlanıyorsa Soğutucu Röle 1'i devreye almak için_____________
    relay_1();

    //Şartlar sağlanıyorsa Isıtıcı Röle 2'yi devreye almak için_____________
    relay_2();
  }
  //___1 dakika sonu___

  //Belirlenen süre dahilinde (5*60000ms=5dk.) max ve min değerleri panelde görelim:
  if (millis() - timer_1 > 5 * 60000) {
    timer_1 = millis();
    t_max_min_yaz();
  }

  // Sıcaklık durumunu panelde görelim:
  indicator();
  temp_bar();

  //Soğutucu Röle 1 derece ayarı
  buttonState_h = digitalRead(higherPin);
  if (buttonState_h == LOW) {
    flag = 0;
    delay(200);
    t_high++;
    if (t_high > 45 || t_high < 30) {
      t_high = 31;
    } else {
      t_high = t_high;
    }

    dmd.clearScreen(true);
    dmd.selectFont(angka6x13);  //Font seçimi
    sprintf(t_high_s, "%02d", t_high);
    dmd.drawString(3, 16, t_high_s, 2, GRAPHICS_NORMAL);
    dmd.drawCircle(20, 18, 1, GRAPHICS_NORMAL);  //derece simgesi...
    dmd.selectFont(SystemFont5x7);
    dmd.drawString(24, 18, "C", 1, GRAPHICS_NORMAL);  // C harfi...
    dmd.drawString(2, 1, "Role1", 5, GRAPHICS_NORMAL);
    dmd.drawString(9, 9, "On:", 3, GRAPHICS_NORMAL);
  }

  //Isıtıcı Röle2 ayarı
  buttonState_l = digitalRead(lowerPin);
  if (buttonState_l == LOW) {
    flag = 1;
    delay(200);
    t_low--;
    if (t_low > 25 || t_low < 15) {
      t_low = 24;
    } else {
      t_low = t_low;
    }
    //    Serial.println(t_low);
    dmd.clearScreen(true);
    dmd.selectFont(angka6x13);  //Font seçimi
    sprintf(t_low_s, "%02d", t_low);
    dmd.drawString(3, 16, t_low_s, 2, GRAPHICS_NORMAL);
    dmd.drawCircle(20, 18, 1, GRAPHICS_NORMAL);
    dmd.selectFont(SystemFont5x7);
    dmd.drawString(24, 18, "C", 1, GRAPHICS_NORMAL);  // C harfi...
    dmd.drawString(1, 1, "Role2", 5, GRAPHICS_NORMAL);
    dmd.drawString(9, 9, "On:", 3, GRAPHICS_NORMAL);
  }

  //Menu OK
  buttonState_m = digitalRead(menuPin);
  if (buttonState_m == LOW) {
    dmd.clearScreen(true);
    if (!flag) {
      dmd.drawString(2, 7, "Role1", 5, GRAPHICS_NORMAL);
    } else {
      dmd.drawString(2, 7, "Role2", 5, GRAPHICS_NORMAL);
    }
    dmd.drawString(9, 17, "OK!", 3, GRAPHICS_NORMAL);
    delay(5000);

    dmd.clearScreen(true);
    dmd.selectFont(SystemFont5x7);
    dmd.drawString(2, 0, "Role1", 5, GRAPHICS_NORMAL);
    dmd.drawString(1, 8, ">", 1, GRAPHICS_NORMAL);
    sprintf(t_high_s, "%02d", t_high);  //dtostrf'a alternatif
    dmd.drawString(8, 8, t_high_s, 2, GRAPHICS_NORMAL);
    dmd.drawCircle(22, 9, 1, GRAPHICS_NORMAL);       //derece simgesi...
    dmd.drawString(26, 8, "C", 1, GRAPHICS_NORMAL);  // C harfi...
    dmd.drawString(2, 16, "Role2", 5, GRAPHICS_NORMAL);
    dmd.drawString(0, 24, "<", 1, GRAPHICS_NORMAL);
    sprintf(t_low_s, "%02d", t_low);
    dmd.drawString(8, 24, t_low_s, 2, GRAPHICS_NORMAL);
    dmd.drawString(26, 24, "C", 1, GRAPHICS_NORMAL);  // C harfi...
    dmd.drawCircle(22, 25, 1, GRAPHICS_NORMAL);       //derece simgesi...
    delay(5000);
    dmd.clearScreen(true);
    relay_1();
    relay_2();
    dmd_yaz();
  }

  /*----------------------------------------------------------------------------------------
    Bazen üst üste  DHT den iki ya da daha fazla defa veri okunamıyor,
    Bu gibi durumlarda ikinci kontrol olarak loop içinde  tekrar okunsun, ve başa dönülsün...
    isnan(temp) DHT11 ; (temp ==0) DHT22 için...
    ---------------------------------------------------------------------------------------*/
  if ((isnan(temp)) || (temp == 0)) {
    dmd.clearScreen(true);
    read_data();
    //    delay(50);
    dmd_yaz();
    return;
  }
}

/********************************************************************
  VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs__VOIDs
********************************************************************/
void ses() {
  //25-33 derece arasında sesli olarak derece bildirimi, aralığı arttırmak mümkün...
  //_____SWITCH CASE BAŞLANGIÇ_____
  switch (temp)  //Anahtarımız: "temp" değişkeni, 25 ile 33 dereceler arası sesli bildirim...
  {
    case 25:  //Anahtarımız 25 döndürürse 2.sesi çal...
      myDFPlayer.play(2);
      delay(1500);
      break;

    case 26:  //Anahtarımız 26 döndürürse  3.sesi çal...
      myDFPlayer.play(3);
      delay(1500);
      break;

    case 27:
      myDFPlayer.play(4);
      delay(1500);
      break;

    case 28:
      myDFPlayer.play(5);
      delay(1500);
      break;

    case 29:
      myDFPlayer.play(6);
      delay(1500);
      break;

    case 30:
      myDFPlayer.play(7);
      delay(1500);
      break;

    case 31:
      myDFPlayer.play(8);
      delay(1500);
      break;

    case 32:
      myDFPlayer.play(9);
      delay(1500);
      break;

    case 33:
      myDFPlayer.play(10);
      delay(3000);
      break;

    default:
      //Eğer bunlardan hiçbiri olmazsa...
      break;
  }
}

void t_max_ses() {

  //_____SWITCH CASE BAŞLANGIÇ_____
  switch (t_max)  //Anahtarımız: "t_max" değişkeni, 25 ile 33 dereceler arası sesli bildirim...
  {
    case 25:  //Anahtarımız 25 döndürürse 2.sesi çal...
      myDFPlayer.play(11);
      delay(1500);
      break;

    case 26:  //Anahtarımız 26 döndürürse  3.sesi çal...
      myDFPlayer.play(12);
      delay(1500);
      break;

    case 27:
      myDFPlayer.play(13);
      delay(1500);
      break;

    case 28:
      myDFPlayer.play(14);
      delay(1500);
      break;

    case 29:
      myDFPlayer.play(15);
      delay(1500);
      break;

    case 30:
      myDFPlayer.play(16);
      delay(1500);
      break;

    case 31:
      myDFPlayer.play(17);
      delay(1500);
      break;

    case 32:
      myDFPlayer.play(18);
      delay(1500);
      break;

    case 33:
      myDFPlayer.play(19);
      delay(1500);
      break;

    default:
      //Eğer bunlardan hiçbiri olmazsa...
      break;
  }
}

void t_min_ses() {

  //_____SWITCH CASE BAŞLANGIÇ_____
  switch (t_min)  //Anahtarımız: "t_min" değişkeni, 25 ile 33 dereceler arası sesli bildirim...
  {
    case 25:  //Anahtarımız 25 döndürürse 2.sesi çal...
      myDFPlayer.play(20);
      delay(1500);
      break;

    case 26:  //Anahtarımız 26 döndürürse  3.sesi çal...
      myDFPlayer.play(21);
      delay(1500);
      break;

    case 27:
      myDFPlayer.play(22);
      delay(1500);
      break;

    case 28:
      myDFPlayer.play(23);
      delay(1500);
      break;

    case 29:
      myDFPlayer.play(24);
      delay(1500);
      break;

    case 30:
      myDFPlayer.play(25);
      delay(1500);
      break;

    case 31:
      myDFPlayer.play(26);
      delay(1500);
      break;

    case 32:
      myDFPlayer.play(27);
      delay(1500);
      break;

    case 33:
      myDFPlayer.play(28);
      delay(1500);
      break;

    default:
      //Eğer bunlardan hiçbiri olmazsa...
      break;
  }
}

void indicator() {
  //Ölçülen derece 18-27 aralığında ise
  if ((temp >= 18) && (temp <= 27)) {
    dmd.writePixel(0, 0, GRAPHICS_NORMAL, 0);
    dmd.writePixel(0, 13, GRAPHICS_NORMAL, 0);
    if (millis() / 2000 % 2 == 0) {
      dmd.writePixel(0, 7, GRAPHICS_NORMAL, 1);
    } else {
      dmd.writePixel(0, 7, GRAPHICS_NORMAL, 0);
    }
  }


  //Ölçülen derece 28 ve daha büyük ise
  else if ((temp >= 28)) {
    dmd.writePixel(0, 7, GRAPHICS_NORMAL, 0);
    dmd.writePixel(0, 13, GRAPHICS_NORMAL, 0);
    if (millis() / 1000 % 2 == 0) {
      dmd.writePixel(0, 0, GRAPHICS_NORMAL, 1);
    } else {
      dmd.writePixel(0, 0, GRAPHICS_NORMAL, 0);
    }
  }


  //Ölçülen derece 17 ve daha küçük ise
  else if ((temp <= 17)) {
    dmd.writePixel(0, 0, GRAPHICS_NORMAL, 0);
    dmd.writePixel(0, 7, GRAPHICS_NORMAL, 0);
    if (millis() / 1000 % 2 == 0) {
      dmd.writePixel(0, 13, GRAPHICS_NORMAL, 1);
    } else {
      dmd.writePixel(0, 13, GRAPHICS_NORMAL, 0);
    }
  }
}


//____En üst ve en alt sıcaklık değerleri yazdırmak için_______________

void t_max_min_yaz() {
  //t_max , t_min yazdırmak için
  dmd.clearScreen(true);
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(0, 0, "TEMP:", 5, GRAPHICS_NORMAL);
  for (int z = 10; z < 16; z++) {
    dmd.writePixel(2, z, GRAPHICS_NORMAL, 1);
    dmd.writePixel(7, z, GRAPHICS_NORMAL, 1);
  }
  dmd.writePixel(1, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel(3, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel(6, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel(8, 11, GRAPHICS_NORMAL, 1);
  dmd.drawCircle(11, 9, 1, GRAPHICS_NORMAL);  //derece simgesi...
  dmd.drawString(14, 9, "C", 1, GRAPHICS_NORMAL);
  for (int z = 9; z < 15; z++) {
    dmd.writePixel(24, z, GRAPHICS_NORMAL, 1);
    dmd.writePixel(29, z, GRAPHICS_NORMAL, 1);
  }
  dmd.writePixel(23, 13, GRAPHICS_NORMAL, 1);
  dmd.writePixel(25, 13, GRAPHICS_NORMAL, 1);
  dmd.writePixel(28, 13, GRAPHICS_NORMAL, 1);
  dmd.writePixel(30, 13, GRAPHICS_NORMAL, 1);
  dmd.drawString(14, 19, ".", 1, GRAPHICS_NORMAL);
  dmd.selectFont(angka6x13);  //Font seçimi
  char buffer[10];
  dmd.drawString(0, 18, (dtostrf(t_max, 2, 0, buffer)), 2, GRAPHICS_NORMAL);
  dmd.drawString(19, 18, (dtostrf(t_min, 2, 0, buffer)), 2, GRAPHICS_NORMAL);
  t_max_ses();
  t_min_ses();
  delay(4000);
  dmd.clearScreen(true);

  //t_min, h_min yazdırmak için
  dmd.selectFont(SystemFont5x7);
  dmd.drawString(1, 0, "NEM:", 4, GRAPHICS_NORMAL);
  dmd.drawString(14, 19, ".", 1, GRAPHICS_NORMAL);
  for (int z = 10; z < 16; z++) {
    dmd.writePixel(2, z, GRAPHICS_NORMAL, 1);
    dmd.writePixel(7, z, GRAPHICS_NORMAL, 1);
  }
  dmd.writePixel(1, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel(3, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel(6, 11, GRAPHICS_NORMAL, 1);
  dmd.writePixel(8, 11, GRAPHICS_NORMAL, 1);
  dmd.drawString(14, 9, "%", 1, GRAPHICS_NORMAL);
  for (int z = 9; z < 15; z++) {
    dmd.writePixel(24, z, GRAPHICS_NORMAL, 1);
    dmd.writePixel(29, z, GRAPHICS_NORMAL, 1);
  }
  dmd.writePixel(23, 13, GRAPHICS_NORMAL, 1);
  dmd.writePixel(25, 13, GRAPHICS_NORMAL, 1);
  dmd.writePixel(28, 13, GRAPHICS_NORMAL, 1);
  dmd.writePixel(30, 13, GRAPHICS_NORMAL, 1);
  dmd.selectFont(angka6x13);  //Font seçimi
  dmd.drawString(0, 18, (dtostrf(h_max, 2, 0, buffer)), 2, GRAPHICS_NORMAL);
  dmd.drawString(19, 18, (dtostrf(h_min, 2, 0, buffer)), 2, GRAPHICS_NORMAL);
  delay(5000);
  dmd.clearScreen(true);
}

//DHT'den veri okuma...
void read_data() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
}

//Sıcaklık barını panele yazdırma...
void temp_bar() {
  bar = map(temp, 15, 45, 15, 0);  //barımızın en alt değeri 15, en üst değeri 45 derece
  dmd.drawLine(31, 15, 31, bar, GRAPHICS_NORMAL);
  //Sıcaklık düşerken sıcaklık barında yanık kalan pixellerin sönmesi için:
  for (int y = 0; y <= bar - 1; y++) {
    dmd.writePixel(31, y, GRAPHICS_NORMAL, 0);
  }
}

//P10 a veri yazdırma...
void dmd_yaz() {
  //Derece ve nem bilgisini yazdırmak için...
  dmd.selectFont(SystemFont5x7);                                            //Font seçimi
  char buffer[10];                                                          //dmd kütüphanesi ile drawString fonksiyonunda temp ve hum değerlerini yazdırabilmek için ...
  dmd.selectFont(angka6x13);                                                //Font seçimi
  dmd.drawString(4, 0, (dtostrf(temp, 2, 0, buffer)), 2, GRAPHICS_NORMAL);  // int--> stringe çevirelim ve drawString fonksiyonunda kullanalım; spirntf'e alternatif...
  dmd.drawString(0, 18, (dtostrf(hum, 2, 0, buffer)), 2, GRAPHICS_NORMAL);  // int--> stringe çevirelim ve drawString fonksiyonunda kullanalım
  dmd.selectFont(SystemFont5x7);                                            //Font seçimi
  dmd.drawCircle(20, 1, 1, GRAPHICS_NORMAL);                                //derece simgesi...
  dmd.drawString(23, 1, "C", 1, GRAPHICS_NORMAL);                           // C harfi...
  dmd.drawString(15, 25, "Nem", 3, GRAPHICS_NORMAL);                        // Nem yazalım...
  dmd.drawString(21, 17, "%", 1, GRAPHICS_NORMAL);                          // % yazalım...
}

void relay_1() {
  //Soğutucu Röle 1'i devreye almak için_____________

  if (temp > t_high) {
    digitalWrite(COOLER_ROLE, HIGH);
    dmd.writePixel(0, 16, GRAPHICS_NORMAL, 1);
  } else {
    digitalWrite(COOLER_ROLE, LOW);
    dmd.writePixel(0, 16, GRAPHICS_NORMAL, 0);
  }
}

void relay_2() {
  //Isıtıcı Röle 2'yi devreye almak için_____________

  if (temp < t_low) {
    digitalWrite(HEATER_ROLE, HIGH);
    dmd.writePixel(0, 15, GRAPHICS_NORMAL, 1);
  } else {
    digitalWrite(HEATER_ROLE, LOW);
    dmd.writePixel(0, 15, GRAPHICS_NORMAL, 0);
  }
}
// Fonksiyon sonu...

/*___İletişim:
  e-posta: bilgi@ronaer.com
  https://www.instagram.com/dr.tronik2022/
  YouTube: https://www.youtube.com/@DrTRonik
*/