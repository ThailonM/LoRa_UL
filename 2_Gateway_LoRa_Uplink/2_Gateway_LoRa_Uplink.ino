
// ==============================================================================
// Teste de UPLINK - UL
// Gateway LoRa recebe a cada 1 segundo a luminosidade
// Também mede a potência rádio recebida
//
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
// SPI padrão do ESP8266:
//
// D5 / GPIO14 -> SCK
// D6 / GPIO12 -> MISO
// D7 / GPIO13 -> MOSI
//
// Controle do RFM96:
//
// D8 / GPIO15 -> NSS
// D3 / GPIO0  -> RESET
// D1 / GPIO5  -> DIO0
//
// ==============================================================================

#define NSS_PIN   15   // D8
#define RST_PIN    0   // D3
#define DIO0_PIN   5   // D1


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


// ==============================================================================
// PACOTE DE COMUNICAÇÃO
// ==============================================================================

byte Pacote_UL[TAMANHO_PACOTE];


// ==============================================================================
// VARIÁVEIS DE MEDIÇÃO
// ==============================================================================

// RSSI real recebido pelo RFM96
float rssi_ul_real_dbm;

// RSSI convertido para 1 byte
byte rssi_ul_convertido;


// ==============================================================================
// SETUP
// ==============================================================================

void setup()
{
  // --------------------------------------------------------------------------
  // Comunicação Serial
  // --------------------------------------------------------------------------

  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("       GATEWAY LoRa - UPLINK");
  Serial.println("       PK-LORA V2 RO");
  Serial.println("       ESP8266 + RFM96");
  Serial.println("========================================");


  // --------------------------------------------------------------------------
  // Inicialização do SPI
  //
  // ESP8266 utiliza os pinos SPI padrão:
  //
  // D5 -> SCK
  // D6 -> MISO
  // D7 -> MOSI
  //
  // Por isso:
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

    // Para o programa caso o LoRa não inicialize
    while (1)
    {
      delay(1000);
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
  // Coloca o rádio em modo de recepção
  // --------------------------------------------------------------------------

  LoRa.receive();


  // --------------------------------------------------------------------------
  // Mensagens de inicialização
  // --------------------------------------------------------------------------

  Serial.println("RFM96 iniciado com sucesso!");

  Serial.print("Frequencia: ");
  Serial.print(FREQUENCY_IN_HZ / 1000000.0);
  Serial.println(" MHz");

  Serial.print("Spreading Factor: ");
  Serial.println(spreadingFactor);

  Serial.print("Bandwidth: ");
  Serial.print(signalBandwidth / 1000.0);
  Serial.println(" kHz");

  Serial.println("Gateway aguardando pacotes...");
  Serial.println("========================================");
}


// ==============================================================================
// LOOP
// ==============================================================================

void loop()
{
  // --------------------------------------------------------------------------
  // Verifica se chegou algum pacote
  // --------------------------------------------------------------------------

  uint8_t packetSize = LoRa.parsePacket();


  // --------------------------------------------------------------------------
  // Caso tenha recebido um pacote
  // --------------------------------------------------------------------------

  if (packetSize > 0)
  {

    Serial.print("Pacote recebido - tamanho: ");
    Serial.println(packetSize);


    // ------------------------------------------------------------------------
    // Verifica se o pacote possui o tamanho esperado
    // ------------------------------------------------------------------------

    if (packetSize >= TAMANHO_PACOTE)
    {

      // ----------------------------------------------------------------------
      // Leitura dos 20 bytes do pacote
      // ----------------------------------------------------------------------

      for (int i = 0; i < TAMANHO_PACOTE; i++)
      {
        Pacote_UL[i] = LoRa.read();
      }


      // ----------------------------------------------------------------------
      // Leitura do RSSI
      // ----------------------------------------------------------------------

      rssi_ul_real_dbm = LoRa.packetRssi();


      Serial.print("RSSI: ");
      Serial.print(rssi_ul_real_dbm);
      Serial.println(" dBm");


      // ----------------------------------------------------------------------
      // Conversão do RSSI para 1 byte
      // ----------------------------------------------------------------------
      //
      // Mantida a mesma lógica do código original.
      //
      // Faixa:
      //
      // RSSI > -10.5 dBm
      //       -> 127
      //
      // -10.5 >= RSSI >= -74
      //       -> conversão direta
      //
      // RSSI < -74
      //       -> representação em complemento de 2
      //
      // ----------------------------------------------------------------------

      if (rssi_ul_real_dbm > -10.5)
      {
        rssi_ul_convertido = 127;
      }

      else if (
        rssi_ul_real_dbm <= -10.5 &&
        rssi_ul_real_dbm >= -74
      )
      {
        rssi_ul_convertido =
          (byte)((rssi_ul_real_dbm + 74) * 2);
      }

      else
      {
        rssi_ul_convertido =
          (byte)(((rssi_ul_real_dbm + 74) * 2) + 256);
      }


      // ----------------------------------------------------------------------
      // Coloca o RSSI no pacote
      // ----------------------------------------------------------------------

      Pacote_UL[2] = rssi_ul_convertido;

      Pacote_UL[0] = rssi_ul_convertido;


      // ----------------------------------------------------------------------
      // Envia o pacote pela USB/Serial
      //
      // IMPORTANTE:
      //
      // O Serial.write() envia os bytes crus do pacote.
      // Não usamos Serial.println() aqui.
      //
      // ----------------------------------------------------------------------

      for (int i = 0; i < TAMANHO_PACOTE; i++)
      {
        Serial.write(Pacote_UL[i]);
      }


      // ----------------------------------------------------------------------
      // Volta o LoRa para modo de recepção
      // ----------------------------------------------------------------------

      LoRa.receive();
    }

    else
    {
      // ----------------------------------------------------------------------
      // Pacote menor que o esperado
      // ----------------------------------------------------------------------

      Serial.print("Pacote ignorado. Tamanho recebido: ");
      Serial.println(packetSize);

      // Limpa bytes restantes do pacote recebido
      while (LoRa.available())
      {
        LoRa.read();
      }

      LoRa.receive();
    }
  }
}
