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

int humidity = 0;
bool humidity_ok = false;

int wind_current = 0;
float wind_avg = 0;
float wind_max = 0;
float wind_dir = 0;
bool wind_ok = false;

int rain = 0;
bool rain_ok = false;

int moisture = 0;
bool moisture_ok = false;

WeatherSensor weatherSensor;

void debug(String message)
{
    if (DEBUG)
    {
        Serial.println(message);
    }
}

// Example for callback function which is executed while waiting for radio messages
void loopCallback(void)
{
    // Normally something really important would be done here
    Serial.print(".");
}

/*
 * Realiza la conexión al wifi en caso de no estar conectado.
 */
void wifiConnect()
{
    /*
    if (DEBUG)
    {
        return;
    }
    */

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

void uploadDataToApi()
{
    wifiConnect();

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Iniciando subida a la API");
        HTTPClient http;

        // Parámetros a enviar
        // TODO: Convertir a km/h el viento
        String params = "{\"hardware_device_id\":" + (String)DEVICE_ID +
                        ",\"temperature\":" + (String)temp +
                        ",\"humidity\":" + (String)humidity +
                        ",\"wind_speed\":" + (String)wind_avg +
                        ",\"wind_average_speed\":" + (String)wind_avg +
                        ",\"wind_min_speed\":" + (String)wind_avg +
                        ",\"wind_max_speed\":" + (String)wind_max +
                        ",\"wind_grade\":" + (String)wind_dir +
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
            Serial.println(http.getStream());

            debug("Response:");
            Serial.println(response);

            debug("Código de respuesta de la API: ");
            Serial.println(httpCode);
        }

        debug("Ruta de la api: ");
        debug((String)API_DOMAIN + ":" + (String)API_PORT + "/" + (String)API_PATH);

        // Indica que ha terminado de transmitirse el post.
        http.end();
    }
    else
    {
        debug("No se ha conectado al WIFI, no se inicia la subida a la API");
    }

    delay(300);
}

void setup()
{
    Serial.begin(115200);

    // TODO: Conexión al wireless

    weatherSensor.begin();

    // Conectando al wifi
    Serial.println("Antes de primera conexión wireless");
    wifiConnect();
    Serial.println("Después de primera conexión wireless");
}

/*
void callbackOnCorrectReading()
{
    Serial.println("Correct reading");
    debug(weatherSensor.sensor)
}
*/

bool checkAllSensors()
{
    bool allSensorsOk = temp_ok && humidity_ok && wind_ok && rain_ok;

    // bool allSensorsOk = temp_ok && humidity_ok && wind_ok && rain_ok && moisture_ok;

    return allSensorsOk;
}

void loop()
{
    // Clear all sensor data
    weatherSensor.clearSlots();

    // Attempt to receive entire data set with timeout of <xxx> s and callback function
    bool decode_ok = weatherSensor.getData(60000, DATA_COMPLETE, 0, &loopCallback);
    Serial.println();

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
        Serial.printf("Sensor timeout\n");
    }

    for (int i = 0; i < NUM_SENSORS; i++)
    {
        if (weatherSensor.sensor[i].valid)
        {
            Serial.printf("Id: [%8X] Typ: [%X] Battery: [%s] ",
                          weatherSensor.sensor[i].sensor_id,
                          weatherSensor.sensor[i].s_type,
                          weatherSensor.sensor[i].battery_ok ? "OK " : "Low");
#ifdef BRESSER_6_IN_1
            Serial.printf("Ch: [%d] ", weatherSensor.sensor[i].chan);
#endif
            if (weatherSensor.sensor[i].temp_ok)
            {
                temp = weatherSensor.sensor[i].temp_c;
                temp_ok = true;

                Serial.printf("Temp: [%5.1fC] ",
                              weatherSensor.sensor[i].temp_c);
            }
            else
            {
                Serial.printf("Temp: [---.-C] ");
            }
            if (weatherSensor.sensor[i].humidity_ok)
            {
                humidity = weatherSensor.sensor[i].humidity;
                humidity_ok = true;

                Serial.printf("Hum: [%3d%%] ",
                              weatherSensor.sensor[i].humidity);
            }
            else
            {
                Serial.printf("Hum: [---%%] ");
            }
            if (weatherSensor.sensor[i].wind_ok)
            {
                // wind_current = weatherSensor.sensor[i].wind_meter_sec;
                wind_avg = weatherSensor.sensor[i].wind_avg_meter_sec;
                wind_max = weatherSensor.sensor[i].wind_gust_meter_sec;
                wind_dir = weatherSensor.sensor[i].wind_direction_deg;
                wind_ok = true;

                Serial.printf("Wind max: [%4.1fm/s] Wind avg: [%4.1fm/s] Wind dir: [%5.1fdeg] ",
                              weatherSensor.sensor[i].wind_gust_meter_sec,
                              weatherSensor.sensor[i].wind_avg_meter_sec,
                              weatherSensor.sensor[i].wind_direction_deg);
            }
            else
            {
                Serial.printf("Wind max: [--.-m/s] Wind avg: [--.-m/s] Wind dir: [---.-deg] ");
            }
            if (weatherSensor.sensor[i].rain_ok)
            {
                rain = weatherSensor.sensor[i].rain_mm;
                rain_ok = true;

                Serial.printf("Rain: [%7.1fmm] ",
                              weatherSensor.sensor[i].rain_mm);
            }
            else
            {
                Serial.printf("Rain: [-----.-mm] ");
            }
            if (weatherSensor.sensor[i].moisture_ok)
            {
                moisture = weatherSensor.sensor[i].moisture;
                moisture_ok = true;

                Serial.printf("Moisture: [%2d%%] ",
                              weatherSensor.sensor[i].moisture);
            }
            else
            {
                Serial.printf("Moisture: [--%%] ");
            }
            Serial.printf("RSSI: [%5.1fdBm]\n", weatherSensor.sensor[i].rssi);

            // TODO Limpiar datos anteriores
            // TODO Guardar datos de forma que se mantengan lecturas
            // previniendo lecturas nulas
            // TODO: Subir a la api aprox cada 1 minuto

            // Compruebo si está conectado a la red Wireless

            // TODO Check si todas las variables tienen valor

            if (checkAllSensors && upload_to_api)
            {
                wifiConnect();
                delay(1000);
                Serial.println("Compruebo conexión wireless");
                if (WiFi.status() == WL_CONNECTED)
                {
                    Serial.println("Conectado a la red wireless");
                    Serial.println("Subiendo datos a la API");
                }
                else
                {
                    Serial.println("No conectado a la red wireless");
                }

                delay(300);

                Serial.println("Termina una lectura completa\n");
                Serial.println("");
                Serial.printf("Temp: %5.1f", temp);
                Serial.println("");
                Serial.printf("Humidity: %3d", humidity);
                Serial.println("");
                Serial.printf("Wind current: %4.1f", wind_current);
                Serial.println("");
                Serial.printf("Wind avg: %4.1f", wind_avg);
                Serial.println("");
                Serial.printf("Wind max: %4.1f", wind_max);
                Serial.println("");
                Serial.printf("Wind dir: %5.1f", wind_dir);
                Serial.println("");
                Serial.printf("Rain: %7.1f", rain);
                Serial.println("");
                Serial.printf("Moisture: %2d", moisture);
                Serial.println("");

                Serial.println("Subiendo datos a la API");

                uploadDataToApi();

                // Reset variables
                temp,
                    humidity, wind_current, wind_avg, wind_max, wind_dir, rain, moisture = 0;
                temp_ok, humidity_ok, wind_ok, rain_ok = false;

                // TODO: Subir a la api
            }
            else
            {
                Serial.println("Falta algún dato");
            }
        }
    }
    delay(100);
} // loop()
