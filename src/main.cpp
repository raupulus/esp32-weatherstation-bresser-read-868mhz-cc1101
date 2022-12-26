#include <Arduino.h>
#include "WeatherSensorCfg.h"
#include "WeatherSensor.h"

WeatherSensor weatherSensor;

// Example for callback function which is executed while waiting for radio messages
void loopCallback(void)
{
    // Normally something really important would be done here
    Serial.print(".");
}

void setup()
{
    Serial.begin(115200);

    weatherSensor.begin();
}

void loop()
{
    // Clear all sensor data
    weatherSensor.clearSlots();

    // Attempt to receive entire data set with timeout of <xxx> s and callback function
    bool decode_ok = weatherSensor.getData(60000, DATA_COMPLETE, 0, &loopCallback);
    Serial.println();

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
                Serial.printf("Temp: [%5.1fC] ",
                              weatherSensor.sensor[i].temp_c);
            }
            else
            {
                Serial.printf("Temp: [---.-C] ");
            }
            if (weatherSensor.sensor[i].humidity_ok)
            {
                Serial.printf("Hum: [%3d%%] ",
                              weatherSensor.sensor[i].humidity);
            }
            else
            {
                Serial.printf("Hum: [---%%] ");
            }
            if (weatherSensor.sensor[i].wind_ok)
            {
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
                Serial.printf("Rain: [%7.1fmm] ",
                              weatherSensor.sensor[i].rain_mm);
            }
            else
            {
                Serial.printf("Rain: [-----.-mm] ");
            }
            if (weatherSensor.sensor[i].moisture_ok)
            {
                Serial.printf("Moisture: [%2d%%] ",
                              weatherSensor.sensor[i].moisture);
            }
            else
            {
                Serial.printf("Moisture: [--%%] ");
            }
            Serial.printf("RSSI: [%5.1fdBm]\n", weatherSensor.sensor[i].rssi);
        }
    }
    delay(100);
} // loop()
