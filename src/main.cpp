#include <Arduino.h>
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

// --- MAPEAMENTO PINOS (ESP32 Clássico) ---
const int MOTOR_ESQ_1 = 16; 
const int MOTOR_ESQ_2 = 17; 
const int MOTOR_DIR_1 = 18; 
const int MOTOR_DIR_2 = 21; 
const int ARMA_PIN    = 23; 

// --- VARIÁVEIS DE CONTROLE ---
unsigned long ultimaComunicacao = 0;
const int TIMEOUT_MS = 300; 
bool estadoArma = false;
bool conectadoAnteriormente = false;

// --- FUNÇÕES DE MOVIMENTO (DIGITAL) ---

void freioAtivoRodas() {
  // Mini L298N freia quando os dois pinos do motor recebem HIGH
  digitalWrite(MOTOR_ESQ_1, HIGH); digitalWrite(MOTOR_ESQ_2, HIGH);
  digitalWrite(MOTOR_DIR_1, HIGH); digitalWrite(MOTOR_DIR_2, HIGH);
}

void desligarTudo() {
  digitalWrite(MOTOR_ESQ_1, LOW); digitalWrite(MOTOR_ESQ_2, LOW);
  digitalWrite(MOTOR_DIR_1, LOW); digitalWrite(MOTOR_DIR_2, LOW);
}

void setup() {
  // Serial para Debug no Monitor Serial (115200 baud)
  Serial.begin(115200);
  Serial.println("--- Inicializando Cupim RCX ---");

  Dabble.begin("Cupim_Dabble"); 

  // Configura todos os pinos como OUTPUT
  pinMode(MOTOR_ESQ_1, OUTPUT);
  pinMode(MOTOR_ESQ_2, OUTPUT);
  pinMode(MOTOR_DIR_1, OUTPUT);
  pinMode(MOTOR_DIR_2, OUTPUT);
  pinMode(ARMA_PIN, OUTPUT);

  // Inicia em estado seguro
  freioAtivoRodas();
  digitalWrite(ARMA_PIN, LOW);
  
  Serial.println("Sistema Pronto. Aguardando Dabble...");
}

void loop() {
  Dabble.processInput(); 

  // Check de conexão para o Serial
  if (Dabble.isAppConnected() && !conectadoAnteriormente) {
      Serial.println(">> APP CONECTADO!");
      conectadoAnteriormente = true;
  } else if (!Dabble.isAppConnected() && conectadoAnteriormente) {
      Serial.println(">> APP DESCONECTADO!");
      conectadoAnteriormente = false;
  }

  // --- 1. LÓGICA DO WATCHDOG ---
  if (GamePad.isUpPressed() || GamePad.isDownPressed() || 
      GamePad.isLeftPressed() || GamePad.isRightPressed() || 
      GamePad.isTrianglePressed() || GamePad.isCirclePressed()) {
      ultimaComunicacao = millis();
  }

  // --- 2. EXECUÇÃO ---
  if (Dabble.isAppConnected() && (millis() - ultimaComunicacao < TIMEOUT_MS)) {
    
    // LOCOMOÇÃO
    if (GamePad.isUpPressed()) {
      Serial.println("Comando: FRENTE");
      digitalWrite(MOTOR_ESQ_1, HIGH); digitalWrite(MOTOR_ESQ_2, LOW);
      digitalWrite(MOTOR_DIR_1, HIGH); digitalWrite(MOTOR_DIR_2, LOW);
    }
    else if (GamePad.isDownPressed()) {
      Serial.println("Comando: RÉ");
      digitalWrite(MOTOR_ESQ_1, LOW); digitalWrite(MOTOR_ESQ_2, HIGH);
      digitalWrite(MOTOR_DIR_1, LOW); digitalWrite(MOTOR_DIR_2, HIGH);
    }
    else if (GamePad.isLeftPressed()) {
      Serial.println("Comando: GIRAR ESQUERDA");
      digitalWrite(MOTOR_ESQ_1, LOW); digitalWrite(MOTOR_ESQ_2, HIGH);
      digitalWrite(MOTOR_DIR_1, HIGH); digitalWrite(MOTOR_DIR_2, LOW);
    }
    else if (GamePad.isRightPressed()) {
      Serial.println("Comando: GIRAR DIREITA");
      digitalWrite(MOTOR_ESQ_1, HIGH); digitalWrite(MOTOR_ESQ_2, LOW);
      digitalWrite(MOTOR_DIR_1, LOW); digitalWrite(MOTOR_DIR_2, HIGH);
    }
    else {
      freioAtivoRodas();
    }

    // ARMA
    if (GamePad.isTrianglePressed()) {
      estadoArma = !estadoArma;
      digitalWrite(ARMA_PIN, estadoArma ? HIGH : LOW);
      Serial.print("ARMA: "); Serial.println(estadoArma ? "LIGADA" : "DESLIGADA");
      delay(250); // Debounce
    }

    if (GamePad.isCirclePressed()) {
      digitalWrite(ARMA_PIN, LOW);
      estadoArma = false;
      Serial.println("ARMA: PÂNICO (OFF)");
      delay(250);
    }

  } else {
    // FAILSAFE: Se o sinal sumir por 300ms, trava tudo
    if (millis() - ultimaComunicacao >= TIMEOUT_MS && conectadoAnteriormente) {
       // Serial.println("ALERTA: Perda de sinal detectada!");
    }
    freioAtivoRodas();
    digitalWrite(ARMA_PIN, LOW);
  }
}