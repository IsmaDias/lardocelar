# lardocelar
Repositorio para os projetos na casa da Bianca &lt;3

# PROJETO 01 - IOrTa
Automacao da plantinha, lendo sensor de umidade do solo, usando o ESP12E.
Comunicacao atraves de MQTT e Thingverse
Projeto baseado em -> http://blog.filipeflop.com/arduino/planta-iot-com-esp8266-nodemcu-parte-3.html

## Lista de Materiais
  - ESP12E
  - Sensor de umidade do solo
  - Rele 5V
  - Bomba de agua (127 V)
  - Fita de LED (127 V)
  
## Funcionamento
  - O ESP12E le a cada 30 segundos o valor de umidade do solo, entrada analogica A0. 
  - Envia para os servidores Host: api.thingspeak.com e iot.eclipse.org
  - Os dados podem ser acessados remotamente (web ou mobile)
  - Se a umidade do solo ficar menor que 20% o rele eh acionado, ligando a bomba de agua e a fita de LED (mesmo barramento)
  - Quando a umidade do solo for maior que 40% ele desliga
  - Criar as opcoes de ligar a fita de led e a bomba de agua remotamente
  
