#include <SoftwareSerial.h>
#include <DHT.h>

// =============== CONSTANTES Y CONFIGURACIÓN ===============
#define DHTPIN 2
#define DHTTYPE DHT11
#define BLUETOOTH_RX 10
#define BLUETOOTH_TX 11
#define PIN_SENSOR_LLUVIA A0
#define PIN_HUMEDAD_SUELO A2
#define PIN_POWER_SENSOR 7
#define PIN_LUMINISCENCIA A1

// Umbrales de sensores
const float UMBRAL_LLUVIA = 20.0f;
const float UMBRAL_CAMBIO_TEMP = 0.2f;
const float UMBRAL_CAMBIO_HUMEDAD = 0.5f;
const unsigned long INTERVALO_LECTURAS = 2000;

// Objetos de hardware
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial bluetooth(BLUETOOTH_RX, BLUETOOTH_TX);

// =============== VARIABLES GLOBALES ===============
struct SensorData {
    bool lluvia;
    float humedad_suelo;
    float humedad_ambiente;
    float temperatura;
    int luminiscencia;
};

SensorData datos_actuales;
SensorData datos_anteriores = {false, -1.0f, -1.0f, -1.0f, -1};
bool primer_envio = true;

// =============== PROTOTIPOS DE FUNCIONES ===============
SensorData leer_sensores();
bool hubo_cambios_significativos(const SensorData& actual, const SensorData& anterior);
void enviar_datos_bluetooth(const SensorData& datos);

// =============== SETUP ===============
void setup() {
    Serial.begin(9600);

    bluetooth.begin(9600);
    dht.begin();
    
    pinMode(PIN_POWER_SENSOR, OUTPUT);
    pinMode(PIN_LUMINISCENCIA, INPUT);
    
    Serial.println("Sistema de monitoreo ambiental iniciado");
}

// =============== LOOP PRINCIPAL ===============
void loop() {
    static unsigned long ultimo_intervalo = 0;
    
    if (millis() - ultimo_intervalo >= INTERVALO_LECTURAS) {
        ultimo_intervalo = millis();
        
        datos_actuales = leer_sensores();
        
        if (primer_envio || hubo_cambios_significativos(datos_actuales, datos_anteriores)) {
            enviar_datos_bluetooth(datos_actuales);
            datos_anteriores = datos_actuales;
            primer_envio = false;
        }
    }
}

// =============== IMPLEMENTACIÓN DE FUNCIONES ===============
SensorData leer_sensores() {
    SensorData datos;
    
    // Lectura sensor de lluvia/humedad suelo
    digitalWrite(PIN_POWER_SENSOR, HIGH);
    delay(50);  // Estabilización del sensor
    datos.humedad_suelo = analogRead(PIN_HUMEDAD_SUELO) * (100.0f / 1023.0f);
    digitalWrite(PIN_POWER_SENSOR, LOW);
    datos.lluvia = (datos.humedad_suelo >= UMBRAL_LLUVIA);

    // Lectura DHT (ambiente)
    datos.humedad_ambiente = dht.readHumidity();
    datos.temperatura = dht.readTemperature();

    // Lectura luminiscencia (LDR)
    datos.luminiscencia = analogRead(PIN_LUMINISCENCIA);

    // Validación de lecturas
    if (isnan(datos.humedad_ambiente) || isnan(datos.temperatura)) {
        Serial.println("Error en lectura DHT!");
        datos.humedad_ambiente = -1.0f;
        datos.temperatura = -1.0f;
    }

    return datos;
}

bool hubo_cambios_significativos(const SensorData& actual, const SensorData& anterior) {
    return (actual.lluvia != anterior.lluvia) ||
           (fabs(actual.humedad_suelo - anterior.humedad_suelo) >= UMBRAL_CAMBIO_HUMEDAD) ||
           (fabs(actual.temperatura - anterior.temperatura) >= UMBRAL_CAMBIO_TEMP) ||
           (abs(actual.luminiscencia - anterior.luminiscencia) >= 50);  // Umbral luminiscencia
}

void enviar_datos_bluetooth(const SensorData& datos) {
    // Formato: bool,float,float,float,int
    String mensaje = 
        String(datos.lluvia) + "," +
        String(datos.humedad_suelo, 1) + "," +
        String(datos.humedad_ambiente, 1) + "," +
        String(datos.temperatura, 1) + "," +
        String(datos.luminiscencia);

    bluetooth.println(mensaje);
    
    // Debug por serial
    Serial.print("Enviado: ");
    Serial.println(mensaje);
}