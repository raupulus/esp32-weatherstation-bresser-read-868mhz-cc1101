# ESP32 D1 Mini - Lectura Bresser 6 en 1 RF 868MHz con receptor CC1101

## Descripción

Este proyecto permite leer los datos de una estación meteorológica de 868MHz con un ESP32 D1 Mini y un módulo receptor de radiofrecuencia CC1101.

El modelo utilizado es la estación **Bresser 6-in-1**, pero es compatible con otras estaciones (como la **Bresser 5-in-1**) modificando la directiva correspondiente en [`src/WeatherSensorCfg.h`](src/WeatherSensorCfg.h).

Utilizando el receptor RF CC1101 a 868MHz, se capturan los paquetes emitidos por la estación meteorológica para decodificarlos, procesarlos y subirlos a una API REST propia (API V2) con autenticación Bearer y telemetría de hardware integrada.

Sitio web del autor: [https://raupulus.dev](https://raupulus.dev)

![Imagen del Proyecto](docs/images/hardware/16.jpeg "Imagen del Proyecto Completo")
![Imagen del Proyecto](docs/images/bresser.png "Bresser 6 en 1")

---

## Características

- **Recepción inalámbrica 868MHz**: Decodificación en tiempo real de temperatura, humedad, velocidad del viento (media, mínima y máxima), dirección del viento con conversión a rumbos cardinales (N, NE, SW, etc.), lluvia acumulada, intensidad de lluvia y humedad de suelo (*moisture*).
- **Integración con API V2**: Envío estructurado en lote mediante `POST /api/v2/weather-stations/{DEVICE_ID}/readings`.
- **Telemetría de salud de hardware (`hardware_device_info`)**: Monitoreo en cada envío del estado vital del microcontrolador (temperatura interna del silicio ESP32, porcentaje de uso de RAM, porcentaje de uso de memoria Flash/disco, IP local en la red y tiempo de actividad/uptime).
- **Indicadores LED y efectos visuales**:
  - **LED 1 (GPIO 4)**: Indicador de error / timeout de recepción.
  - **LED 2 (GPIO 2)**: Indicador de lectura y subida correcta a la API.
  - **LED 3 (GPIO 0)**: Parpadeo durante la escucha y muestreo de datos.
  - **Simulación de rayos**: Ráfaga de destellos aleatorios entre LEDs al recibir y procesar datos válidos, adaptado para iluminar la caja con diseño 3D.

---

## Preparar el código

1. Copiar el archivo de configuración de ejemplo:
   ```bash
   cp src/api.cpp.example src/api.cpp
   ```
2. Modificar las credenciales y configuración en `src/api.cpp` (*este archivo está en `.gitignore` para proteger tus datos privados*):
   - `AP_NAME`: Nombre de la red Wi-Fi (SSID).
   - `AP_PASSWORD`: Contraseña de la red Wi-Fi.
   - `upload_to_api`: `true` para activar el envío HTTP o `false` para trabajar en modo local/depuración.
   - `API_DOMAIN`: Dominio o IP del servidor (ej. `https://api.tudominio.dev`).
   - `API_PORT`: Puerto HTTP/HTTPS (por defecto `"443"`).
   - `API_TOKEN_BEARER`: Token de autenticación Sanctum en formato `"Bearer <token>"`.
   - `DEVICE_ID`: ID del dispositivo en la API (ej. `"10"`).
   - `DEBUG`: `true` para ver las trazas completas por puerto serie (115200 baudios).

3. Compilar y flashear al ESP32 con PlatformIO:
   ```bash
   # Compilar
   pio run

   # Compilar y subir por USB
   pio run -t upload

   # Monitor serie
   pio device monitor -b 115200
   ```

Repositorio: [https://gitlab.com/raupulus/esp32-weatherstation-bresser-read-868mhz-cc1101](https://gitlab.com/raupulus/esp32-weatherstation-bresser-read-868mhz-cc1101)

---

## Imágenes del Hardware

<p align="center">
  <img src="docs/images/hardware/16.jpeg" alt="Imagen del Proyecto 1" height="150">
  <img src="docs/images/hardware/17.jpeg" alt="Imagen del Proyecto 2" height="150">
  <img src="docs/images/hardware/18.jpeg" alt="Imagen del Proyecto 3" height="150">
  <img src="docs/images/hardware/19.jpeg" alt="Imagen del Proyecto 4" height="150">
</p>

---

## Diseño 3D Propio

Para complementar el proyecto he creado un diseño personalizado simulando el tiempo (nube, sol y rayo) con la estación detrás a un nivel muy minimalista, logrando una estética visual cuidada para el receptor.

Enlaces de descarga del modelo para impresión 3D:

- [Thingiverse](https://www.thingiverse.com/thing:6800139)
- [Printables](https://www.printables.com/model/1042119-esp32-d1-mini-project-box-bresser-6-in-one-rf-868m)
- [MakerWorld](https://makerworld.com/es/models/708896#profileId-639168)

<p align="center">
  <img src="docs/images/3D/1.jpeg" alt="Diseño 3D 1" height="150">
  <img src="docs/images/3D/2.jpeg" alt="Diseño 3D 2" height="150">
  <img src="docs/images/3D/3.jpeg" alt="Diseño 3D 3" height="150">
  <img src="docs/images/3D/4.png" alt="Diseño 3D 4" height="150">
  <img src="docs/images/3D/5.png" alt="Diseño 3D 5" height="150">
  <img src="docs/images/3D/6.png" alt="Diseño 3D 6" height="150">
  <img src="docs/images/3D/7.png" alt="Diseño 3D 7" height="150">
  <img src="docs/images/3D/8.jpeg" alt="Diseño 3D 8" height="150">
  <img src="docs/images/3D/9.jpeg" alt="Diseño 3D 9" height="150">
  <img src="docs/images/3D/10.jpeg" alt="Diseño 3D 10" height="150">
  <img src="docs/images/3D/11.jpeg" alt="Diseño 3D 11" height="150">
  <img src="docs/images/3D/12.jpeg" alt="Diseño 3D 12" height="150">
  <img src="docs/images/3D/13.jpeg" alt="Diseño 3D 13" height="150">
  <img src="docs/images/3D/14.jpeg" alt="Diseño 3D 14" height="150">
  <img src="docs/images/3D/15.jpeg" alt="Diseño 3D 15" height="150">
</p>

---

## Esquemas de Pines

### Conexión ESP32 D1 Mini a módulo CC1101

| ESP32 D1 Mini | CC1101 | Función |
| ------------- | ------ | ------- |
| 3V3           | VCC    | Alimentación 3.3V |
| GND           | GND    | Masa / Tierra |
| D23           | MOSI   | SPI MOSI |
| D18           | SCK    | SPI Reloj |
| D19           | MISO   | SPI MISO |
| D27           | GDO2   | GPIO CC1101 |
| D25 / D26     | GDO0   | IRQ CC1101 |
| D5            | CSN    | Chip Select (CS) |

---

## Authors and acknowledgment

El proyecto actual ha sido desarrollado por [@raupulus](https://raupulus.dev) / [@fryntiz](https://github.com/fryntiz) (Raúl Caro Pastorino) basándose en la librería de [@matthias-bs](https://github.com/matthias-bs) (Matthias Prinke: [BresserWeatherSensorReceiver](https://github.com/matthias-bs/BresserWeatherSensorReceiver)), tomando como referencia el ejemplo `BresserWeatherSensorCallback`.

---

## License

Este proyecto está bajo la licencia [MIT](LICENSE).

---

## Estado del proyecto y posibles mejoras

El proyecto es totalmente funcional y estable en producción.

Posibles mejoras futuras:
- Externalizar la capa de sensores a un módulo/librería dedicada.
- Delegar la recepción de radiofrecuencia a un núcleo de la CPU del ESP32 (FreeRTOS task) y la gestión Wi-Fi / HTTP al otro núcleo.
- Implementar modo *deep sleep* con despertar por interrupción RF validando checksum e ID de estación para optimizar el consumo de energía.
