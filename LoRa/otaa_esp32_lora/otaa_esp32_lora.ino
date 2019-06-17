#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <LoraEncoder.h>

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const PROGMEM u1_t NWKSKEY[16] = { 0x5F, 0x38, 0x7B, 0xE9, 0x0C, 0x8E, 0xAE, 0xBC, 0x18, 0x18, 0xEC, 0x12, 0xD4, 0xE8, 0xBC, 0xFB };

// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN
// network.
static const u1_t PROGMEM APPSKEY[16] = { 0xAE, 0x85, 0x75, 0x18, 0x9F, 0x57, 0xC2, 0x0D, 0x97, 0x8A, 0xF8, 0x71, 0x86, 0x37, 0x59, 0x7D };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x26011C92; // <-- Change this address for every node!

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 18,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = {/*dio0*/ 26, /*dio1*/ 33, /*dio2*/ 32}
};

int temp_lida = 0;
float temperatura = 2.0;
int contador = 0;
float latitude = -8.055668;
float longitude = -34.951578;
// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t mydata[] = { 0, 0, 0, 0, 0, 0, 0, 0};
static osjob_t initjob, sendjob, blinkjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 20;
void do_send(osjob_t* j) {
  
  //dtostrf(latitude, 5, 4, (char*)mydata);
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare transmission at the next possible time.
    //LMIC_setTxData2(1, mydata, strlen((char*) mydata), 0);
    Serial.println(); 
    Serial.println("Packet queued");
    Serial.print("TX nº: ");
    Serial.println(contador);
    contador++;
    Serial.println(LMIC.freq);
    Serial.print("Latitude = ");
    Serial.println(latitude);
    Serial.print("Longitude = ");
    Serial.println(longitude);

    // Split both words (16 bits) into 6 bytes of 8
    /*byte message[4];
    int latInt = latitude * 1000000;
    message[0] = highByte(latInt);
    message[1] = lowByte(latInt);
    int longInt = longitude * 1000000;
    message[2] = highByte(longInt);
    message[3] = lowByte(longInt);
    
    //int temp = temperatura * 100;
    //message[0] = highByte(temp);
    //message[1] = lowByte(temp);
    
    LMIC_setTxData2(1, message, sizeof(message), 0);*/

    /*uint8_t coords[6]; // 6*8 bits = 48
    int32_t lat = latitude * 10000;
    int32_t lon = longitude * 10000;
    
    // Pad 2 int32_t to 6 8uint_t, skipping the last byte (x >> 24)
    coords[0] = lat;
    coords[1] = lat >> 8;
    coords[2] = lat >> 16;
    
    coords[3] = lon;
    coords[4] = lon >> 8;
    coords[5] = lon >> 16;*/

    //coordenadas
    byte coordenadas[12];
    LoraEncoder encoder(coordenadas);
    encoder.writeUnixtime(1468075322);
    encoder.writeLatLng(latitude, longitude);
    
    LMIC_setTxData2(1, coordenadas, sizeof(coordenadas), 0);
  }
}
void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  Serial.println(ev);
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println("EV_SCAN_TIMEOUT");
      break;
    case EV_BEACON_FOUND:
      Serial.println("EV_BEACON_FOUND");
      break;
    case EV_BEACON_MISSED:
      Serial.println("EV_BEACON_MISSED");
      break;
    case EV_BEACON_TRACKED:
      Serial.println("EV_BEACON_TRACKED");
      break;
    case EV_JOINING:
      Serial.println("EV_JOINING");
      break;
    case EV_JOINED:
      Serial.println("EV_JOINED");
      break;
    case EV_RFU1:
      Serial.println("EV_RFU1");
      break;
    case EV_JOIN_FAILED:
      Serial.println("EV_JOIN_FAILED");
      break;
    case EV_REJOIN_FAILED:
      Serial.println("EV_REJOIN_FAILED");
      break;
    case EV_TXCOMPLETE:
      Serial.println("EV_TXCOMPLETE (includes waiting for RX windows)");
      if (LMIC.dataLen) {
        // data received in rx slot after tx
        Serial.print("Data Received: ");
        Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
        Serial.println();
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println("EV_LOST_TSYNC");
      break;
    case EV_RESET:
      Serial.println("EV_RESET");
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println("EV_RXCOMPLETE");
      break;
    case EV_LINK_DEAD:
      Serial.println("EV_LINK_DEAD");
      break;
    case EV_LINK_ALIVE:
      Serial.println("EV_LINK_ALIVE");
      break;
    default:
      Serial.println("Unknown event");
      break;
  }
}
 
void setup() {
  Serial.begin(115200);
  //analogReference(INTERNAL);
  while (!Serial);
  Serial.println("Starting");
#ifdef VCC_ENABLE
  // For Pinoccio Scout boards
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  delay(1000);
#endif
 
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();
  //LMIC_setClockError(MAX_CLOCK_ERROR * 1/100);
  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif
 
  // Disable link check validation
  LMIC_setLinkCheckMode(0);
 
  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;
 
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7, 14);
 
  for (int i = 1; i < 64; i++)
  {
    LMIC_disableChannel(i);  // only the first channel 902.3Mhz works now.
  }
 
  // Start job
  do_send(&sendjob);
}
 
void loop() {
  os_runloop_once();
  temp_lida = 1.4;
  temperatura = temp_lida * 0.1075268817204301;
}
