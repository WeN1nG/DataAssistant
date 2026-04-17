#ifndef PYTHONEMBEDDER_H
#define PYTHONEMBEDDER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <QVector>

class PythonEmbedder : public QObject
{
    Q_OBJECT

public:
    static PythonEmbedder& instance();
    
    bool initialize();
    void shutdown();
    bool isInitialized() const { return m_initialized; }
    
    QVariant callPythonFunction(const QString& moduleName, const QString& functionName, const QVariantList& args = QVariantList());
    QVariant evalPythonCode(const QString& code);
    
    QString getLastError() const { return m_lastError; }

signals:
    void initialized();
    void errorOccurred(const QString& error);

private:
    PythonEmbedder(QObject* parent = nullptr);
    ~PythonEmbedder();
    PythonEmbedder(const PythonEmbedder&) = delete;
    PythonEmbedder& operator=(const PythonEmbedder&) = delete;
    
    bool m_initialized;
    QString m_lastError;
    
    bool setPythonPath();
};

#endif