#include <Servo.h>

Servo tokat;  // Servo nesnesi oluştur
Servo ceza_tokat;

// HABERLEŞME
#define sayac_sinyal A0
#define ceza_sinyal A1
#define kilit_bildirim A2
#define ceza_tokatla_bildirim A3
#define otur_sinyal A4
#define ceza_tokatlandi_sinyal A5

#define KIRMIZI 1
#define MAVI 0

#define KIRMIZI_TOP 1
#define MAVI_TOP 2
#define YESIL_TOP 3

boolean baslangic_ceza_aldik_mi = false;

byte out = 2, s0 = 3, s1 = 4, s2 = 5, s3 = 6, top_sensor = 12, bolge_switch = 8, bolge = 0;

float kirmizi = 0, mavi = 0, yesil = 0;

int kirmizi_alt_limit = 200, mavi_ust_limit = 80, yesil_ust_limit = 70;

// TOPLARI TOKATLAYAN VE CEZAYI TOKATLAYAN SERVOLARIN NORMAL DURUMLARI
// ÖZELLİKLE tokat_default TOKATLAMA KODLARINDA DA KULLANILIYOR, BURADAN DEĞİŞTİRİLMESİ ÖNEMLİ
byte tokat_default = 90, ceza_tokat_default = 70;
byte sayac = 0;
byte sayac_ust_limit = 3;

void setup() {

  tokat.attach(10);
  ceza_tokat.attach(11);
  tokat.write(tokat_default);
  ceza_tokat.write(ceza_tokat_default);

  pinMode(out, INPUT);
  pinMode(top_sensor, INPUT);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

  //HABERLEŞME PİNLERİ
  pinMode(sayac_sinyal, OUTPUT);
  pinMode(ceza_sinyal, OUTPUT);
  pinMode(otur_sinyal, OUTPUT);
  pinMode(ceza_tokatlandi_sinyal, OUTPUT);
  pinMode(kilit_bildirim, INPUT);
  pinMode(ceza_tokatla_bildirim, INPUT);
  pinMode(bolge_switch, INPUT);

  digitalWrite(s0, HIGH);
  digitalWrite(s1, LOW);

  bolge = digitalRead(bolge_switch);


  digitalWrite(ceza_tokatlandi_sinyal, 0);
  Serial.begin(9600);
}

void loop() {
  //olcum_gozlem();
  baslangic_kod();
  //otur();
  if (baslangic_ceza_aldik_mi == true) {
    ceza_birak();
  }
  unsigned long firstMillis = millis();
  while ((millis() - firstMillis) < 45000) {
    normal_kod();
  }
  bizim_topu_birak();
  while (1) {
    tek_tek_topla();
  }
}

void normal_kod() {
  //NORMAL KOD
  if (digitalRead(top_sensor)) {
    byte sonuc = olcum_yeni();
    if (sonuc == KIRMIZI_TOP)  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("KIRMIZI: ");
      //Serial.println(sonuc);

      if (bolge == KIRMIZI && digitalRead(top_sensor)) {
        dogru_al();
        sayac++;
      } else {
        rakip_al();
      }

      if (sayac == sayac_ust_limit) {
        bizim_topu_birak();
        sayac = 0;
      }

    } else if (sonuc == MAVI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("MAVİ: ");
      //Serial.println(sonuc);

      if (bolge == MAVI) {
        dogru_al();
        sayac++;
      } else {
        rakip_al();
      }

      if (sayac == sayac_ust_limit) {
        bizim_topu_birak();
        sayac = 0;
      }

    } else if (sonuc == YESIL_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("CEZA: ");
      //Serial.println(sonuc);

      ceza_al();
      ceza_birak();
    } else {
      //Serial.print("KARARSIZ: ");
      //Serial.println(sonuc);
    }
  } else {
    //Serial.println("BOŞŞ");
  }
}

void baslangic_kod() {
  while (1) {
    //NORMAL KOD
    if (digitalRead(top_sensor)) {
      byte sonuc = olcum_yeni();                            //OLCUM YAKLAŞIK 240 MİLİSANİYEDE TAMAMLANIYOR
      if (sonuc == KIRMIZI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
      {
        //Serial.print("KIRMIZI: ");
        //Serial.println(sonuc);

        if (bolge == KIRMIZI) {
          dogru_al();
          baslangic_birak();
          break;
        } else {
          rakip_al();
        }

      } 
      else if (sonuc == MAVI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
      {
        //Serial.print("MAVİ: ");
        //Serial.println(sonuc);

        if (bolge == MAVI) {
          dogru_al();
          baslangic_birak();
          break;
        } else {
          rakip_al();
        }

      } 
      else if (sonuc == YESIL_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
      {
        //Serial.print("CEZA: ");
        //Serial.println(sonuc);

        ceza_al();
        baslangic_ceza_aldik_mi = true;
      } else {
        //Serial.print("KARARSIZ: ");
        //Serial.println(sonuc);
      }
    } else {
      //Serial.println("BOŞŞ");
    }
  }
}

void tek_tek_topla() {
  //TEK TEK BIRAKAN KOD
  if (digitalRead(top_sensor)) {
    byte sonuc = olcum_yeni();                            //OLCUM YAKLAŞIK 240 MİLİSANİYEDE TAMAMLANIYOR
    if (sonuc == KIRMIZI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("KIRMIZI: ");
      //Serial.println(sonuc);

      if (bolge == KIRMIZI) {
        dogru_al();
        bizim_topu_birak();
      } else {
        rakip_al();
      }

    } else if (sonuc == MAVI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("MAVİ: ");
      //Serial.println(sonuc);

      if (bolge == MAVI) {
        dogru_al();
        bizim_topu_birak();
      } else {
        rakip_al();
      }

    } else if (sonuc == YESIL_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("CEZA: ");
      //Serial.println(sonuc);

      ceza_al();
      ceza_birak();
    } else {
      //Serial.print("KARARSIZ: ");
      //Serial.println(sonuc);
    }
  } else {
    //Serial.println("BOŞŞ");
  }
}

void sadece_ayikla() {
  //NORMAL KOD
  if (digitalRead(top_sensor)) {
    byte sonuc = olcum_yeni();                            //OLCUM YAKLAŞIK 240 MİLİSANİYEDE TAMAMLANIYOR
    if (sonuc == KIRMIZI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("KIRMIZI: ");
      //Serial.println(sonuc);

      if (bolge == KIRMIZI) {
        dogru_al();
      } else {
        rakip_al();
      }

    } else if (sonuc == MAVI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("MAVİ: ");
      //Serial.println(sonuc);

      if (bolge == MAVI) {
        dogru_al();
      } else {
        rakip_al();
      }

    } else if (sonuc == YESIL_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      //Serial.print("CEZA: ");
      //Serial.println(sonuc);

      ceza_al();
      baslangic_ceza_aldik_mi = true;
    } else {
      //Serial.print("KARARSIZ: ");
      //Serial.println(sonuc);
    }
  } else {
    //Serial.println("BOŞŞ");
  }
}

void kalibre_top_okuma() {
  //NORMAL KOD
  if (digitalRead(top_sensor)) {
    byte sonuc = olcum_yeni();                            //OLCUM YAKLAŞIK 240 MİLİSANİYEDE TAMAMLANIYOR
    if (sonuc == KIRMIZI_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      Serial.print("KIRMIZI: ");
      Serial.println(sonuc);

    } else if (sonuc == MAVI_TOP && digitalRead(top_sensor)) {
      Serial.print("MAVİ: ");
      Serial.println(sonuc);
    } else if (sonuc == YESIL_TOP && digitalRead(top_sensor))  //TOKATLAYACAKSAK, OLCUM SIRASINDA TOPUN AĞIZDAN ÇIKMADIĞINI TEYİT ETMELİYİZ
    {
      Serial.print("YESIL: ");
      Serial.println(sonuc);
    } else {
      Serial.print("KARARSIZ: ");
      Serial.println(sonuc);
    }
  } else {
    //Serial.println("BOŞŞ");
  }
}

void dogru_al() {
    tokat.write(180);
    delay(120);
    tokat.write(tokat_default);
  
}

void rakip_al() {
    tokat.write(0);
    delay(150);
    tokat.write(tokat_default);
}

void ceza_al() {
    ceza_tokat.write(ceza_tokat_default + 80);  // CEZA KAPISINI AÇ
    delay(120);
    tokat.write(180);
    delay(150);
    tokat.write(tokat_default);
    delay(100);
    ceza_tokat.write(ceza_tokat_default + 30);  // CEZA KAPISINI SIKIŞTIR
  
}

void bizim_topu_birak() {
  digitalWrite(sayac_sinyal, HIGH);
  delay(2000);
  while (digitalRead(kilit_bildirim) == 1) {}
  digitalWrite(sayac_sinyal, LOW);
}

void baslangic_birak() {
  digitalWrite(sayac_sinyal, HIGH);
  delay(2000);
  while (digitalRead(kilit_bildirim) == 1) {
    sadece_ayikla();
  }
  digitalWrite(sayac_sinyal, LOW);
}

void ceza_birak() {
  digitalWrite(ceza_sinyal, HIGH);
  delay(4000);
  while (digitalRead(kilit_bildirim) == 1) {
    if (digitalRead(ceza_tokatla_bildirim) == 1) {
      ceza_tokat.write(ceza_tokat_default + 40);  // 10 DERECE GERİ SALINIM (en son ceza_tokat_default + 30 konumundaydı çünkü)
      delay(100);
      ceza_tokat.write(ceza_tokat_default - 60);
      delay(300);
      //TEKRAR GERİ SAL
      ceza_tokat.write(ceza_tokat_default + 40);  // 10 DERECE GERİ SALINIM
      delay(300);
      //TEKRAR TOKATLA
      ceza_tokat.write(ceza_tokat_default - 60);
      delay(300);
      ceza_tokat.write(ceza_tokat_default);
      delay(300);
      digitalWrite(ceza_tokatlandi_sinyal, 1); //MEGAYA CEZANIN TOKATLANDIĞINI SÖYLE
      delay(1000);
    }
  }
  digitalWrite(ceza_tokatlandi_sinyal, 0); //BİR SONRAKİ YEŞİL TOP İÇİN DEĞER DEFAULT'A ÇEKİLDİ
  digitalWrite(ceza_sinyal, LOW);
}

void otur() {
  digitalWrite(otur_sinyal, HIGH);
  while (1)
    ;
}

byte olcum_yeni() {
  olc();

  float sonuc_m_k = ((float)mavi / (float)kirmizi) * 100;
  float sonuc_m_y = ((float)mavi / (float)yesil) * 100;
  float sonuc_y_k = ((float)yesil / (float)kirmizi) * 100;
  /*
      Serial.print("m k: ");
      Serial.print(sonuc_m_k);
      Serial.print(" m y: ");
      Serial.print(sonuc_m_y);
      Serial.print(" y k: ");
      Serial.println(sonuc_y_k);
      */

  if (sonuc_m_k < mavi_ust_limit && sonuc_m_y < mavi_ust_limit) {
    return MAVI_TOP;
  } else if (sonuc_m_k > kirmizi_alt_limit && sonuc_y_k > kirmizi_alt_limit) {
    return KIRMIZI_TOP;
  } else if (sonuc_m_y > 100 && sonuc_y_k < yesil_ust_limit) {
    return YESIL_TOP;
  } else {
    return 0;
  }

}

void olc() {
  //RENK SENSÖRÜ KIRMIZI FİLTRE AYARI
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  delay(15);
  kirmizi = pulseIn(out, LOW);

  //RENK SENSÖRÜ MAVİ FİLTRE AYARI
  digitalWrite(s2, LOW);
  digitalWrite(s3, HIGH);
  delay(15);
  mavi = pulseIn(out, LOW);

  //RENK SENSÖRÜ YEŞİL FİLTRE AYARI
  digitalWrite(s2, HIGH);
  digitalWrite(s3, HIGH);
  delay(15);
  yesil = pulseIn(out, LOW);
}

void olcum_gozlem() {
  while (1) {
    olc();

    float sonuc_m_k = ((float)mavi / (float)kirmizi) * 100;
    float sonuc_m_y = ((float)mavi / (float)yesil) * 100;
    float sonuc_y_k = ((float)yesil / (float)kirmizi) * 100;

    Serial.print("Mavi: ");
    Serial.print(mavi);

    Serial.print("  Kırmızı: ");
    Serial.print(kirmizi);

    Serial.print("  Yesil: ");
    Serial.println(yesil);

    Serial.print("m_k: ");
    Serial.print(sonuc_m_k);

    Serial.print("  m_y: ");
    Serial.print(sonuc_m_y);

    Serial.print("  y_k: ");
    Serial.println(sonuc_y_k);

    if (sonuc_m_k < mavi_ust_limit && sonuc_m_y < mavi_ust_limit) {
      Serial.print("MAVI_TOP");
    } else if (sonuc_m_k > kirmizi_alt_limit && sonuc_y_k > kirmizi_alt_limit) {
      Serial.print("KIRMIZI_TOP");
    } else if (sonuc_m_y > 100 && sonuc_y_k < yesil_ust_limit) {
      Serial.print("YESIL_TOP");
    }

    Serial.println("\n");

    delay(300);
  }
}