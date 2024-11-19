#define TRIG_PIN 2 
#define ECHO_PIN 3  
#define MAX_DIST 30 
#define MAX_VEL 255 

int motorPin1 = 5;          
int motorPin2 = 6;          

int distanciaRecibida = 0;  

float velocidadAjustada = 0;

int velocidadMotor1 = 0;
int velocidadMotor2 = 0;

bool configurado = false;

float errorAcumulado = 0;
float tiempoAnterior = 0;
float Kp = 10;
float Ki = 2;

void setup() {
  Serial.begin(9600);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {

  if (!configurado) {
    Serial.read();
    distanciaRecibida = -1;
    while (distanciaRecibida <= 0 || distanciaRecibida > MAX_DIST) {
      Serial.println("Ingrese la distancia objetivo (valor referencia): ");
      while (!Serial.available());
      String input = Serial.readStringUntil('\n');
      distanciaRecibida = input.toInt();
      if (distanciaRecibida <= 0 || distanciaRecibida > MAX_DIST) {
        Serial.println("Error: valor de distancia no válido.");
      }
    }

    configurado = true;
  } else {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);

    float distanciaActual = duration * 0.034 / 2;

    float error = distanciaRecibida - distanciaActual;

    unsigned long tiempoActual = millis();
    float deltaTiempo = (tiempoActual - tiempoAnterior) / 1000.0; // Convertir a segundos
    tiempoAnterior = tiempoActual;

    errorAcumulado += abs(error) * deltaTiempo;

    velocidadAjustada = Kp * abs(error) + Ki * errorAcumulado;

    if (velocidadAjustada > MAX_VEL) {
      velocidadAjustada = MAX_VEL;
    } else if (velocidadAjustada < 150) {
      velocidadAjustada = 150;
    }
    
    float tolerancia = 0.3;

    if (error > 5) {
      velocidadMotor1 = MAX_VEL;
      velocidadMotor2 = 0;
    } else if (error < -5) {
      velocidadMotor1 = 0;
      velocidadMotor2 = MAX_VEL;
    }
    
    if (error > 0 && error <= 5) {
      velocidadMotor1 = velocidadAjustada;
      velocidadMotor2 = 0;
    } else if (error <= 0 && error >= -5) {
      velocidadMotor1 = 0;
      velocidadMotor2 = velocidadAjustada;
    }
    
    if (abs(error) <= tolerancia) {
      velocidadMotor1 = 0;
      velocidadMotor2 = 0;
      errorAcumulado = 0;
      tiempoAnterior = 0;
      configurado = false;
    } 

    analogWrite(motorPin1, velocidadMotor1);
    analogWrite(motorPin2, velocidadMotor2);

    Serial.print(distanciaActual);
    Serial.print("\t");

    Serial.print(velocidadAjustada);
    Serial.print("\t");

    Serial.println(abs(error));

    if(!configurado){
      Serial.println("Aproximacion finalizada.");
      loop();
    }
    
  }

}