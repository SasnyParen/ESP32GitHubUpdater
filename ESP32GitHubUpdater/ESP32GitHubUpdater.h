#ifndef ESP32GITHUBUPDATER_H
#define ESP32GITHUBUPDATER_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <ArduinoJson.h>

class ESP32GitHubUpdater {
public:
  /**
   * Конструктор библиотеки.
   * @param ssid Имя WiFi сети.
   * @param password Пароль от WiFi.
   * @param githubUser Имя пользователя GitHub.
   * @param githubRepo Имя репозитория.
   * @param githubBranch Ветка репозитория (по умолчанию "main").
   * @param port Порт для веб-сервера (по умолчанию 80).
   */
  ESP32GitHubUpdater(const char* githubUser, const char* githubRepo, 
                     const char* githubBranch = "main", uint16_t port = 80);

  /**
   * Инициализация библиотеки (подключение к WiFi, запуск SPIFFS, настройка веб-сервера).
   */
  void begin();

  /**
   * Обработка входящих HTTP-запросов.
   * Вызывать в loop().
   */
  void handleClient();

private:
  String _githubUser;
  String _githubRepo;
  String _githubBranch;
  WebServer _server;
  int g_full_length;
  int g_curr_length;

  void initWiFi();
  void initFlashMemory();
  void updateFlash(uint8_t* data, size_t len);
  void updateFirmware(HTTPClient &client);

  void handleRoot();
  void handleListRepo();
  void handleUpdate();
};

#endif
