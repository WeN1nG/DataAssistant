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
    QString wind;
    QString temperatureRange;  // 当天温度区间，格式："18°C~25°C"
    QString forecast[3];
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
    
    void saveSettings();
    void loadSettings();
};

#endif // WEATHERMANAGER_H