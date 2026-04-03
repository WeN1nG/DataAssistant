#include "WeatherManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>

WeatherManager::WeatherManager(QObject *parent) :
    QObject(parent),
    networkManager(new QNetworkAccessManager(this)),
    updateTimer(new QTimer(this)),
    enabled(false)
{
    connect(networkManager, &QNetworkAccessManager::finished, this, &WeatherManager::onNetworkReply);
    connect(updateTimer, &QTimer::timeout, this, &onTimerTimeout);
    
    loadSettings();
    
    // 每 30 分钟更新一次天气
    updateTimer->start(30 * 60 * 1000);
    if (enabled && !apiKey.isEmpty() && !city.isEmpty()) {
        fetchWeather();
    }
}

WeatherManager::~WeatherManager()
{
    delete networkManager;
    delete updateTimer;
}

void WeatherManager::setApiKey(const QString &apiKey) {
    this->apiKey = apiKey;
    saveSettings();
}

void WeatherManager::setCity(const QString &city) {
    this->city = city;
    saveSettings();
}

void WeatherManager::setEnabled(bool enabled) {
    this->enabled = enabled;
    saveSettings();
    if (enabled && !apiKey.isEmpty() && !city.isEmpty()) {
        fetchWeather();
    }
}

bool WeatherManager::isEnabled() const {
    return enabled;
}

void WeatherManager::fetchWeather() {
    if (!enabled || apiKey.isEmpty() || city.isEmpty()) {
        return;
    }
    
    // 高德天气 API 请求
    QString url = QString("https://restapi.amap.com/v3/weather/weatherInfo?key=%1&city=%2&extensions=base")
        .arg(apiKey).arg(city);
    
    networkManager->get(QNetworkRequest(QUrl(url)));
}

WeatherInfo WeatherManager::getWeatherInfo() const {
    return weatherInfo;
}

void WeatherManager::onNetworkReply(QNetworkReply *reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        reply->deleteLater();
        return;
    }
    
    QByteArray response = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
    QJsonObject jsonObj = jsonDoc.object();
    
    if (jsonObj["status"].toString() == "1") {
        QJsonArray forecasts = jsonObj["lives"].toArray();
        if (!forecasts.isEmpty()) {
            QJsonObject forecast = forecasts[0].toObject();
            weatherInfo.city = forecast["city"].toString();
            weatherInfo.temperature = forecast["temperature"].toString() + "°C";
            weatherInfo.condition = forecast["weather"].toString();
            weatherInfo.wind = forecast["winddirection"].toString() + forecast["windpower"].toString() + "级";
            weatherInfo.humidity = forecast["humidity"].toString() + "%";
            
            // 模拟未来3天预报
            weatherInfo.forecast[0] = "明天: 晴 26°C";
            weatherInfo.forecast[1] = "后天: 多云 24°C";
            weatherInfo.forecast[2] = "大后天: 阴 22°C";
            
            emit weatherUpdated();
        }
    } else {
        emit errorOccurred("获取天气数据失败: " + jsonObj["info"].toString());
    }
    
    reply->deleteLater();
}

void WeatherManager::onTimerTimeout() {
    fetchWeather();
}

void WeatherManager::saveSettings() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    QString settingsPath = appDataPath + "/weather_settings.ini";
    
    QFile file(settingsPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << "apiKey=" << apiKey << "\n";
        out << "city=" << city << "\n";
        out << "enabled=" << (enabled ? "1" : "0") << "\n";
        file.close();
    }
}

void WeatherManager::loadSettings() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString settingsPath = appDataPath + "/weather_settings.ini";
    
    QFile file(settingsPath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("apiKey=")) {
                apiKey = line.mid(7);
            } else if (line.startsWith("city=")) {
                city = line.mid(5);
            } else if (line.startsWith("enabled=")) {
                enabled = (line.mid(8) == "1");
            }
        }
        file.close();
    }
}
