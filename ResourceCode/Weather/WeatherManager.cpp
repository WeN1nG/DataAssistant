#include "WeatherManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QDebug>
#include <QDate>

QString WeatherManager::formatForDisplay( const QString str ){
    QString result;
    int commaCount = 0;
    for (int i = 0; i < str.length(); ++i) {
        QChar c = str.at(i);
        result.append(c);
        if (c == ',') {
            commaCount++;
            if (commaCount % 2 == 0) {
                result.append('\n');
            }
        }
    }
    return result;
}

WeatherManager::WeatherManager(QObject *parent) :
    QObject(parent),
    networkManager(new QNetworkAccessManager(this)),
    updateTimer(new QTimer(this)),
    enabled(false)
{
    // qDebug() << "WeatherManager: Initializing...";
    
    connect(networkManager, &QNetworkAccessManager::finished, this, &WeatherManager::onNetworkReply);
    connect(updateTimer, &QTimer::timeout, this, &WeatherManager::onTimerTimeout);
    
    loadSettings();
    
    // qDebug() << "WeatherManager: Settings loaded - enabled:" << enabled 
    //          << "apiKey:" << (apiKey.isEmpty() ? "empty" : "set")
    //          << "city:" << city;
    
    // 每 30 分钟更新一次天气
    updateTimer->start(30 * 60 * 1000);
    
    if (enabled && !apiKey.isEmpty() && !city.isEmpty()) {
        // qDebug() << "WeatherManager: Starting initial fetch...";
        fetchWeather();
    } else {
        // qDebug() << "WeatherManager: Weather not enabled or missing config";
    }
}

WeatherManager::~WeatherManager()
{
    delete networkManager;
    delete updateTimer;
}

QString WeatherManager::weatherToEmoji(const QString &weather) {
    if (weather.isEmpty()) {
        return weather;
    }
    
    QString w = weather;
    if (w.contains("晴", Qt::CaseInsensitive)) {
        if (w.contains("多云")) {
            return "⛅ " + weather;
        }
        return "🌤 " + weather;
    }
    
    if (w.contains("阴", Qt::CaseInsensitive)) {
        return "☁️ " + weather;
    }
    
    if (w.contains("雨", Qt::CaseInsensitive)) {
        if (w.contains("雷", Qt::CaseInsensitive)) {
            return "⛈ " + weather;
        }
        if (w.contains("大", Qt::CaseInsensitive)) {
            return "🌧 " + weather;
        }
        if (w.contains("小", Qt::CaseInsensitive)) {
            return "🌦 " + weather;
        }
        return "🌧 " + weather;
    }
    
    if (w.contains("雪", Qt::CaseInsensitive)) {
        if (w.contains("大", Qt::CaseInsensitive)) {
            return "❄️ " + weather;
        }
        if (w.contains("小", Qt::CaseInsensitive)) {
            return "🌨 " + weather;
        }
        return "❄️ " + weather;
    }
    
    if (w.contains("雾", Qt::CaseInsensitive) || w.contains("霾", Qt::CaseInsensitive)) {
        return "🌫 " + weather;
    }
    
    if (w.contains("风", Qt::CaseInsensitive)) {
        return "💨 " + weather;
    }
    
    if (w.contains("云", Qt::CaseInsensitive)) {
        return "☁️ " + weather;
    }
    
    return weather;
}

void WeatherManager::setApiKey(const QString &newApiKey) {
    apiKey = newApiKey;
    saveSettings();
    // qDebug() << "WeatherManager: API key set to:" << (apiKey.isEmpty() ? "empty" : "***");
}

void WeatherManager::setCity(const QString &newCity) {
    city = newCity;
    saveSettings();
    // qDebug() << "WeatherManager: City set to:" << city;
}

void WeatherManager::setEnabled(bool newEnabled) {
    enabled = newEnabled;
    saveSettings();
    // qDebug() << "WeatherManager: Enabled set to:" << enabled;
    
    if (enabled && !apiKey.isEmpty() && !city.isEmpty()) {
        // qDebug() << "WeatherManager: Fetching weather after enable...";
        fetchWeather();
    }
}

bool WeatherManager::isEnabled() const {
    return enabled;
}

void WeatherManager::fetchWeather() {
    if (!enabled) {
        // qDebug() << "WeatherManager: fetchWeather called but weather is disabled";
        return;
    }
    
    if (apiKey.isEmpty()) {
        // qDebug() << "WeatherManager: fetchWeather called but API key is empty";
        emit errorOccurred("API密钥未设置，请在设置中配置");
        return;
    }
    
    if (city.isEmpty()) {
        // qDebug() << "WeatherManager: fetchWeather called but city is empty";
        emit errorOccurred("城市未设置，请在设置中配置");
        return;
    }
    
    // qDebug() << "WeatherManager: Fetching weather for city:" << city;
    
    // 高德天气 API 请求
    // 注意：高德地图 API 支持城市名称，但需要 URL 编码
    QString encodedCity = QUrl::toPercentEncoding(city);
    QString url = QString("https://restapi.amap.com/v3/weather/weatherInfo?key=%1&city=%2&extensions=all")
        .arg(apiKey)
        .arg(encodedCity);
    
    // qDebug() << "WeatherManager: Request URL:" << url;
    
    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::UserAgentHeader, "PersonalDateAssistant/1.0");
    networkManager->get(request);
}

WeatherInfo WeatherManager::getWeatherInfo() const {
    return weatherInfo;
}

void WeatherManager::onNetworkReply(QNetworkReply *reply) {
    // qDebug() << "WeatherManager: Network reply received";
    // qDebug() << "WeatherManager: Reply error:" << reply->error();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = QString("网络错误: %1").arg(reply->errorString());
        // qDebug() << "WeatherManager: Network error:" << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }
    
    QByteArray response = reply->readAll();
    // qDebug() << "WeatherManager: Response size:" << response.size();
    
    // 打印响应内容（用于调试）
    QString responseStr = QString::fromUtf8(response);
    // qDebug() << "WeatherManager: Response preview:" << responseStr;
    
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
    
    if (jsonDoc.isNull()) {
        // qDebug() << "WeatherManager: Failed to parse JSON response";
        emit errorOccurred("天气数据解析失败");
        reply->deleteLater();
        return;
    }
    
    QJsonObject jsonObj = jsonDoc.object();
    
    // 检查 API 返回状态
    QString status = jsonObj["status"].toString();
    QString info = jsonObj["info"].toString();
    
    // qDebug() << "WeatherManager: API status:" << status << "info:" << info;
    
    if (status != "1") {
        QString errorMsg = QString("API返回错误: %1").arg(info);
        // qDebug() << "WeatherManager: API error:" << errorMsg;
        emit errorOccurred(errorMsg);
        reply->deleteLater();
        return;
    }
    
    // 解析 forecasts 数据（使用 extensions=all 时返回此格式）
    QJsonArray forecasts = jsonObj["forecasts"].toArray();
    QString compactJson = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));
    // qDebug().noquote() << "compactJson:" << formatForDisplay(compactJson);
    
    if (forecasts.isEmpty()) {
        // qDebug() << "WeatherManager: No forecasts data in response";
        emit errorOccurred("未获取到天气数据");
        reply->deleteLater();
        return;
    }
    
    // qDebug() << "WeatherManager: Found" << forecasts.size() << "forecast entries";
    
    QJsonObject forecastObj = forecasts[0].toObject();
    QJsonArray castArray = forecastObj["casts"].toArray();
    
    if (castArray.isEmpty()) {
        // qDebug() << "WeatherManager: No cast data in forecast";
        emit errorOccurred("天气预报数据为空");
        reply->deleteLater();
        return;
    }
    
    // qDebug() << "WeatherManager: Processing" << castArray.size() << "days of forecast";
    
    // 清空之前的预报数据
    weatherInfo = WeatherInfo();
    
    // 获取今天的日期
    QDate today = QDate::currentDate();
    QDate targetDates[3] = {
        today.addDays(1),      // 明天
        today.addDays(2),      // 后天
        today.addDays(3)       // 大后天
    };
    
    // 先处理今天的天气（用于顶部显示）
    for (int i = 0; i < castArray.size(); i++) {
        QJsonObject dayForecast = castArray[i].toObject();
        QString dateStr = dayForecast["date"].toString();
        QDate forecastDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        
        if (forecastDate == today) {
            QString dayWeather = dayForecast["dayweather"].toString();
            QString nightWeather = dayForecast["nightweather"].toString();
            QString dayTemp = dayForecast["daytemp"].toString();
            QString nightTemp = dayForecast["nighttemp"].toString();
            
            weatherInfo.city = forecastObj["city"].toString();
            weatherInfo.condition = weatherToEmoji(dayWeather);
            weatherInfo.nightCondition = weatherToEmoji(nightWeather);
            weatherInfo.temperatureRange = QString("%1~%2°C").arg(nightTemp).arg(dayTemp);
            weatherInfo.temperature = dayTemp + "°C";
            weatherInfo.wind = dayForecast["daywind"].toString();
            
            // qDebug() << "WeatherManager: Today's weather -" 
            //          << "City:" << weatherInfo.city
            //          << "Day:" << weatherInfo.condition
            //          << "Night:" << weatherInfo.nightCondition
            //          << "Temp:" << weatherInfo.temperature
            //          << "Range:" << weatherInfo.temperatureRange;
            break;
        }
    }
    
    // 查找明天、后天、大后天的预报
    int forecastIndex = 0;
    for (int i = 0; i < castArray.size() && forecastIndex < 3; i++) {
        QJsonObject dayForecast = castArray[i].toObject();
        
        QString dateStr = dayForecast["date"].toString();
        QDate forecastDate = QDate::fromString(dateStr, "yyyy-MM-dd");
        
        // 检查是否是我们需要的日期（明天、后天或大后天）
        bool isNeeded = false;
        for (int j = 0; j < 3; j++) {
            if (forecastDate == targetDates[j]) {
                isNeeded = true;
                break;
            }
        }
        
        if (!isNeeded) {
            continue;  // 跳过不需要的日期
        }
        
        QString dayWeather = dayForecast["dayweather"].toString();
        QString nightWeather = dayForecast["nightweather"].toString();
        QString dayTemp = dayForecast["daytemp"].toString();
        QString nightTemp = dayForecast["nighttemp"].toString();
        
        // qDebug() << "WeatherManager: Found forecast for" << dateStr 
        //          << "- Day:" << dayWeather
        //          << "- Night:" << nightWeather
        //          << "- Temp:" << nightTemp << "~" << dayTemp << "°C";
        
        // 获取星期信息
        QString weekDay;
        switch (forecastDate.dayOfWeek()) {
            case 1: weekDay = "周一"; break;
            case 2: weekDay = "周二"; break;
            case 3: weekDay = "周三"; break;
            case 4: weekDay = "周四"; break;
            case 5: weekDay = "周五"; break;
            case 6: weekDay = "周六"; break;
            case 7: weekDay = "周日"; break;
        }
        
        // 填充预报数据（明天、后天、大后天），格式："周一 🌤 晴 - 🌧️ 小雨 18~25°C"
        weatherInfo.forecast[forecastIndex] = QString("%1 %2 - %3 %4~%5°C")
            .arg(weekDay)
            .arg(weatherToEmoji(dayWeather))
            .arg(weatherToEmoji(nightWeather))
            .arg(nightTemp)
            .arg(dayTemp);
        
        forecastIndex++;
    }
    
    // qDebug() << "WeatherManager: Weather data updated successfully";
    emit weatherUpdated();
    
    reply->deleteLater();
}

void WeatherManager::onTimerTimeout() {
    // qDebug() << "WeatherManager: Timer triggered - fetching weather...";
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
        // qDebug() << "WeatherManager: Settings saved to" << settingsPath;
    } else {
        // qDebug() << "WeatherManager: Failed to save settings to" << settingsPath;
    }
}

void WeatherManager::loadSettings() {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString settingsPath = appDataPath + "/weather_settings.ini";
    
    // qDebug() << "WeatherManager: Loading settings from" << settingsPath;
    
    QFile file(settingsPath);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("apiKey=")) {
                apiKey = line.mid(7);
                // qDebug() << "WeatherManager: Loaded apiKey:" << (apiKey.isEmpty() ? "empty" : "set");
            } else if (line.startsWith("city=")) {
                city = line.mid(5);
                // qDebug() << "WeatherManager: Loaded city:" << city;
            } else if (line.startsWith("enabled=")) {
                enabled = (line.mid(8) == "1");
                // qDebug() << "WeatherManager: Loaded enabled:" << enabled;
            }
        }
        file.close();
    } else {
        // qDebug() << "WeatherManager: Settings file not found or cannot be opened";
    }
}
