#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

// --- MAPEAMENTO MOTORES ---
const int MOTOR_ESQ_1 = 4; 
const int MOTOR_ESQ_2 = 5; 
const int MOTOR_DIR_1 = 6; // CUIDADO: Pino de Boot
const int MOTOR_DIR_2 = 1; // CUIDADO: Pino de Serial TX

// --- MAPEAMENTO ARMA (L298N) ---
const int ARMA_IN2 = 7;
const int ARMA_IN3 = 10;

// Configurações PWM
const int freq = 5000;
const int resolution = 8;     
const int velocidade = 200; 

const int CH_ESQ_1 = 0;
const int CH_ESQ_2 = 1;
const int CH_DIR_1 = 2;
const int CH_DIR_2 = 3;

// --- VARIÁVEIS DE SEGURANÇA (FAILSAFE) ---
unsigned long ultimaComunicacao = 0;
const int TIMEOUT_MS = 300; // Se ficar 300ms sem sinal, o robô para
bool estadoArma = false;

void pararTudo() {
  // Para Tração
  ledcWrite(CH_ESQ_1, 0); ledcWrite(CH_ESQ_2, 0);
  ledcWrite(CH_DIR_1, 0); ledcWrite(CH_DIR_2, 0);
  
  // Desliga Arma
  digitalWrite(ARMA_IN2, LOW);
  digitalWrite(ARMA_IN3, LOW);
  estadoArma = false;
}

void setup() {
  // NOTA: Serial desativada para evitar conflito nos pinos 0 e 1
  
  Dabble.begin("Cupim_Dabble"); 

  pinMode(ARMA_IN2, OUTPUT);
  pinMode(ARMA_IN3, OUTPUT);
  digitalWrite(ARMA_IN2, LOW);
  digitalWrite(ARMA_IN3, LOW);

  ledcSetup(CH_ESQ_1, freq, resolution);
  ledcSetup(CH_ESQ_2, freq, resolution);
  ledcSetup(CH_DIR_1, freq, resolution);
  ledcSetup(CH_DIR_2, freq, resolution);

  ledcAttachPin(MOTOR_ESQ_1, CH_ESQ_1);
  ledcAttachPin(MOTOR_ESQ_2, CH_ESQ_2);
  ledcAttachPin(MOTOR_DIR_1, CH_DIR_1);
  ledcAttachPin(MOTOR_DIR_2, CH_DIR_2);

  pararTudo();
  ultimaComunicacao = millis();
}

void loop() {
  Dabble.processInput(); 

  // --- LÓGICA DO WATCHDOG ---
  // Se houver qualquer comando vindo do Gamepad, reseta o cronômetro
  if (GamePad.isUpPressed() || GamePad.isDownPressed() || 
      GamePad.isLeftPressed() || GamePad.isRightPressed() || 
      GamePad.isTrianglePressed() || GamePad.isCirclePressed()) {
      ultimaComunicacao = millis();
  }

  // Verifica se a conexão está ativa e dentro do tempo limite
  if (Dabble.isAppConnected() && (millis() - ultimaComunicacao < TIMEOUT_MS)) {
    
    // --- MOVIMENTAÇÃO ---
    if (GamePad.isUpPressed()) {
      ledcWrite(CH_ESQ_1, velocidade); ledcWrite(CH_ESQ_2, 0);
      ledcWrite(CH_DIR_1, velocidade); ledcWrite(CH_DIR_2, 0);
    }
    else if (GamePad.isDownPressed()) {
      ledcWrite(CH_ESQ_1, 0); ledcWrite(CH_ESQ_2, velocidade);
      ledcWrite(CH_DIR_1, 0); ledcWrite(CH_DIR_2, velocidade);
    }
    else if (GamePad.isLeftPressed()) {
      ledcWrite(CH_ESQ_1, 0); ledcWrite(CH_ESQ_2, velocidade);
      ledcWrite(CH_DIR_1, velocidade); ledcWrite(CH_DIR_2, 0);
    }
    else if (GamePad.isRightPressed()) {
      ledcWrite(CH_ESQ_1, velocidade); ledcWrite(CH_ESQ_2, 0);
      ledcWrite(CH_DIR_1, 0); ledcWrite(CH_DIR_2, velocidade);
    }
    else {
      // Para a tração se nenhum botão de direção estiver pressionado
      ledcWrite(CH_ESQ_1, 0); ledcWrite(CH_ESQ_2, 0);
      ledcWrite(CH_DIR_1, 0); ledcWrite(CH_DIR_2, 0);
    }

    // --- CONTROLE DA ARMA ---
    // Triângulo: Liga/Desliga (Toggle)
    if (GamePad.isTrianglePressed()) {
      estadoArma = !estadoArma;
      digitalWrite(ARMA_IN2, estadoArma ? HIGH : LOW);
      digitalWrite(ARMA_IN3, estadoArma ? LOW : HIGH); 
      delay(250); // Debounce para não oscilar
    }

    // Círculo: Desliga Imediato (Botão de Pânico)
    if (GamePad.isCirclePressed()) {
      estadoArma = false;
      digitalWrite(ARMA_IN2, LOW);
      digitalWrite(ARMA_IN3, LOW);
      delay(250);
    }

  } else {
    // --- MODO FAILSAFE ATIVADO ---
    // Entra aqui se isAppConnected for falso OU se estourar o TIMEOUT_MS
    pararTudo();
  }
}