#ifndef MAINLOOP_H
#define MAINLOOP_H

#include <QObject>
#include <QHash>
#include <QJsonObject>

class Map;
class QTcpSocket;

struct Player {
    Player (const QJsonObject &data) {
        id = data["id"].toInt();
        x = data["x"].toInt();
        y = data["y"].toInt();
    }

    int id;
    int x;
    int y;
};

class MainLoop : public QObject
{
    Q_OBJECT
public:
    explicit MainLoop(QObject *parent = 0);

    bool connectToHost(const QString host, int port);

signals:

public slots:

private slots:
    void onReadyRead();
    void onDisconnected();

private:
    void parseMessage(const QByteArray &message);
    void parseStateUpdate(const QJsonObject &state);
    void findAction();

    Map *m_map;
    QTcpSocket *m_socket;
    bool m_alive;
    int m_currentX;
    int m_currentY;
    QHash<int, Player> m_others;
};

#endif // MAINLOOP_H
