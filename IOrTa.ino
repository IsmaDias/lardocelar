/*Programa: Planta IoT com ESP8266 NodeMCU e MQTT
 * Automacao da plantinha, lendo sensor de umidade do solo, usando o ESP12E. Comunicacao atraves de MQTT e Thingverse Projeto baseado em -> http://blog.filipeflop.com/arduino/planta-iot-com-esp8266-nodemcu-parte-3.html

    Lista de Materiais
          ESP12E
          Sensor de umidade do solo
          Rele 5V
          Bomba de agua (127 V)
          Fita de LED (127 V)
    
   Funcionamento
    
          O ESP12E le a cada 30 segundos o valor de umidade do solo, entrada analogica A0.
          Envia para os servidores Host: api.thingspeak.com e iot.eclipse.org
          Os dados podem ser acessados remotamente (web ou mobile)
          Se a umidade do solo ficar menor que 20% o rele eh acionado, ligando a bomba de agua e a fita de LED (mesmo barramento)
          Quando a umidade do solo for maior que 40% ele desliga
          Criar as opcoes de ligar a fita de led e a bomba de agua remotamente
 */
#include <ESP8266WiFi.h>  //essa biblioteca já vem com a IDE. Portanto, não é preciso baixar nenhuma biblioteca adicional
#include <PubSubClient.h> // Importa a Biblioteca PubSubClient
 
//defines
/*================================ defines =======================================================*/
#define SSID_REDE     "Bingo"                //coloque aqui o nome da rede que se deseja conectar
#define SENHA_REDE    "lardocelar"           //coloque aqui a senha da rede que se deseja conectar
#define INTERVALO_ENVIO_THINGSPEAK  30000    //intervalo entre envios de dados ao ThingSpeak (em ms)
#define INTERVALO_ENVIO_MQTT        10000    //intervalo entre envios de dados via MQTT (em ms)
  
//defines de id mqtt e tópicos para publicação e subscribe
#define TOPICO_SUBSCRIBE "MQTTLARDOCELAREnvia"     //tópico MQTT de escuta
#define TOPICO_PUBLISH   "MQTTLARDOCELARRecebe"    //tópico MQTT de envio de informações para Broker
                                               
#define ID_MQTT  "IOrTaLARDOCELAR"     //id mqtt (para identificação de sessão)

#define UMIDADEMINIMA 20              // a minima umidade percentual, serve para acionar o rele
#define UMIDADEIDEAL  50              // valor alvo do acionamento do rele
#define UMIDADEMAXIMA 60              // maxima umidade percentual da planta

                          
/*================================constantes e variáveis globais===================================*/
const char* BROKER_MQTT = "iot.eclipse.org";                //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883;                                     // Porta do Broker MQTT
char EnderecoAPIThingSpeak[] = "api.thingspeak.com";
String ChaveEscritaThingSpeak = "VOJXHI5QVBF9Y05H";         //coloque aqui sua chave de escrita do seu canal no ThingSpeak
long lastConnectionTime; 
long lastMQTTSendTime;
WiFiClient client;
WiFiClient clientMQTT;
PubSubClient MQTT(clientMQTT);                              // Instancia o Cliente MQTT passando o objeto clientMQTT
  
//prototypes
void EnviaInformacoesThingspeak(String StringDados);
float FazLeituraUmidade(void);
void initWiFi(void);
void initMQTT(void);
void reconectWiFi(void); 
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void VerificaConexoesWiFIEMQTT(void); 
 
/*=================================Implementações====================================== */
  
//Função: envia informações ao ThingSpeak
//Parâmetros: String com a  informação a ser enviada
//Retorno: nenhum
void EnviaInformacoesThingspeak(String StringDados)
{
    if (client.connect(EnderecoAPIThingSpeak, 80))
    {         
        //faz a requisição HTTP ao ThingSpeak
        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+ChaveEscritaThingSpeak+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(StringDados.length());
        client.print("\n\n");
        client.print(StringDados);
    
        lastConnectionTime = millis();
        Serial.println("- Informações enviadas ao ThingSpeak!");
     }   
}
  
//Função: inicializa e conecta-se na rede WI-FI desejada
//Parâmetros: nenhum
//Retorno: nenhum
void initWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID_REDE);
    Serial.println("Aguarde");
      
    reconectWiFi();
}
   
//Função: inicializa parâmetros de conexão MQTT(endereço do 
//        broker, porta e seta função de callback)
//Parâmetros: nenhum
//Retorno: nenhum
void initMQTT() 
{
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);   //informa qual broker e porta deve ser conectado
    MQTT.setCallback(mqtt_callback);            //atribui função de callback (função chamada quando qualquer informação de um dos tópicos subescritos chega)
}
   
//Função: função de callback 
//        esta função é chamada toda vez que uma informação de 
//        um dos tópicos subescritos chega)
//Parâmetros: nenhum
//Retorno: nenhum
void mqtt_callback(char* topic, byte* payload, unsigned int length) 
{
         
}
   
//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
    while (!MQTT.connected()) 
    {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) 
        {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE); 
        } 
        else
        {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Havera nova tentatica de conexao em 2s");
            delay(2000);
        }
    }
}
   
//Função: reconecta-se ao WiFi
//Parâmetros: nenhum
//Retorno: nenhum
void reconectWiFi() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
          
    WiFi.begin(SSID_REDE, SENHA_REDE); // Conecta na rede WI-FI
      
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID_REDE);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());
}
  
//Função: verifica o estado das conexões WiFI e ao broker MQTT. 
//        Em caso de desconexão (qualquer uma das duas), a conexão
//        é refeita.
//Parâmetros: nenhum
//Retorno: nenhum
void VerificaConexoesWiFIEMQTT(void)
{
    if (!MQTT.connected()) 
        reconnectMQTT(); //se não há conexão com o Broker, a conexão é refeita
      
     reconectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
  
//Função: faz a leitura do nível de umidade
//Parâmetros: nenhum
//Retorno: umidade percentual (0-100)
//Observação: o ADC do NodeMCU permite até, no máximo, 1V. Dessa forma,
//            para 1V, obtem-se (empiricamente) 418 como leitura de ADC
float FazLeituraUmidade(void)
{
    int ValorADC;
    float UmidadePercentual;
  
     ValorADC = analogRead(0);   //418 -> 1.0V
     Serial.print("[Leitura ADC] ");
     Serial.println(ValorADC);
  
     //Quanto maior o numero lido do ADC, menor a umidade.
     //Sendo assim, calcula-se a porcentagem de umidade por:
     //      
     //   Valor lido                 Umidade percentual
     //      _    0                           _ 100
     //      |                                |   
     //      |                                |   
     //      -   ValorADC                     - UmidadePercentual 
     //      |                                |   
     //      |                                |   
     //     _|_  418                         _|_ 0
     //
     //   (UmidadePercentual-0) / (100-0)  =  (ValorADC - 418) / (-418)
     //      Logo:
     //      UmidadePercentual = 100 * ((418-ValorADC) / 418)  
       
     UmidadePercentual = 100 * ((418-(float)ValorADC) / 418);
     Serial.print("[Umidade Percentual] ");
     Serial.print(UmidadePercentual);
     Serial.println("%");
  
     return UmidadePercentual;
}



//Função: configura rele
//Parâmetros: nenhum
//Retorno: nenhum
//Observação: essa funcao 
void ligaRELE( float UmidadePercentual ){
if (UmidadePercentual <= UMIDADEMINIMA)
{
  digitalWrite(RELE, LOW);      // aciona o GPIO do rele, ele tem logica negada (liga em LOW e desliga em HIGH)
  while(UmidadePercentual < UMIDADEIDEAL)
  {
      UmidadePercentual = FazLeituraUmidade(void);
      delay(50);
  }
  return;
}
  
}


//Função: aciona o rele
//Parâmetros: umidade do solo
//Retorno: nenhum
//Observação: essa funcao executa por 10 segundos
void ligaRELE( float UmidadePercentual ){
while (UmidadePercentual <= UMIDADEMINIMA)
{
  digitalWrite(RELE, LOW);      // aciona o GPIO do rele, ele tem logica negada (liga em LOW e desliga em HIGH)
}
  
}
void setup()
{  
    Serial.begin(9600);
    
    lastConnectionTime = 0; 
    lastMQTTSendTime = 0;
    initWiFi();
    initMQTT();
    Serial.println("Planta IoT com ESP8266 NodeMCU");
}
  
/*=====================================loop principal==============================*/
void loop()
{
    float UmidadePercentualLida;
    int UmidadePercentualTruncada;
    char FieldUmidade[11];
    char MsgUmidadeMQTT[50];
      
    VerificaConexoesWiFIEMQTT(); 
      
    //Força desconexão ao ThingSpeak (se ainda estiver desconectado)
    if (client.connected())
    {
        client.stop();
        Serial.println("- Desconectado do ThingSpeak");
        Serial.println();
    }
  
    UmidadePercentualLida = FazLeituraUmidade();
    UmidadePercentualTruncada = (int)UmidadePercentualLida; //trunca umidade como número inteiro
      
    //verifica se está conectado no WiFi e se é o momento de enviar dados ao ThingSpeak
    if(!client.connected() && 
      ((millis() - lastConnectionTime) > INTERVALO_ENVIO_THINGSPEAK))
    {
        sprintf(FieldUmidade,"field1=%d",UmidadePercentualTruncada);
        EnviaInformacoesThingspeak(FieldUmidade);
    }
  
    //verifica se é o momento de enviar informações via MQTT
    if ((millis() - lastMQTTSendTime) > INTERVALO_ENVIO_MQTT)
    {
        sprintf(MsgUmidadeMQTT,"- Umidade do solo: %d porcento.",UmidadePercentualTruncada);
        MQTT.publish(TOPICO_PUBLISH, MsgUmidadeMQTT);
        lastMQTTSendTime = millis();
    }
     
    delay(1000);
}
