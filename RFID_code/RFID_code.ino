#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::Uid acceptedUid;

bool tagScanned = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();       // Init SPI bus
  rfid.PCD_Init();   // Init MFRC522

  Serial.println("Scan your RFID tag to begin...");

  // Wait for RFID tag before proceeding to loop
  while (!tagScanned) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.print("RFID UID: ");
      for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
        acceptedUid.uidByte[i] = rfid.uid.uidByte[i];
      }
      acceptedUid.size = rfid.uid.size;
      Serial.println();
      tagScanned = true;

      // Halt PICC
      rfid.PICC_HaltA();
      // Stop encryption on PCD
      rfid.PCD_StopCrypto1();
    }
  }

  Serial.println("Access granted. Entering main loop...");
}

void loop() {
  // Main operations after initial RFID scan
  Serial.println("Main loop running...");
  delay(1000); // simulate main task
}
