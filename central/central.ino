#include <BLEDevice.h>
#include <BLEScan.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Preferences.h>

int buzzer = 25;
int ledVerde = 27;
int ledAmarelo = 26;
int ledVermelho = 32;

bool buzzerLigado = false;

int limiteSinal = -75;

MFRC522 rfid(4, 14);
byte cartaoAutorizado[4];
bool temCartao = false;
Preferences memoria;

BLEScan* scanner;
bool procurando = false;
bool achouNaBusca = false;
unsigned long ultimoSinal = 0;
float sinalMedio = -100;
bool perto = false;
bool liberado = false;

class ProcurarPulseira : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice aparelho) {
    if (aparelho.haveName() && aparelho.getName() == "SempreAqui") {
      achouNaBusca = true;
      int sinal = aparelho.getRSSI();

      if (sinalMedio <= -100 || millis() - ultimoSinal > 5000) {
        sinalMedio = sinal;
      } else {
        sinalMedio = sinalMedio * 0.7 + sinal * 0.3;
      }
      ultimoSinal = millis();

      Serial.print("Pulseira - sinal: ");
      Serial.print(sinal);
      Serial.print("  | media: ");
      Serial.println(sinalMedio);
    }
  }
};

void fimDaBusca(BLEScanResults resultados) {
  if (!achouNaBusca) {
    Serial.println("Pulseira nao encontrada");
  }
  achouNaBusca = false;
  procurando = false;
}

void acenderLeds(bool verde, bool amarelo, bool vermelho) {
  digitalWrite(ledVerde, verde);
  digitalWrite(ledAmarelo, amarelo);
  digitalWrite(ledVermelho, vermelho);
}

void testarLed(int pino, const char* nome) {
  Serial.print("Teste: LED ");
  Serial.println(nome);
  digitalWrite(pino, HIGH);
  delay(600);
  digitalWrite(pino, LOW);
}

void setup() {
  Serial.begin(115200);
  pinMode(buzzer, OUTPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarelo, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  noTone(buzzer);

  testarLed(ledVerde, "verde (P27)");
  testarLed(ledAmarelo, "amarelo (P26)");
  testarLed(ledVermelho, "vermelho (P32)");

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_DumpVersionToSerial();

  BLEDevice::init("");
  scanner = BLEDevice::getScan();
  scanner->setAdvertisedDeviceCallbacks(new ProcurarPulseira());
  scanner->setActiveScan(true);
  scanner->setInterval(100);
  scanner->setWindow(99);

  memoria.begin("sempreaqui", false);
  if (memoria.getBytesLength("cartao") == 4) {
    memoria.getBytes("cartao", cartaoAutorizado, 4);
    temCartao = true;
    Serial.println("Cartao cadastrado carregado");
  } else {
    Serial.println("Nenhum cartao cadastrado: passe o cartao para cadastrar");
  }

  ultimoSinal = millis();
}

void piscar(int pino) {
  for (int i = 0; i < 3; i++) {
    acenderLeds(false, false, false);
    digitalWrite(pino, HIGH);
    delay(150);
    digitalWrite(pino, LOW);
    delay(150);
  }
}

void loop() {
  if (!procurando) {
    scanner->clearResults();
    procurando = scanner->start(1, fimDaBusca, false);
  }

  bool vistaAgora = millis() - ultimoSinal < 5000;
  if (!vistaAgora) {
    perto = false;
  } else if (sinalMedio > limiteSinal) {
    perto = true;
  } else if (sinalMedio < limiteSinal - 3) {
    perto = false;
  }

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.print("UID: ");
    for (int i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    if (!temCartao) {
      memcpy(cartaoAutorizado, rfid.uid.uidByte, 4);
      memoria.putBytes("cartao", cartaoAutorizado, 4);
      temCartao = true;
      Serial.println("Cartao CADASTRADO");
      piscar(ledAmarelo);
    } else if (memcmp(rfid.uid.uidByte, cartaoAutorizado, 4) == 0) {
      liberado = !liberado;
      if (liberado) {
        Serial.println("Cartao autorizado: saida LIBERADA");
      } else {
        Serial.println("Cartao autorizado: saida BLOQUEADA");
      }
    } else {
      Serial.println("Cartao recusado");
      piscar(ledVermelho);
    }
    rfid.PICC_HaltA();
    delay(500);
  }

  if (Serial.available()) {
    char comando = Serial.read();
    if (comando == 'c') {
      memoria.remove("cartao");
      temCartao = false;
      liberado = false;
      Serial.println("Cartao apagado: passe um cartao para cadastrar");
    }
  }

  if (liberado) {
    acenderLeds(false, true, false);
    noTone(buzzer);
  } else if (perto) {
    acenderLeds(true, false, false);
    noTone(buzzer);
  } else {
    acenderLeds(false, false, true);
    if (buzzerLigado) {
      tone(buzzer, 2000);
    }
  }

  delay(20);
}
