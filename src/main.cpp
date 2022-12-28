#include <Arduino.h>
#include <HTTPClient.h>
#include "WiFi.h"
#include <string>
#include "WeatherSensorCfg.h"
#include "WeatherSensor.h"
#include "api.cpp"

const int LED_ON = 2; // Pin para indicar que está encendido el circuito.

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
bool rain_ok = false;

float moisture = 0;
bool moisture_ok = false;

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
    // Normally something really important would be done here
    debug(".", false);
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
    }

    delay(2000);
}

bool uploadDataToApi()
{
    wifiConnect();

    if (WiFi.status() == WL_CONNECTED)
    {
        debug("Iniciando subida a la API");

        HTTPClient http;

        // Parámetros a enviar
        // TODO: Convertir a km/h el viento
        String params = "{\"hardware_device_id\":" + (String)DEVICE_ID +
                        ",\"temperature\":" + (String)temp +
                        ",\"humidity\":" + (String)humidity +
                        ",\"wind_speed\":" + (String)wind_avg +
                        ",\"wind_average_speed\":" + (String)wind_avg +
                        ",\"wind_min_speed\":" + (String)wind_min +
                        ",\"wind_max_speed\":" + (String)wind_max +
                        ",\"wind_grades\":" + (String)wind_dir +
                        ",\"rain\":" + (String)rain +
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

void setup()
{

    if (DEBUG)
    {
        Serial.begin(115200);
    }

    weatherSensor.begin();

    // Conectando al wifi
    debug("Antes de primera conexión wireless");
    wifiConnect();
    debug("Después de primera conexión wireless");
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
        rain = sensor.rain_mm;
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
    else
    {
        debug("Sensor timeout\n");
    }

    ///////////////////////////////

    if (!decode_ok)
    {
        debug("Sensor timeout\n");
    }

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        bool isValid = checkReadsSensor(i);

        if (checkAllSensors() && upload_to_api)
        {

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
                Serial.printf("Moisture: %4.1f", moisture);
                Serial.println("");

                Serial.println("Subiendo datos a la API");
            }

            // Si intenta subir datos a la api, reinicia lecturas.
            uploadDataToApi() ? resetAllReads() : debug("Error al subir datos a la API");
        }
    }
    delay(100);
} // loop()
