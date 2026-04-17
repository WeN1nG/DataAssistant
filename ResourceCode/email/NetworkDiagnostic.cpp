#include <QCoreApplication>
#include <QTcpSocket>
#include <QSslSocket>
#include <QDebug>
#include <QTimer>
#include <QDateTime>

class NetworkDiagnostic : public QObject {
    Q_OBJECT

public:
    NetworkDiagnostic(QObject* parent = nullptr) : QObject(parent) {
        connect(&m_timer, &QTimer::timeout, this, &NetworkDiagnostic::onTimeout);
    }

    void runDiagnostics(const QString& host, quint16 port, bool useSsl) {
        qDebug() << "========================================";
        qDebug() << "Network Diagnostic Tool";
        qDebug() << "========================================";
        qDebug() << "Target:" << host << ":" << port << (useSsl ? "(SSL)" : "(No SSL)");
        qDebug() << "Started at:" << QDateTime::currentDateTime().toString(Qt::ISODate);
        qDebug() << "========================================";

        if (useSsl) {
            testSslConnection(host, port);
        } else {
            testTcpConnection(host, port);
        }
    }

private slots:
    void onTimeout() {
        qDebug() << "[TIMEOUT] Operation timed out";
        m_socket->disconnectFromHost();
    }

private:
    void testTcpConnection(const QString& host, quint16 port) {
        qDebug() << "[TEST] Testing TCP connection...";
        
        m_socket = new QTcpSocket(this);
        
        connect(m_socket, &QTcpSocket::connected, this, &NetworkDiagnostic::onConnected);
        connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::error),
                this, &NetworkDiagnostic::onError);
        connect(m_socket, &QTcpSocket::disconnected, this, &NetworkDiagnostic::onDisconnected);

        m_timer.start(10000);
        
        qDebug() << "[INFO] Connecting to" << host << ":" << port << "...";
        m_socket->connectToHost(host, port);
    }

    void testSslConnection(const QString& host, quint16 port) {
        qDebug() << "[TEST] Testing SSL connection...";
        
        QSslSocket* sslSocket = new QSslSocket(this);
        m_socket = sslSocket;
        
        sslSocket->setProtocol(QSsl::TlsV1_2OrLater);
        qDebug() << "[INFO] SSL Protocol set to: TlsV1_2OrLater";
        
        connect(sslSocket, &QSslSocket::connected, this, &NetworkDiagnostic::onConnected);
        connect(sslSocket, QOverload<QAbstractSocket::SocketError>::of(&QSslSocket::error),
                this, &NetworkDiagnostic::onError);
        connect(sslSocket, &QSslSocket::disconnected, this, &NetworkDiagnostic::onDisconnected);
        connect(sslSocket, &QSslSocket::encrypted, this, &NetworkDiagnostic::onEncrypted);
        connect(sslSocket, &QSslSocket::sslErrors,
                this, &NetworkDiagnostic::onSslErrors);
        connect(sslSocket, &QSslSocket::bytesWritten, this, &NetworkDiagnostic::onBytesWritten);

        m_timer.start(30000);
        
        qDebug() << "[INFO] Connecting to" << host << ":" << port << "with SSL...";
        sslSocket->connectToHostEncrypted(host, port);
    }

    void onConnected() {
        qDebug() << "[SUCCESS] ✓ TCP Connection established!";
        qDebug() << "[INFO] Local address:" << m_socket->localAddress().toString();
        qDebug() << "[INFO] Local port:" << m_socket->localPort();
        qDebug() << "[INFO] Peer address:" << m_socket->peerAddress().toString();
        qDebug() << "[INFO] Peer port:" << m_socket->peerPort();
        
        QSslSocket* sslSocket = qobject_cast<QSslSocket*>(m_socket);
        if (sslSocket) {
            qDebug() << "[INFO] Waiting for SSL handshake...";
            if (!sslSocket->waitForEncrypted(10000)) {
                qDebug() << "[ERROR] SSL handshake timeout";
            }
        }
    }

    void onEncrypted() {
        qDebug() << "[SUCCESS] ✓ SSL/TLS encrypted connection established!";
        
        QSslSocket* sslSocket = qobject_cast<QSslSocket*>(m_socket);
        if (sslSocket) {
            qDebug() << "[INFO] SSL Protocol:" << sslSocket->protocol();
            qDebug() << "[INFO] Is encrypted:" << sslSocket->isEncrypted();
            
            QList<QSslCertificate> certChain = sslSocket->peerCertificateChain();
            qDebug() << "[INFO] Peer certificate chain size:" << certChain.size();
            
            qDebug() << "[INFO] Sending test data...";
            QString testData = "A1 CAPABILITY\r\n";
            qint64 written = sslSocket->write(testData.toUtf8());
            qDebug() << "[INFO] Bytes written:" << written;
            
            qDebug() << "[INFO] Waiting for response (5 seconds)...";
            if (!sslSocket->waitForReadyRead(5000)) {
                qDebug() << "[ERROR] No data received within 5 seconds";
                qDebug() << "[INFO] Socket state:" << sslSocket->state();
                qDebug() << "[INFO] Bytes available:" << sslSocket->bytesAvailable();
                qDebug() << "[INFO] Bytes to write:" << sslSocket->bytesToWrite();
            }
        }
    }

    void onBytesWritten(qint64 bytes) {
        qDebug() << "[INFO] Bytes written:" << bytes;
    }

    void onError(QAbstractSocket::SocketError error) {
        qDebug() << "[ERROR] Socket error:" << error << "-" << m_socket->errorString();
        m_timer.stop();
        
        qDebug() << "========================================";
        qDebug() << "Diagnostic completed with errors";
        qDebug() << "========================================";
    }

    void onSslErrors(const QList<QSslError>& errors) {
        qDebug() << "[WARNING] SSL errors occurred:";
        for (const QSslError& error : errors) {
            qDebug() << "  -" << error.errorString();
        }
        
        QSslSocket* sslSocket = qobject_cast<QSslSocket*>(m_socket);
        if (sslSocket) {
            qDebug() << "[INFO] Ignoring SSL errors...";
            sslSocket->ignoreSslErrors();
        }
    }

    void onDisconnected() {
        qDebug() << "[INFO] Connection closed";
        m_timer.stop();
        
        QSslSocket* sslSocket = qobject_cast<QSslSocket*>(m_socket);
        if (sslSocket) {
            qDebug() << "[INFO] Bytes available when disconnected:" << sslSocket->bytesAvailable();
            if (sslSocket->bytesAvailable() > 0) {
                QByteArray data = sslSocket->readAll();
                qDebug() << "[INFO] Data received:" << data.size() << "bytes";
                qDebug() << "[INFO] Data:" << QString(data).trimmed();
            }
        }
        
        qDebug() << "========================================";
        qDebug() << "Diagnostic completed";
        qDebug() << "========================================";
        
        QCoreApplication::quit();
    }

    QIODevice* m_socket;
    QTimer m_timer;
};

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    NetworkDiagnostic diagnostic;
    
    QTimer::singleShot(100, [&]() {
        diagnostic.runDiagnostics("imap.qq.com", 993, true);
    });
    
    return app.exec();
}

#include "main.moc"
