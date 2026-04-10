#ifndef WEATHERMANAGER_H
#define WEATHERMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

struct WeatherInfo {
    QString city;
    QString temperature;
    QString condition;
    QString nightCondition;    // 晚上天气描述，如："🌧️ 小雨"
    QString wind;
    QString temperatureRange;  // 当天温度区间，格式："18°C~25°C"
    QString forecast[3];        // 未来三日预报，格式："周一 🌤 晴 - 🌧️ 小雨 18~25°C"
};

class WeatherManager : public QObject
{
    Q_OBJECT

public:
    explicit WeatherManager(QObject *parent = nullptr);
    ~WeatherManager();

    void setApiKey(const QString &apiKey);
    void setCity(const QString &city);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    
    void fetchWeather();
    WeatherInfo getWeatherInfo() const;
    
    static QString weatherToEmoji(const QString &weather);

signals:
    void weatherUpdated();
    void errorOccurred(const QString &error);

private slots:
    void onNetworkReply(QNetworkReply *reply);
    void onTimerTimeout();

private:
    QNetworkAccessManager *networkManager;
    QTimer *updateTimer;
    QString apiKey;
    QString city;
    bool enabled;
    WeatherInfo weatherInfo;
    QString formatForDisplay( const QString str );
    
    void saveSettings();
    void loadSettings();
};

#endif // WEATHERMANAGER_H