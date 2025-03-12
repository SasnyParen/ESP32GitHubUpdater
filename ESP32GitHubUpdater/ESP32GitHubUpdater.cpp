#include "ESP32GitHubUpdater.h"

ESP32GitHubUpdater::ESP32GitHubUpdater(
  const char* githubUser, const char* githubRepo, const char* githubBranch, uint16_t port)
  :  _githubUser(githubUser), _githubRepo(githubRepo),
    _githubBranch(githubBranch), _server(port), g_full_length(0), g_curr_length(0)
{}

void ESP32GitHubUpdater::begin() {
  Serial.begin(115200);
  initFlashMemory();

  // Настройка маршрутов веб-сервера
  _server.on("/", [this]() { handleRoot(); });
  _server.on("/git/list", [this]() { handleListRepo(); });
  _server.on("/git/update", [this]() { handleUpdate(); });

  _server.begin();
  Serial.println("HTTP сервер запущен");
}

void ESP32GitHubUpdater::handleClient() {
  _server.handleClient();
}

void ESP32GitHubUpdater::initFlashMemory() {
  Serial.println("Инициализация SPIFFS...");
  if (!SPIFFS.begin(true)) {
    Serial.println("Ошибка инициализации SPIFFS. Форматирование...");
    SPIFFS.format();
    if (!SPIFFS.begin(true)) {
      Serial.println("Ошибка форматирования SPIFFS. Остановка.");
    }
  }
}

void ESP32GitHubUpdater::updateFlash(uint8_t* data, size_t len) {
  Update.write(data, len);
  g_curr_length += len;

  static int count = 0;
  count++;
  if (count % 1024 == 0) Serial.print(".");

  if (g_curr_length != g_full_length) return;

  Update.end(true);
}

void ESP32GitHubUpdater::updateFirmware(HTTPClient &client) {
  uint8_t buff[128]{};
  g_full_length = client.getSize();

  if (g_full_length <= 0) {
    Serial.println("Неверный размер прошивки.");
    return;
  }

  Serial.println("Начало OTA обновления...");
  if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
    Serial.println("Ошибка начала обновления.");
    return;
  }

  WiFiClient* stream = client.getStreamPtr();
  int len = g_full_length;
  while (client.connected() && (len > 0 || len == -1)) {
    size_t size = stream->available();
    if (size) {
      int bytes_read = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
      updateFlash(buff, bytes_read);
      if (len > 0) len -= bytes_read;
    }
    delay(1);
  }
}

void ESP32GitHubUpdater::handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>ESP32 GitHub File Loader</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }
        .container { max-width: 800px; margin: 0 auto; background: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
        h1 { color: #333; text-align: center; }
        .file-list { border: 1px solid #ddd; border-radius: 4px; margin: 20px 0; max-height: 300px; overflow-y: auto; }
        .file-item { padding: 10px; border-bottom: 1px solid #eee; cursor: pointer; }
        .file-item:hover { background-color: #f0f0f0; }
        .file-item.selected { background-color: #e0f7fa; }
        button { background-color: #4CAF50; color: white; border: none; padding: 10px 15px; cursor: pointer; border-radius: 4px; }
        button:disabled { background-color: #ccc; cursor: not-allowed; }
        .status { margin-top: 15px; padding: 10px; }
        .loading { text-align: center; padding: 20px; font-style: italic; color: #666; }
    </style>
</head>
<body>
    <div class="container">
        <h1>ESP32 GitHub File Loader</h1>
        <button onclick="loadRepoContents()">Загрузить файлы репозитория</button>
        <div class="file-list" id="fileList"><div class="loading">Пожалуйста, загрузите содержимое репозитория...</div></div>
        <button id="updateBtn" onclick="updateFirmware()" disabled>Обновить прошивку ESP32</button>
        <div id="status" class="status"></div>
    </div>
    <script>
        let selectedFile = null;
        function loadRepoContents() {
            fetch("/git/list").then(response => response.json()).then(data => {
                let fileList = document.getElementById("fileList");
                fileList.innerHTML = "";
                data.forEach(file => {
                    if (file.type === "file") {
                        let fileItem = document.createElement("div");
                        fileItem.className = "file-item";
                        fileItem.innerText = file.name;
                        fileItem.onclick = function() {
                            document.querySelectorAll(".file-item").forEach(item => item.classList.remove("selected"));
                            this.classList.add("selected");
                            selectedFile = file.download_url;
                            document.getElementById("updateBtn").disabled = false;
                        };
                        fileList.appendChild(fileItem);
                    }
                });
            }).catch(error => console.error("Ошибка при загрузке файлов репозитория:", error));
        }
        function updateFirmware() {
            if (!selectedFile) return;
            document.getElementById("status").innerText = "Обновление...";
            fetch("/git/update?url=" + encodeURIComponent(selectedFile))
                .then(response => response.json())
                .then(data => {
                    document.getElementById("status").innerText = data.success ? "Обновление прошло успешно!" : "Обновление не удалось!";
                    if (data.success) setTimeout(() => { document.getElementById("status").innerText = "Перезапуск ESP32..."; }, 3000);
                });
        }
    </script>
</body>
</html>
  )rawliteral";
  _server.send(200, "text/html", html);
}

void ESP32GitHubUpdater::handleListRepo() {
  String url = "https://api.github.com/repos/" + _githubUser + "/" + _githubRepo + "/contents?ref=" + _githubBranch;
  HTTPClient http;
  http.begin(url);
  http.addHeader("User-Agent", "ESP32");

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    _server.send(200, "application/json", http.getString());
  } else {
    _server.send(500, "application/json", "{\"error\":\"Запрос к GitHub API не выполнен\"}");
  }
  http.end();
}

void ESP32GitHubUpdater::handleUpdate() {
  String url = _server.arg("url");
  if (url.length() == 0) {
    _server.send(400, "application/json", "{\"success\":false,\"message\":\"Отсутствует параметр URL\"}");
    return;
  }

  Serial.println("Начало обновления прошивки из: " + url);
  HTTPClient client;
  client.begin(url);
  client.addHeader("User-Agent", "ESP32");

  int httpcode = client.GET();
  if (httpcode == HTTP_CODE_OK) {
    g_curr_length = 0;
    updateFirmware(client);
  } else {
    Serial.printf("HTTP ошибка: %d\n", httpcode);
  }

  client.end();
  _server.send(200, "application/json", "{\"success\":true}");
  Serial.println("Обновление завершено, перезагрузка...");
  delay(2000);
  ESP.restart();
}
