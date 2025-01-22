#include <WiFi.h>
#include <PubSubClient.h>
#include <RoboCore_Vespa.h>
#include <ArduinoJson.h>

// ------------------- PINOS E SENSORES -------------------
const uint8_t PINO_HCSR04_ECHO     = 26;
const uint8_t PINO_HCSR04_TRIGGER  = 25;
const int PINO_SENSOR_VELOCIDADE   = 32;

// ------------------- VARIÁVEIS PARA O SENSOR DE VELOCIDADE -------------------
volatile unsigned long lastDetectionTime = 0;
volatile unsigned long timeInterval      = 0;

/*
  Se a distância entre marcas for 10,4 cm => 0,104 m.
  Ajuste conforme o diâmetro e número de marcas que você tiver.
*/
const float distanceBetweenMarks = 0.104;

// Debounce do sensor (antirruído)
const unsigned long debounceDelay = 5000; // 5 µs

// ------------------- CONFIGURAÇÕES DE Wi-Fi -------------------
const char* ssid     = " ";
const char* password = " ";

// ------------------- CONFIGURAÇÕES DE MQTT -------------------
const char* mqtt_server          = " "; 
const char* mqtt_topic_comandos  = "robo/comandos";
const char* mqtt_topic_bateria   = "robo/bateria";
const char* mqtt_topic_velocidade= "robo/velocidade";

// ------------------- CLIENTES Wi-Fi E MQTT -------------------
WiFiClient espClient;
PubSubClient client(espClient);

// ------------------- OBJETOS MOTORES E BATERIA -------------------
VespaMotors motores;
VespaBattery vbat;

// ------------------- TEMPOS DE PUBLICAÇÃO -------------------
const uint32_t TEMPO_ATUALIZACAO_VBAT       = 5000; // 5s
const uint32_t TEMPO_ATUALIZACAO_VELOCIDADE = 1000; // 1s
uint32_t timeout_vbat;
uint32_t timeout_velocidade;

// ------------------- PARÂMETROS PARA CONTROLADOR P NO MODO AUTÔNOMO -------------------
// Velocidade alvo = 0,2 m/s
const float V_REF  = 0.22f;    
// Ganho do controlador
const float P_GAIN = 3.0f;  
// Offset de PWM
const float BASE_POWER = 74.60f;

// ------------------- CONTROLE DO ROBÔ -------------------
int16_t velocidade = 0;
int16_t angulo     = 0;
float   velocidadeAtual = 0.0f; // Velocidade medida

// ------------------- CONEXÃO E MODO AUTÔNOMO -------------------
unsigned long ultimaTentativaConexao    = 0; 
const unsigned long TEMPO_MAXIMO_RECONEXAO = 10000; // 10 s
bool modoAutonomo = false; 

// ------------------- ANTICOLISÃO E MANOBRAS -------------------
const int DISTANCIA_OBSTACULO = 25; 
const int ESPERA_MOVIMENTO    = 250;
int distancia;

// ------------------- VELOCIDADE AUTÔNOMA P/ DESVIAR/RECUAR -------------------
const int VELOCIDADE_AUTONOMA = 80;  

// ------------------- ZERAR SE 3 s SEM PULSOS -------------------

unsigned long lastPulseTime               = 0;
const unsigned long TEMPO_RESET_VELOCIDADE= 3000; 
float ultimaVelocidadeImpressa            = -1.0f;
const float LIMITE_MINIMO_VELOCIDADE      = 0.05f;  

// ---------------------------------------------------
// Conectar ao Wi-Fi
// ---------------------------------------------------

void setup_wifi() {
    delay(10);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        // prints omitidos
    }
}

// ---------------------------------------------------
// Callback MQTT (recebe dados do broker)
// ---------------------------------------------------

void callback(char* topic, byte* payload, unsigned int length) {
    if (modoAutonomo) return; 

    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        return;
    }

    if (strcmp(topic, mqtt_topic_comandos) == 0) {
        velocidade = doc["velocidade"];
        angulo     = doc["angulo"];

        // Ajustar ângulo <0 => 0..360
        if (angulo < 0) {
            angulo = 360 + angulo;
        }

        // Movimento manual (não-autônomo)
        if ((angulo >= 90) && (angulo <= 180)) {
            motores.turn(-velocidade, -velocidade * (135 - angulo) / 45);
        } 
        else if ((angulo >= 0) && (angulo < 90)) {
            motores.turn(-velocidade * (angulo - 45) / 45, -velocidade);
        }
        else if ((angulo > 180) && (angulo <= 270)) {
            motores.turn(velocidade * (angulo - 225) / 45, velocidade);
        }
        else if (angulo > 270) {
            motores.turn(velocidade, velocidade * (315 - angulo) / 45);
        }
        else {
            motores.stop();
        }
    } 
    else {
        // Se for outro tópico, parar
        motores.stop();
    }
}

// ---------------------------------------------------
// Conectar ao broker MQTT
// ---------------------------------------------------

void reconnect() {
    if (!client.connected()) {
        unsigned long agora = millis();
        if (agora - ultimaTentativaConexao > 5000) {
            if (client.connect("ESP32Client")) {
                client.subscribe(mqtt_topic_comandos);

                // Se estava em modo autônomo, desligar
                if (modoAutonomo) {
                    modoAutonomo = false;
                    motores.stop();
                }
                ultimaTentativaConexao = millis();
            } 
            else {
                // Falha
            }
        }

        if (millis() - ultimaTentativaConexao >= TEMPO_MAXIMO_RECONEXAO && !modoAutonomo) {
            // Habilitar modo autônomo se não conseguiu reconectar
            modoAutonomo = true;
        }
    }
}

// ---------------------------------------------------
// Enviar tensão da bateria via MQTT
// ---------------------------------------------------

void enviarTensaoBateria() {
    if (millis() > timeout_vbat) {
        uint32_t tensao = vbat.readVoltage();
        StaticJsonDocument<128> doc;
        doc["vbat"] = tensao;

        char mensagem[128];
        serializeJson(doc, mensagem);
        client.publish(mqtt_topic_bateria, mensagem);

        // Atualiza timer
        timeout_vbat = millis() + TEMPO_ATUALIZACAO_VBAT;
    }
}

// ---------------------------------------------------
// Publicar velocidade no MQTT
// ---------------------------------------------------

void publicarVelocidade() {
    if (millis() > timeout_velocidade) {
        StaticJsonDocument<128> doc;
        doc["velocidade"] = velocidadeAtual;

        char mensagem[128];
        serializeJson(doc, mensagem);
        client.publish(mqtt_topic_velocidade, mensagem);

        // Atualiza timer
        timeout_velocidade = millis() + TEMPO_ATUALIZACAO_VELOCIDADE;
    }
}

// ---------------------------------------------------
// Leitura do sensor ultrassonico
// ---------------------------------------------------

int sensor_ultrassonico(int pinotrigger, int pinoecho) {
    digitalWrite(pinotrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinotrigger, LOW);

    return pulseIn(pinoecho, HIGH) / 58; // cm
}

// ---------------------------------------------------
// Modo autônomo (anticolisão) + controlador P
// ---------------------------------------------------

void modoAutonomoAnticolisao() {
    distancia = sensor_ultrassonico(PINO_HCSR04_TRIGGER, PINO_HCSR04_ECHO);

    // Se obstáculo muito perto, recuar e desviar
    if (distancia <= DISTANCIA_OBSTACULO) {
        motores.stop();
        motores.backward(VELOCIDADE_AUTONOMA);
        delay(ESPERA_MOVIMENTO);
        motores.stop();

        if (millis() % 2 == 0) {
            motores.turn(-VELOCIDADE_AUTONOMA, VELOCIDADE_AUTONOMA);
        } else {
            motores.turn(VELOCIDADE_AUTONOMA, -VELOCIDADE_AUTONOMA);
        }
        delay(ESPERA_MOVIMENTO);
        motores.stop();
    }
    else {
        // Controlador P
        float erro  = V_REF - velocidadeAtual;  
        float Pcalc = P_GAIN * erro + BASE_POWER;

        // saturar [0..100]
        if (Pcalc < 0)    Pcalc = 0;
        if (Pcalc > 100)  Pcalc = 100;

        // Log de debug
        Serial.printf("erro=%.2f, Pcalc=%.2f, velAtual=%.2f\n", erro, Pcalc, velocidadeAtual);

        // Aplicar PWM calculado
        motores.forward((int)Pcalc);
    }
}

// ---------------------------------------------------
// Interrupção do sensor de velocidade
// ---------------------------------------------------

void sensorInterrupt() {
    unsigned long currentTime = micros();
    if ((currentTime - lastDetectionTime) > debounceDelay) {
        if (lastDetectionTime != 0) {
            timeInterval = currentTime - lastDetectionTime;
        }
        lastDetectionTime = currentTime;
        lastPulseTime     = millis();
    }
}

// ---------------------------------------------------
// Calcular velocidade
// ---------------------------------------------------

void calcularVelocidade() {
    noInterrupts();
    unsigned long interval = timeInterval;
    timeInterval = 0;
    interrupts();

    bool tevePulsoNovo = false;

    if (interval > 0) {
        float timeIntervalInSeconds = interval / 100000.0; 
        float novaVelocidade        = distanceBetweenMarks / timeIntervalInSeconds;

        if (novaVelocidade >= LIMITE_MINIMO_VELOCIDADE) {
            velocidadeAtual = novaVelocidade;
        }
        tevePulsoNovo = true;
    }

    if (millis() - lastPulseTime >= TEMPO_RESET_VELOCIDADE) {
        if (velocidadeAtual != 0.0f) {
            velocidadeAtual = 0.0f;
            Serial.println("Velocidade: 0.00 m/s");
            ultimaVelocidadeImpressa = 0.0f;
        }
    }
    else {
        // Imprime só se houve pulso novo e a velocidade for diferente da anterior
        if (tevePulsoNovo && (velocidadeAtual != ultimaVelocidadeImpressa)) {
            Serial.print("Velocidade: ");
            Serial.print(velocidadeAtual);
            Serial.println(" m/s");
            ultimaVelocidadeImpressa = velocidadeAtual;
        }
    }
}

// ---------------------------------------------------
// SETUP
// ---------------------------------------------------

void setup() {
    Serial.begin(115200);

    setup_wifi(); 
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    pinMode(PINO_HCSR04_TRIGGER, OUTPUT);
    pinMode(PINO_HCSR04_ECHO,    INPUT);

    pinMode(PINO_SENSOR_VELOCIDADE, INPUT);
    attachInterrupt(digitalPinToInterrupt(PINO_SENSOR_VELOCIDADE), sensorInterrupt, FALLING);

    // Inicializa timers
    timeout_vbat       = millis();
    timeout_velocidade = millis();
}

// ---------------------------------------------------
// LOOP
// ---------------------------------------------------

void loop() {
    // Verificar conexão ao broker MQTT
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    // Calcular velocidade e publicar
    calcularVelocidade();
    publicarVelocidade();

    // Enviar tensão da bateria
    enviarTensaoBateria();

    // Se estiver em modo autônomo, usar anticolisão + controlador
    if (modoAutonomo) {
        modoAutonomoAnticolisao();
    }
}
