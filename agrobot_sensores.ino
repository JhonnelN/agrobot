#include <SoftwareSerial.h>
// DHT
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11 // o DHT22
DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial BT(10, 11); // Bluetooth: TX, RX

const int PIN_SENSOR_LLUVIA = A0;
const int powerPin = 7;
const int PIN_SENSOR_HUMEDAD_SUELO = A1;
const int PIN_LUZ = A5;


const float umbralLluvia = 20.0;
const float umbralTempCambio = 0.2;     // más sensible
const float umbralHumedadCambio = 0.5;  // más sensible

bool lluviaAnterior = false;
float tempAnterior = -1000;
float humedadAnterior = -1000;
float humedad_suelo_anterior = -1000;
bool primerEnvio = true;

void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  dht.begin();
  pinMode(powerPin, OUTPUT);
}

void loop() {
  // Leer lluvia
  digitalWrite(powerPin, HIGH);
  delay(100);
  int raw = analogRead(PIN_SENSOR_LLUVIA);
  digitalWrite(powerPin, LOW);

  float profundidad = raw * (5.0 / 1023.0) * 100;
  bool hayAgua = (profundidad >= umbralLluvia);

  // Leer DHT
  float temp = dht.readTemperature();
  float humedad = dht.readHumidity();
    if (isnan(temp) || isnan(humedad)) {
    Serial.println("Error leyendo DHT");
    return;
  }
  // sensor humedad suelo
  int rawValue = analogRead(PIN_SENSOR_HUMEDAD_SUELO); // Rango típico: 0-1023
  int humedad_suelo = map(rawValue, 0, 1023, 0, 100); // Conversión a porcentaje
  // sensor Luz
  int valor_luz = analogRead(PIN_LUZ); // Lectura del sensor (0-1023)
  int porcentaje_luz = map(valor_luz, 0, 1023, 0, 100); // Conversión a porcentaje

  // Verificar cambios
  bool cambioLluvia = (hayAgua != lluviaAnterior);
  bool cambioTemp = abs(temp - tempAnterior) >= umbralTempCambio;
  bool cambioHumedad = abs(humedad - humedadAnterior) >= umbralHumedadCambio;
  bool cambio_humedad_suelo = abs(humedad_suelo - humedad_suelo_anterior) != 0;

  if (primerEnvio || cambioLluvia || cambioTemp || cambioHumedad || cambio_humedad_suelo ) {
    primerEnvio = false;
    lluviaAnterior = hayAgua;
    tempAnterior = temp;
    humedadAnterior = humedad;
    humedad_suelo_anterior = humedad_suelo;

    String estadoLluvia = hayAgua ? "1" : "0";

    String mensaje = "Estado: " + estadoLluvia;
    mensaje += " | Temp: " + String(temp, 1) + "°C";
    mensaje += " | Humedad: " + String(humedad, 1) + "%";


    Serial.print("Valor analógico: ");
    Serial.print(rawValue);
    Serial.print(" | Humedad: ");
    Serial.print(humedad_suelo);
    Serial.println("%");
    Serial.println(estadoLluvia + "," + humedad_suelo + "," + humedad + "," + temp+","+ valor_luz);
    BT.println(estadoLluvia + "," + humedad_suelo + "," + humedad + "," + temp +","+ valor_luz);
  }

  delay(2000);
}
