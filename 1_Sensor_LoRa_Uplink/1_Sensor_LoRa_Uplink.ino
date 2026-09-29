
// ==============================================================================
// Teste de UPLINK - UL
// Nó sensor LoRa transmite a cada 1 segundo a luminosidade
// PK-LORA V2 RO - WissTek IoT
// ESP8266 + RFM96/RFM95
// ==============================================================================

// ==============================================================================
// IMPORTAÇÃO DE BIBLIOTECAS
// ==============================================================================

#include <SPI.h>
#include <LoRa.h>

// ==============================================================================
// PINAGEM - ESP8266 + RFM96
// ==============================================================================
//
// Comunicação SPI:
// D5 -> SCK
// D6 -> MISO
// D7 -> MOSI
//
// Controle do RFM96:
// D8 -> NSS / CS
// D3 -> RESET
// D1 -> DIO0
//
// Sensor:
// A0 -> LDR
//
// ==============================================================================
#define NSS_PIN            15   // D8
#define RST_PIN             0   // D3
#define DIO0_PIN            5   // D1

#define LED_VERMELHO_PIN   4   // D2
#define LDR_PIN            A0

// LED de indicação


// ==============================================================================
// PARÂMETROS DO LORA
// ==============================================================================

#define FREQUENCY_IN_HZ       903E6
#define txPower               17
#define spreadingFactor       7
#define signalBandwidth       125E3
#define codingRateDenominator 8


// ==============================================================================
// TAMANHO DO PACOTE
// ==============================================================================

#define TAMANHO_PACOTE 20

// Pacote de Uplink
byte Pacote_UL[TAMANHO_PACOTE];


// ==============================================================================
// VARIÁVEIS DE MEDIÇÃO
// ==============================================================================

int luminosidade;
int contador_pacotes = 0;


// ==============================================================================
// SETUP
// ==============================================================================

void setup()
{
  // --------------------------------------------------------------------------
  // Serial
  // --------------------------------------------------------------------------

  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("   PK-LORA V2 RO - SENSOR UPLINK");
  Serial.println("   ESP8266 + RFM96");
  Serial.println("========================================");


  // --------------------------------------------------------------------------
  // LED
  // --------------------------------------------------------------------------

  pinMode(LED_VERMELHO_PIN, OUTPUT);

  digitalWrite(LED_VERMELHO_PIN, LOW);


  // --------------------------------------------------------------------------
  // SPI
  //
  // No ESP8266 os pinos SPI são fixos:
  //
  // D5 -> SCK
  // D6 -> MISO
  // D7 -> MOSI
  //
  // Por isso usamos simplesmente:
  //
  // SPI.begin();
  // --------------------------------------------------------------------------

  SPI.begin();


  // --------------------------------------------------------------------------
  // Configuração dos pinos do RFM96
  // --------------------------------------------------------------------------

  LoRa.setPins(
    NSS_PIN,
    RST_PIN,
    DIO0_PIN
  );


  // --------------------------------------------------------------------------
  // Inicialização do LoRa
  // --------------------------------------------------------------------------

  Serial.println("Inicializando RFM96...");

  if (!LoRa.begin(FREQUENCY_IN_HZ))
  {
    Serial.println("ERRO: nao foi possivel iniciar o RFM96!");

    // Pisca o LED continuamente indicando erro
    while (1)
    {
      digitalWrite(LED_VERMELHO_PIN, HIGH);
      delay(200);

      digitalWrite(LED_VERMELHO_PIN, LOW);
      delay(200);
    }
  }


  // --------------------------------------------------------------------------
  // Configuração dos parâmetros do LoRa
  // --------------------------------------------------------------------------

  LoRa.setTxPower(txPower);

  LoRa.setSpreadingFactor(spreadingFactor);

  LoRa.setSignalBandwidth(signalBandwidth);

  LoRa.setCodingRate4(codingRateDenominator);


  // --------------------------------------------------------------------------
  // Inicialização concluída
  // --------------------------------------------------------------------------

  Serial.println("RFM96 iniciado com sucesso!");

  Serial.print("Frequencia: ");
  Serial.print(FREQUENCY_IN_HZ / 1000000.0);
  Serial.println(" MHz");

  Serial.print("Potencia TX: ");
  Serial.print(txPower);
  Serial.println(" dBm");

  Serial.print("Spreading Factor: ");
  Serial.println(spreadingFactor);

  Serial.println("========================================");


  // --------------------------------------------------------------------------
  // Pisca LED indicando inicialização bem-sucedida
  // --------------------------------------------------------------------------

  digitalWrite(LED_VERMELHO_PIN, HIGH);

  delay(1000);

  digitalWrite(LED_VERMELHO_PIN, LOW);
}


// ==============================================================================
// LOOP
// ==============================================================================

void loop()
{
  // --------------------------------------------------------------------------
  // FLUXO DE ENVIO
  //
  // Nó Sensor -> Gateway LoRa
  // --------------------------------------------------------------------------


  // --------------------------------------------------------------------------
  // LED inicialmente desligado
  // --------------------------------------------------------------------------

  digitalWrite(LED_VERMELHO_PIN, LOW);


  // --------------------------------------------------------------------------
  // Zera o pacote
  // --------------------------------------------------------------------------

  for (int i = 0; i < TAMANHO_PACOTE; i++)
  {
    Pacote_UL[i] = 0;
  }


  // --------------------------------------------------------------------------
  // Leitura do LDR
  //
  // No ESP8266:
  //
  // analogRead(A0)
  //
  // Retorna normalmente um valor entre 0 e 1023.
  // --------------------------------------------------------------------------

  luminosidade = analogRead(LDR_PIN);


  // --------------------------------------------------------------------------
  // Mostra luminosidade no Serial Monitor
  // --------------------------------------------------------------------------

  Serial.print("Luminosidade: ");
  Serial.println(luminosidade);


  // --------------------------------------------------------------------------
  // Incrementa contador de pacotes
  // --------------------------------------------------------------------------

  contador_pacotes++;


  // --------------------------------------------------------------------------
  // Coloca luminosidade nos bytes 18 e 19
  //
  // Byte 18 -> MSB
  // Byte 19 -> LSB
  // --------------------------------------------------------------------------

  Pacote_UL[18] = (byte)(luminosidade / 256);

  Pacote_UL[19] = (byte)(luminosidade % 256);


  // --------------------------------------------------------------------------
  // Coloca contador nos bytes 12 e 13
  //
  // Byte 12 -> MSB
  // Byte 13 -> LSB
  // --------------------------------------------------------------------------

  Pacote_UL[12] = (byte)(contador_pacotes / 256);

  Pacote_UL[13] = (byte)(contador_pacotes % 256);


  // --------------------------------------------------------------------------
  // INÍCIO DA TRANSMISSÃO
  // --------------------------------------------------------------------------

  Serial.print("Enviando pacote: ");
  Serial.println(contador_pacotes);


  LoRa.beginPacket();


  // --------------------------------------------------------------------------
  // Envia os 20 bytes do pacote
  // --------------------------------------------------------------------------

  for (int i = 0; i < TAMANHO_PACOTE; i++)
  {
    LoRa.write(Pacote_UL[i]);

    // LED aceso durante o envio
    digitalWrite(LED_VERMELHO_PIN, HIGH);
  }


  // --------------------------------------------------------------------------
  // Finaliza transmissão
  // --------------------------------------------------------------------------

  LoRa.endPacket();


  // --------------------------------------------------------------------------
  // LED desligado
  // --------------------------------------------------------------------------

  digitalWrite(LED_VERMELHO_PIN, LOW);


  Serial.println("Pacote enviado!");
  Serial.println();


  // --------------------------------------------------------------------------
  // Aguarda 1 segundo
  // --------------------------------------------------------------------------

  delay(1000);
}
