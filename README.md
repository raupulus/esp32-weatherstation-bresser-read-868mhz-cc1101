# esp32-weatherstation-read-868mhz-cc1101

## Description

Esté proyecto permite leer los datos de una estación meteorológica de 868Mhz con un ESP32 D1 Mini y un módulo CC1101.

El modelo de la estación es Bresser 6-in-1, pero debería funcionar con cualquier otras estaciones como la de 5-in1 cambiando una definición.

## Getting started

Tenemos que copiar el archivo de ejemplo api.cpp.example al mismo nivel
de directorio y llamarlo simplemente api.cpp modificando posteriormente
las variables privadas de acceso a nuestro router wireless personal y la api
que tengamos en uso.

En caso de no tener o no querer utilizarlo, establecer la variable
upload_to_api en false y las demás variables vacías tal como están en el
archivo de ejemplo actualmente.

## Esquemas de Pines

### ESP32 D1 Mini

| ESP32 D1 Mini | CC1101 |
| ------------- | ------ |
| 3V3           | VCC    |
| GND           | GND    |
| D23           | MOSI   |
| D18           | SCK    |
| D19           | MISO   |
| D27           | GDO2   |
| D26           | GDO0   |
| D5            | CSN    |

## Authors and acknowledgment

El proyecto actual lo ha implementado @raupulus @fryntiz Raúl Caro Pastorino basándose en el proyecto de @matthias-bs Matthias Bresser (https://github.com/matthias-bs/BresserWeatherSensorReceiver)

Usando el ejemplo "BresserWeatherSensorCallback" como base para el desarrollo.

## License

MIT License

## Project status

El proyecto es funcional, pero se puede mejorar y añadir funcionalidades.

Cosas pendientes o muy mejorables:

- Externalizar la parte de los sensores a una librería
- Dejar en un hilo/cpu la parte de la recepción de datos y en otro la parte de la API
- Entrar en modo sleep y despertar con interrupción para ahorrar batería, la interrupción debería ser validando el id de la estación y el checksum.
