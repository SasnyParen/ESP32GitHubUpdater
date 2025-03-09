# ESP32GitHubUpdater

ESP32GitHubUpdater – это библиотека для ESP32, позволяющая обновлять прошивку устройства через интернет с использованием GitHub в качестве источника обновлений. С помощью встроенного веб-сервера библиотека предоставляет удобный HTML-интерфейс для просмотра файлов репозитория и запуска OTA (Over-The-Air) обновлений.

## Особенности

- **Подключение к WiFi:** Автоматическое подключение к заданной WiFi сети с выводом IP-адреса в Serial Monitor.
- **Работа с SPIFFS:** Инициализация файловой системы SPIFFS для хранения файлов и работы веб-сервера.
- **OTA обновление:** Загрузка бинарного файла прошивки с GitHub и обновление устройства без необходимости физического подключения.
- **Веб-интерфейс:** HTML-страница для загрузки списка файлов репозитория и выбора файла для обновления.

## Требования

- ESP32 (Arduino IDE с поддержкой ESP32)
- Библиотеки:
  - [WiFi](https://github.com/espressif/arduino-esp32)
  - [HTTPClient](https://github.com/espressif/arduino-esp32)
  - [Update](https://github.com/espressif/arduino-esp32)
  - [SPIFFS](https://github.com/espressif/arduino-esp32)
  - [WebServer](https://github.com/espressif/arduino-esp32)
  - [ArduinoJson](https://arduinojson.org/)

## Установка

1. Скачайте или склонируйте репозиторий:
   ```bash
   git clone https://github.com/yourusername/ESP32GitHubUpdater.git
2. Скопируйте файлы ESP32GitHubUpdater.h и ESP32GitHubUpdater.cpp в папку libraries/ESP32GitHubUpdater вашего Arduino IDE.
3. Перезапустите Arduino IDE, чтобы библиотека появилась в списке установленных.

## Использование

```cpp
#include "ESP32GitHubUpdater.h"

// Параметры WiFi и GitHub
const char* ssid = "test";
const char* password = "test";
const char* githubUser = "test";
const char* githubRepo = "test";
const char* githubBranch = "test";

// Создаем объект обновления (порт веб-сервера по умолчанию 80)
ESP32GitHubUpdater updater(ssid, password, githubUser, githubRepo, githubBranch);

void setup() {
  // Инициализация библиотеки
  updater.begin();
}

void loop() {
  // Обработка HTTP-запросов
  updater.handleClient();
}
```

## Описание маршрутов веб-сервера

/
Главная страница с HTML-интерфейсом для загрузки списка файлов и запуска обновления.

/git/list
Запрос к GitHub API для получения списка файлов репозитория. Возвращает JSON с информацией о файлах.

/git/update?url=<URL>
Запускает OTA-обновление, загружая бинарный файл по указанному URL. После успешного обновления происходит перезагрузка устройства.

