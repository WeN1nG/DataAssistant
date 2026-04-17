#include <QCoreApplication>
#include <QTcpSocket>
#include <QSslSocket>
#include <QDebug>
#include <QTimer>
#include <QDateTime>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>

void printBanner() {
    qDebug() << "========================================";
    qDebug() << "Simple IMAP Connection Test";
    qDebug() << "========================================";
    qDebug() << "Target: imap.qq.com:993";
    qDebug() << "Started at:" << QDateTime::currentDateTime().toString(Qt::ISODateMs);
    qDebug() << "========================================";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    printBanner();
    
    qDebug() << "[INFO] Disabling system proxy...";
    QNetworkProxyFactory::setUseSystemConfiguration(false);
    qDebug() << "[INFO] System proxy disabled";
    
    QSslSocket* socket = new QSslSocket();
    
    QObject::connect(socket, &QSslSocket::connected, [&]() {
        qDebug() << "[SUCCESS] ✓ TCP Connected!";
        qDebug() << "[INFO] Waiting for SSL handshake...";
    });
    
    QObject::connect(socket, &QSslSocket::encrypted, [&, socket]() {
        qDebug() << "[SUCCESS] ✓ SSL Handshake completed!";
        qDebug() << "[INFO] Protocol:" << socket->protocol();
        qDebug() << "[INFO] Is encrypted:" << socket->isEncrypted();
        
        qDebug() << "[INFO] Sending: A1 CAPABILITY\\r\\n";
        QString cmd = "A1 CAPABILITY\r\n";
        qint64 written = socket->write(cmd.toUtf8());
        qDebug() << "[INFO] Bytes written:" << written;
        
        qDebug() << "[INFO] Waiting for response (10 seconds)...";
    });
    
    QObject::connect(socket, &QSslSocket::readyRead, [&, socket]() {
        QByteArray data = socket->readAll();
        qDebug() << "[SUCCESS] ✓ Received data:" << data.size() << "bytes";
        qDebug() << "[DATA]" << QString(data).trimmed();
        
        qDebug() << "========================================";
        qDebug() << "[SUCCESS] Connection test PASSED!";
        qDebug() << "========================================";
        
        QTimer::singleShot(100, [&]() {
            socket->disconnectFromHost();
            QTimer::singleShot(500, [&]() {
                qApp->quit();
            });
        });
    });
    
    QObject::connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error),
        [&, socket](QAbstractSocket::SocketError error) {
            qDebug() << "[ERROR] Socket error:" << error << "-" << socket->errorString();
            
            qDebug() << "========================================";
            qDebug() << "[FAILED] Connection test FAILED!";
            qDebug() << "========================================";
            qDebug() << "[HINT] Possible causes:";
            qDebug() << "[HINT] 1. Firewall blocking port 993";
            qDebug() << "[HINT] 2. Network proxy interference";
            qDebug() << "[HINT] 3. SSL/TLS version mismatch";
            qDebug() << "[HINT] 4. QQ email server issues";
            qDebug() << "========================================";
            
            QTimer::singleShot(100, [&]() {
                qApp->quit();
            });
        });
    
    QObject::connect(socket, &QSslSocket::sslErrors, [&](const QList<QSslError>& errors) {
        qDebug() << "[WARNING] SSL errors occurred:";
        for (const QSslError& err : errors) {
            qDebug() << "[WARNING] -" << err.errorString();
        }
        qDebug() << "[INFO] Ignoring SSL errors...";
        socket->ignoreSslErrors();
    });
    
    QObject::connect(socket, &QSslSocket::disconnected, [&]() {
        qDebug() << "[INFO] Connection closed";
    });
    
    qDebug() << "[INFO] Setting SSL protocol to TlsV1_3...";
    socket->setProtocol(QSsl::TlsV1_3);
    
    qDebug() << "[INFO] Connecting to imap.qq.com:993...";
    socket->connectToHostEncrypted("imap.qq.com", 993);
    
    qDebug() << "[INFO] Starting 30 second timeout...";
    QTimer timeoutTimer;
    QObject::connect(&timeoutTimer, &QTimer::timeout, [&]() {
        qDebug() << "[TIMEOUT] No response within 30 seconds";
        qDebug() << "[INFO] Socket state:" << socket->state();
        qDebug() << "[INFO] Bytes available:" << socket->bytesAvailable();
        qDebug() << "[INFO] Bytes to write:" << socket->bytesToWrite();
        
        socket->abort();
        
        qDebug() << "========================================";
        qDebug() << "[TIMEOUT] Connection test TIMED OUT!";
        qDebug() << "========================================";
        qDebug() << "[HINT] The connection was established but server did not respond";
        qDebug() << "[HINT] This suggests a firewall or network issue";
        qDebug() << "========================================";
        
        qApp->quit();
    });
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(30000);
    
    return app.exec();
}
