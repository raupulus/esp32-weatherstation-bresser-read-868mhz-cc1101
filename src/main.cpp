#include <Arduino.h>
#include <HTTPClient.h>
#include "WiFi.h"
#include <string>
#include "WeatherSensorCfg.h"
#include "WeatherSensor.h"
#include "api.cpp"

#include <esp_timer.h>

const int LED_ON = 2; // Pin para indicar que está encendido el circuito.

// Defino los pines para los LEDs secundarios de información
#define LED1_PIN 4 // Indica un error
#define LED2_PIN 2 // Indica una lectura correcta
#define LED3_PIN 0 // Indica que está leyendo datos (Led verde parpadeando)

float temp;
bool temp_ok = false;

float humidity = 0;
bool humidity_ok = false;

float wind_current = 0;
float wind_avg = 0;
float wind_min = 0;
float wind_max = 0;
float wind_dir = 0;
bool wind_ok = false;

float rain = 0;
float rain_intensity = 0;
float rain_month = 0;
bool rain_ok = false;
uint64_t rain_timestamp = esp_timer_get_time(); // Fecha de la última lectura de lluvia, para calcular la intensidad

float moisture = 0;
bool moisture_ok = false;

// Indica si es la primera lectura tras iniciar
bool first_read = true;

WeatherSensor weatherSensor;

void debug(String message, bool newLine = true)
{
    if (DEBUG)
    {
        if (newLine)
        {
            Serial.println(message);
        }
        else
        {
            Serial.print(message);
        }
    }
}

// Loop en cada iteración buscando datos
void loopCallback(void)
{
    digitalWrite(LED3_PIN, HIGH);
    // Normally something really important would be done here
    debug(".", false);
    digitalWrite(LED3_PIN, LOW);
}

/*
 * Realiza la conexión al wifi en caso de no estar conectado.
 */
void wifiConnect()
{
    if (upload_to_api && (WiFi.status() != WL_CONNECTED))
    {
        debug("Conectando al WiFi..");

        WiFi.begin(AP_NAME, AP_PASSWORD);

        delay(500);

        if (WiFi.status() == WL_CONNECTED)
        {
            debug("Se ha conectado al wifi correctamente..");
        }
        else
        {
            debug("No se ha podido conectar al wifi..");
            debug("Estado: " + (String)WiFi.status());
            debug("SSID: " + (String)WiFi.SSID());
            debug("IP: " + (String)WiFi.localIP());
            debug("MAC: " + (String)WiFi.macAddress());
            debug("RSSI: " + (String)WiFi.RSSI());
            debug("BSSID: " + (String)WiFi.BSSIDstr());
            debug("Canal: " + (String)WiFi.channel());
            debug("Gateway: " + (String)WiFi.gatewayIP());
            debug("Máscara: " + (String)WiFi.subnetMask());
            debug("DNS: " + (String)WiFi.dnsIP());
            debug("Puerta de enlace: " + (String)WiFi.gatewayIP());
        }
    }

    delay(2000);
}

bool uploadDataToApi()
{
    if (first_read)
    {
        return true;
    }

    wifiConnect();

    if (WiFi.status() == WL_CONNECTED)
    {
        debug("Iniciando subida a la API");

        HTTPClient http;

        // Parámetros a enviar
        // TODO: Preparar diff en precipitaciones
        String params = "{\"hardware_device_id\":" + (String)DEVICE_ID +
                        ",\"temperature\":" + (String)temp +
                        ",\"humidity\":" + (String)humidity +
                        ",\"wind_speed\":" + (String)wind_avg +
                        ",\"wind_average_speed\":" + (String)wind_avg +
                        ",\"wind_min_speed\":" + (String)wind_min +
                        ",\"wind_max_speed\":" + (String)wind_max +
                        ",\"wind_grades\":" + (String)wind_dir +
                        ",\"rain\":" + (String)rain +
                        ",\"rain_intensity\":" + (String)rain_intensity +
                        ",\"rain_month\":" + (String)rain_month +
                        ",\"moisture\":" + (String)moisture +
                        "}";

        debug("Parámetros json: ");
        debug(params);

        // http.begin("https://api.fryntiz.dev/smartplant/register/add-json");
        http.begin((String)API_DOMAIN + ":" + (String)API_PORT + "/" + (String)API_PATH);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", API_TOKEN_BEARER);
        http.addHeader("Accept", "*/*");

        // Realiza la subida a la API
        int httpCode = http.POST(params);

        // Respuesta de la API
        auto response = http.getString();

        debug("Stream:");

        if (DEBUG)
        {
            // Serial.println(http.getStream());

            debug("Response:");
            debug(response);

            debug("Código de respuesta de la API: ");
            debug((String)httpCode);
        }

        debug("Ruta de la api: ");
        debug((String)API_DOMAIN + ":" + (String)API_PORT + "/" + (String)API_PATH);

        // Indica que ha terminado de transmitirse el post.
        http.end();
    }
    else
    {
        debug("No se ha conectado al WIFI, no se inicia la subida a la API");

        return false;
    }

    return true;
}

void simulateLightning()
{
    int flashes = random(5, 11); // Número de flashes en una ráfaga de rayos
    for (int i = 0; i < flashes; i++)
    {
        int led = random(1, 3); // Selecciono aleatoriamente uno de los dos LEDs
        if (led == 1)
        {
            digitalWrite(LED1_PIN, HIGH);
        }
        else
        {
            digitalWrite(LED2_PIN, HIGH);
        }

        delay(random(50, 200)); // Duración del flash entre 50 y 200 ms

        // Apagar el LED
        if (led == 1)
        {
            digitalWrite(LED1_PIN, LOW);
        }
        else
        {
            digitalWrite(LED2_PIN, LOW);
        }

        delay(random(100, 500)); // Tiempo entre flashes de 100 a 500 ms
    }
}

void setup()
{

    if (DEBUG)
    {
        Serial.begin(115200);
        printf("Serial iniciado");
    }

    // Configuro los pines de los LEDs como salidas
    pinMode(LED1_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);
    pinMode(LED3_PIN, OUTPUT);

    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, HIGH);

    weatherSensor.begin();

    // Conectando al wifi
    debug("Antes de primera conexión wireless");
    wifiConnect();
    debug("Después de primera conexión wireless");

    delay(1000);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);

    delay(1000);

    simulateLightning();
}

/*
void callbackOnCorrectReading()
{
    Serial.println("Correct reading");
    debug(weatherSensor.sensor)
}
*/

/**
 * Comprueba si todos los sensores han sido leídos correctamente.
 */
bool checkAllSensors()
{
    bool allSensorsOk = temp_ok && humidity_ok && wind_ok && rain_ok;

    // bool allSensorsOk = temp_ok && humidity_ok && wind_ok && rain_ok && moisture_ok;

    return allSensorsOk;
}

/**
 * Resetea todos los sensores a false.
 */
void resetAllReads()
{
    temp = 0.0;
    humidity = 0.0;
    wind_current = 0.0;
    wind_avg = 0.0;
    wind_max = 0.0;
    wind_min = 0.0;
    wind_dir = 0.0;
    rain = 0.0;
    rain_intensity = 0;
    // rain_month = 0; // Para calcular diferencia mm, no puedo resetearlo
    moisture = 0.0;

    temp_ok = false;
    humidity_ok = false;
    wind_ok = false;
    rain_ok = false;
    moisture_ok = false;
}

/**
 * Comprueba si el sensor ha sido leído correctamente.
 * También actualiza los valores de los sensores.
 */
bool checkReadsSensor(int sensorId)
{
    WeatherSensor::sensor_t sensor = weatherSensor.sensor[sensorId];

    // Si el sensor no es válido
    if (!sensor.valid)
    {
        debug("Sensor no válido: " + (String)sensorId);
        return false;
    }

    // Si el sensor es válido
    if (DEBUG)
    {
        Serial.printf("Id: [%8X] Typ: [%X] Battery: [%s] ",
                      sensor.sensor_id,
                      sensor.s_type,
                      sensor.battery_ok ? "OK " : "Low");

#ifdef BRESSER_6_IN_1
        Serial.printf("Ch: [%d] ", sensor.chan);
#endif
    }

    if (sensor.temp_ok)
    {
        temp = sensor.temp_c;
        temp_ok = true;
    }

    if (sensor.humidity_ok)
    {
        humidity = sensor.humidity;
        humidity_ok = true;
    }

    if (sensor.wind_ok)
    {
        // wind_current = weatherSensor.sensor[i].wind_meter_sec;
        wind_avg = sensor.wind_avg_meter_sec;
        wind_max = sensor.wind_gust_meter_sec;
        wind_min = wind_min == 0.0 || (wind_min > sensor.wind_avg_meter_sec) ? sensor.wind_avg_meter_sec : wind_min;
        wind_dir = sensor.wind_direction_deg;
        wind_ok = true;
    }

    if (sensor.rain_ok)
    {
        // Cuando es la primera lectura, almaceno el acumulado para después obtener la diferencia
        if (first_read)
        {
            rain_month = sensor.rain_mm;
            rain_timestamp = esp_timer_get_time();

            first_read = false;
        }
        else
        {
            // Diferencia de lluvia entre lecturas
            float rain_diff = sensor.rain_mm - rain_month;

            rain = rain_diff > 0 ? rain_diff : 0;

            // Acumulado de lluvia durante el último mes
            rain_month = sensor.rain_mm;

            if (rain > 0)
            {
                // Intensidad de lluvia
                uint64_t now = esp_timer_get_time();
                uint64_t diff = now - rain_timestamp;
                rain_timestamp = now;

                int diff_in_seconds = diff / 1000000;

                rain_intensity = (rain / diff_in_seconds) * 3600;
            }
            else
            {
                rain_timestamp = esp_timer_get_time();
                rain_intensity = 0;
            }
        }

        rain_ok = true;
    }

    if (sensor.moisture_ok)
    {
        moisture = sensor.moisture;
        moisture_ok = true;
    }

    if (DEBUG)
    {
        Serial.printf("RSSI: [%5.1fdBm]\n", sensor.rssi);
    }

    return true;
}

void loop()
{
    // Clear all sensor data
    weatherSensor.clearSlots();

    // Attempt to receive entire data set with timeout of <xxx> s and callback function
    bool decode_ok = weatherSensor.getData(60000, DATA_COMPLETE, 0, &loopCallback);

    ///////////////////////////////

    if (decode_ok)
    {
        debug("Sensor data received\n");
    }

    if (!decode_ok)
    {
        digitalWrite(LED1_PIN, HIGH);
        debug("Sensor timeout\n");
        delay(100);
        digitalWrite(LED1_PIN, LOW);
    }

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        bool isValid = checkReadsSensor(i);

        if (checkAllSensors() && upload_to_api)
        {

            digitalWrite(LED2_PIN, HIGH);

            if (DEBUG)
            {
                Serial.println("Termina una lectura completa\n");
                Serial.println("");
                Serial.printf("Temp: %4.1f", temp);
                Serial.println("");
                Serial.printf("Humidity: %4.1f", humidity);
                Serial.println("");
                Serial.printf("Wind current: %4.1f", wind_current);
                Serial.println("");
                Serial.printf("Wind avg: %4.1f", wind_avg);
                Serial.println("");
                Serial.printf("Wind min: %4.1f", wind_min);
                Serial.println("");
                Serial.printf("Wind max: %4.1f", wind_max);
                Serial.println("");
                Serial.printf("Wind dir: %4.1f", wind_dir);
                Serial.println("");
                Serial.printf("Rain: %4.1f", rain);
                Serial.println("");
                Serial.printf("Rain Intensity: %4.1f", rain_intensity);
                Serial.println("");
                Serial.printf("Rain Month: %4.1f", rain_month);
                Serial.println("");
                Serial.printf("Moisture: %4.1f", moisture);
                Serial.println("");

                Serial.println("Subiendo datos a la API");
            }

            // Si intenta subir datos a la api, reinicia lecturas.
            uploadDataToApi() ? resetAllReads() : debug("Error al subir datos a la API");

            digitalWrite(LED2_PIN, LOW);

            simulateLightning();
        }
    }
    delay(100);
} // loop()
