#include "mainloop.h"

#include "map.h"

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QElapsedTimer>

MainLoop::MainLoop(QObject *parent) : QObject(parent),
    m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::readyRead, this, &MainLoop::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &MainLoop::onDisconnected);
}

bool MainLoop::connectToHost(const QString host, int port)
{
    m_socket->connectToHost(host, port);
    return m_socket->waitForConnected();
}

void MainLoop::onReadyRead()
{
    while (m_socket->canReadLine()) {
        parseMessage(m_socket->readLine());
    }
}

void MainLoop::onDisconnected()
{
    qWarning() << "Disconnected from host, shutting down...";
    qApp->quit();
}

void MainLoop::parseMessage(const QByteArray &message)
{
    const QJsonDocument json = QJsonDocument::fromJson(message);
    if (json.isNull()) {
        qWarning() << "Invalid JSON received!" << message;
        return;
    }
    const QJsonObject object = json.object();
    const QString messageType = object["messagetype"].toString();

    if (messageType == "welcome") {
        qDebug() << "Got welcome";
        m_currentState.map.loadMap(object["map"].toObject());
        m_socket->write("NAME MARTiN\n");
        return;
    } else if (messageType == "stateupdate") {
        parseStateUpdate(object["gamestate"].toObject());
        findAction();
    } else if (messageType == "dead") {
        m_agent.update(m_currentState, m_lastAction, m_currentState, -100 - m_currentState.map.pelletsLeft());
        qDebug() << "I died";
    } else if (messageType == "endofround") {
        qDebug() << "Round over";
        m_agent.store();
        // Since this is reset at the server end
        m_currentState.score = 0;
    }
}

void MainLoop::parseStateUpdate(const QJsonObject &state)
{
    if (state.isEmpty()) {
        qWarning() << "Got empty gamestate!";
        return;
    }
    if (!state.contains("map")) {
        qDebug() << "Missing map in" << state;
    }

//    m_prevState = std::move(m_currentState);
//    std::swap(m_prevState, m_currentState);
    State newState;

    newState.map.loadMap(state["map"].toObject());

    const QJsonObject me = state["you"].toObject();
    newState.x = me["x"].toInt();
    newState.y = me["y"].toInt();
    newState.score = me["score"].toInt();
    newState.dangerous = me["isdangerous"].toBool();

    newState.map.loadPlayers(state["others"].toArray());

    m_agent.update(m_currentState, m_lastAction, newState);

    m_currentState = std::move(newState);
}

void MainLoop::findAction()
{
    QElapsedTimer timer;
    timer.start();
    switch(m_agent.getAction(m_currentState)) {
    case Agent::Up:
//        qDebug() << "up";
        m_socket->write("UP\n");
        break;
    case Agent::Left:
//        qDebug() << "left";
        m_socket->write("LEFT\n");
        break;
    case Agent::Right:
//        qDebug() << "right";
        m_socket->write("RIGHT\n");
        break;
    case Agent::Down:
//        qDebug() << "down";
        m_socket->write("DOWN\n");
        break;
    default:
        break;
    }

    if (timer.elapsed() > 10) {
        qDebug() << "Action calculated in" << timer.elapsed() << "ms";
    }
}
