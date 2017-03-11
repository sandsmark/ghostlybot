#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "agent.h"
#include "map.h"

#include <QObject>
#include <QHash>
#include <QJsonObject>

class QTcpSocket;

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

    State m_currentState;
    State m_prevState;
    Agent::Action m_lastAction;
    QTcpSocket *m_socket;
    Agent m_agent;
};

#endif // MAINLOOP_H
