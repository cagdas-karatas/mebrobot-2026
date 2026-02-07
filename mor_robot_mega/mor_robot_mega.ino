#include<Servo.h>
#include <Adafruit_NeoPixel.h>

Servo bizim;
Servo rakip;
Servo ceza;

// HABERLEŞME
#define sayac_bildirim A0
#define ceza_bildirim A1
#define kilit_sinyal A2
#define ceza_tokatla_sinyal A3
#define otur_bildirim A4

// Sağ motor pinleri
#define EN_R 2
#define RPWM_R 3
#define LPWM_R 4

// Sol motor pinleri
#define RPWM_L 5
#define LPWM_L 6
#define EN_L 7

#define sol_goz 22
#define sag_goz 24

#define s0 52
#define s1 50
#define s2 46
#define s3 48
#define out 44

#define LED_PIN 42
#define LED_COUNT  8 // LED sayısı
#define KIRMIZI 1
#define MAVI 0
#define BIZIM_TOPU_BIRAK 1
#define CEZA_BIRAK 0

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

float kirmizi = 0, mavi = 0;
int kirmizi_veriler[4] = { 0, 0, 0, 0 };
int mavi_veriler[4] = { 0, 0, 0, 0 };
int mavi_ust_limit = 82, kirmizi_alt_limit = 330;
byte bolge = 0, duvarla_isim_var = 0;

byte bizim_kapak_default = 20, ceza_kapak_default = 8;

void setup() {
  // SERVO TANIMLARI
  rakip.attach(A7);
  bizim.attach(A8);
  ceza.attach(A9);
  bizim.write(bizim_kapak_default);
  rakip.write(170);
  ceza.write(ceza_kapak_default);

  // MZ80 TANIMLARI
  pinMode(sol_goz, INPUT);
  pinMode(sag_goz, INPUT);

  //RENK SENSÖRÜ TANIMLARI
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);

  // Sağ motor pin tanımları
  pinMode(EN_R, OUTPUT);
  pinMode(RPWM_R, OUTPUT);
  pinMode(LPWM_R, OUTPUT);

  // Sol motor pin tanımları
  pinMode(EN_L, OUTPUT);
  pinMode(RPWM_L, OUTPUT);
  pinMode(LPWM_L, OUTPUT);

  //HABERLEŞME PİNLERİ
  pinMode(sayac_bildirim, INPUT);
  pinMode(ceza_bildirim, INPUT);
  pinMode(otur_bildirim, INPUT);
  pinMode(kilit_sinyal, OUTPUT);
  pinMode(ceza_tokatla_sinyal, OUTPUT);

  // Motor sürücülerini aktif et
  digitalWrite(EN_R, HIGH);
  digitalWrite(EN_L, HIGH);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);

  //NEOPİXEL
  pinMode(A14, INPUT);
  bolge = digitalRead(A14);
  strip.begin();
  strip.show();

  for (int i = 0; i < LED_COUNT; i++) {
    if (bolge == KIRMIZI)
    {
      strip.setPixelColor(i, strip.Color(255, 0, 0)); // kırmızı
    }
    else
    {
      strip.setPixelColor(i, strip.Color(0, 0, 255)); // mavi
    }
  }
  strip.show();

  Serial.begin(9600);
  while(digitalRead(sag_goz) == 0);
  basla();
}

void loop() {
  if (digitalRead(sayac_bildirim) == 1)
  {
    digitalWrite(kilit_sinyal, HIGH);
    duvarla_isim_var = 1;
    while(duvarla_isim_var == 1)
    {
      duvar_takip(BIZIM_TOPU_BIRAK);
    }
    digitalWrite(kilit_sinyal, LOW);
    delay(2000);
  }
  else if (digitalRead(ceza_bildirim) == 1)
  {
    digitalWrite(kilit_sinyal, HIGH);
    duvarla_isim_var = 1;
    while(duvarla_isim_var == 1)
    {
      duvar_takip(CEZA_BIRAK);
    }
    digitalWrite(kilit_sinyal, LOW);
    delay(2000);
  }
  else if(digitalRead(otur_bildirim) == 1)
  {
    duvar_takip_otur();
  }
  else
  {
    rastgele();
  }
}

void goz_okuma()
{
  Serial.print("Sol göz: ");
  Serial.println(digitalRead(sol_goz));
  Serial.print("Saü göz: ");
  Serial.println(digitalRead(sag_goz));
  Serial.print("\n\n\n");
  delay(300);
}

void rastgele()
{
  if (digitalRead(sol_goz) == 0)
  {
    sag(80);
    delay(600);
  }
  else if (digitalRead(sag_goz) == 0)
  {
    sag(80);
    delay(800);
  }
  else
  {
    ileri(150);
  }
}

void duvar_takip(byte nereye)
{
  if (digitalRead(sol_goz) == 0 && digitalRead(sag_goz) == 0)
  {
    dur(200);
    int sonuc = olcum();

    /* AŞAĞIDAKİ IF KOŞULUNUN TÜRKÇE MEALİ :)
     * MAVİ BÖLGE OKUDUK: KIRMIZI KÖŞEYİZ(bolge = KIRMIZI = 1) ve CEZA BIRAKACAĞIZ(nereye = CEZA_BIRAK = 0) --> bolge(1) != nereye(0)
     * MAVİ BÖLGE OKUDUK: MAVİ KÖŞEYİZ(bolge = MAVI = 0) ve BİZİM TOPU BIRAKACAĞIZ(nereye = BIZIM_TOPU_BIRAK = 1) --> bolge(0) != nereye(1)
     * KIRMIZI BÖLGE OKUDUK: KIRMIZI KÖŞEYİZ(bolge = KIRMIZI = 1) ve BİZİM TOPU BIRAKACAĞIZ(nereye = BIZIM_TOPU_BIRAK = 1) --> bolge(1) == nereye(1)
     * KIRMIZI BÖLGE OKUDUK: MAVİ KÖŞEYİZ(bolge = MAVI = 0) ve CEZA BIRAKACAĞIZ(nereye = CEZA_BIRAK = 0) --> bolge(0) == nereye(0)
     * GERİ KALAN HİÇBİR KOŞULDA IF'İN İÇİNE GİRİLİP PARK YAPILMAZ DUVAR TAKİBE DEVAM EDİLİR <3
     */
    if ( (sonuc < mavi_ust_limit && bolge != nereye) || (sonuc > kirmizi_alt_limit && bolge == nereye) )
    {
      dur(200);
      park(nereye);
    }
    while (digitalRead(sag_goz) == 0)
    {
      sol(100);
    }
    
    dur(200);
  }
  else if(digitalRead(sol_goz) == 0 && digitalRead(sag_goz) == 1)
  {
    dur(200);
    delay(200);
    unsigned long firstMillis = millis();
    while (digitalRead(sag_goz) == 1)
    {
      if( (millis() - firstMillis) > 3000)
      {
        ileri(100);
        delay(300);
        break;
      }
      sol(100);
    }
  }
  else
  {
    if (digitalRead(sag_goz) == 1) //SAĞA YANAŞ
    {
      ileri(150, 100);
    }
    else
    {
      ileri(100, 150);
    }
  }
}

void duvar_takip_otur()
{
  if (digitalRead(sol_goz) == 0 && digitalRead(sag_goz) == 0)
  {
    dur(200);
    int sonuc = olcum();
    
    if ( (sonuc < mavi_ust_limit && bolge == KIRMIZI) || (sonuc > kirmizi_alt_limit && bolge == MAVI) )
    {
      ileri(100);
      delay(150);
      dur(0);
      while(1); // OTURDUK BEKLİYORUZ
    }
    
    while (digitalRead(sag_goz) == 0)
    {
      sol(100);
    }
    
    dur(200);
  }
  else if(digitalRead(sol_goz) == 0 && digitalRead(sag_goz) == 1)
  {
    unsigned long firstMillis = millis();
    while (digitalRead(sag_goz) == 1)
    {
      if( (millis() - firstMillis) > 3000)
      {
        ileri(100);
        delay(300);
        break;
      }
      sol(100);
    }
  }
  else
  {
    if (digitalRead(sag_goz) == 1) //SAĞA YANAŞ
    {
      ileri(150, 100);
    }
    else
    {
      ileri(100, 150);
    }
  }
}

void park (byte nereye)
{
  while (digitalRead(sag_goz) == 0)
  {
    sol(100);
  }
  dur(200);
  sol(100);
  delay(100);
  dur(200);
  if (nereye == 1)
  {
    bizim.write(bizim_kapak_default + 90);
    delay(150);
  }
  else
  {
    sol(100);
    delay(300);
    dur(200);
    ceza.write(ceza_kapak_default + 120);
    delay(100);
    digitalWrite(ceza_tokatla_sinyal, HIGH);
    delay(500);
    digitalWrite(ceza_tokatla_sinyal, LOW);
  }
  ileri(100);
  delay(1600);
  dur(200);
  if(nereye == 1)
  {
    bizim.write(bizim_kapak_default);
  }
  else
  {
    ceza.write(ceza_kapak_default);
  }
  duvarla_isim_var = 0;
}

void ileri(byte hiz)
{
  analogWrite(RPWM_R, hiz);
  analogWrite(LPWM_R, 0);
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, hiz);
}

void ileri(byte sol_hiz, byte sag_hiz)
{
  analogWrite(RPWM_R, sag_hiz);
  analogWrite(LPWM_R, 0);
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, sol_hiz);
}

void geri(byte hiz)
{
  analogWrite(RPWM_R, 0);
  analogWrite(LPWM_R, hiz);
  analogWrite(RPWM_L, hiz);
  analogWrite(LPWM_L, 0);
}

void sol(byte hiz)
{
  analogWrite(RPWM_R, hiz);
  analogWrite(LPWM_R, 0);
  analogWrite(RPWM_L, hiz);
  analogWrite(LPWM_L, 0);
}

void sag(byte hiz)
{
  analogWrite(RPWM_R, 0);
  analogWrite(LPWM_R, hiz);
  analogWrite(RPWM_L, 0);
  analogWrite(LPWM_L, hiz);
}

void dur(byte guc)
{
  analogWrite(RPWM_R, guc);
  analogWrite(LPWM_R, guc);
  analogWrite(RPWM_L, guc);
  analogWrite(LPWM_L, guc);
}

void basla()
{
  // İLERİ GİT (her iki motor ileri yönde döner)
  ileri(255, 225); // ileri(byte hız)

  delay(800);  // 1 saniye ileri git

  // DUR
  dur(100); //dur(byte guc) -> guc ne kadar yüksekse o kadar sert fren yapar.

  //durduğunu anla
  delay(1000);
}


float olcum() {
  //verileri güncelliyoruz
  for (int i = 0; i < 4; i++) {
    olc();
    kirmizi_veriler[i] = kirmizi;
    mavi_veriler[i] = mavi;
  }

  // 6 TANE VERİ ALIP İÇELERİNDEN TUTARLI HALE GETİRİYORUZ
  // ÖRNEK : 64,65,68,97 ÖLÇMÜŞ OLALIM -> 67 GİBİ BİR SONUÇ DÖNDÜRÜYOR ANLIK DALGALANMALARDAN ETKİLENMEMİŞ OLUYORUZ
  kirmizi = stabilSonucuBul(kirmizi_veriler, 4);
  mavi = stabilSonucuBul(mavi_veriler, 4);

  float sonuc = ((float)mavi / (float)kirmizi) * 100;
  return sonuc;
}

void olc() {
  //RENK SENSÖRÜ KIRMIZI FİLTRE AYARI
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  delay(10);
  kirmizi = pulseIn(out, LOW);

  //RENK SENSÖRÜ MAVİ FİLTRE AYARI
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  delay(10);
  mavi = pulseIn(out, LOW);
}





// ******************************Ortalama hesaplayan fonksiyon*****************************
double ortalamaHesapla(const int arr[], int size) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += arr[i];
  }
  return sum / size;
}

// Standart sapmayı hesaplayan fonksiyon
double standartSapmaHesapla(const int arr[], int size, double mean) {
  double sum = 0;
  for (int i = 0; i < size; i++) {
    sum += pow(arr[i] - mean, 2);
  }
  return sqrt(sum / size);
}

// Uç değerleri filtreleyerek en istikrarlı değeri bul
int stabilSonucuBul(const int arr[], int size) {
  // Dizinin ortalamasını hesapla
  double mean = ortalamaHesapla(arr, size);

  // Dizinin standart sapmasını hesapla
  double stdDev = standartSapmaHesapla(arr, size, mean);

  // Standart sapmanın 1.5 katından daha uzak olan değerleri göz ardı et
  const double threshold = 1.5 * stdDev;

  // Filtrelenmiş dizinin ortalamasını ve en yakın değeri bul
  double filteredSum = 0;
  int filteredCount = 0;
  int closestValue = arr[0];
  double minDifference = 1e6;  // çok büyük bir sayı kullanıyoruz

  for (int i = 0; i < size; i++) {
    if (fabs(arr[i] - mean) <= threshold) {
      filteredSum += arr[i];
      filteredCount++;

      // Ortalamaya en yakın değeri bl
      double diff = fabs(arr[i] - mean);
      if (diff < minDifference) {
        minDifference = diff;
        closestValue = arr[i];
      }
    }
  }

  return closestValue;
}
