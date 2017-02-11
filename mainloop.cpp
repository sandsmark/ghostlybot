#include "mainloop.h"

#include "map.h"

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>

MainLoop::MainLoop(QObject *parent) : QObject(parent),
    m_map(new Map(this)),
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
        m_map->loadMap(object["map"].toObject());
        m_socket->write("NAME MARTiN\n");
        return;
    } else if (messageType == "stateupdate") {
        parseStateUpdate(object["gamestate"].toObject());
        findAction();
    } else if (messageType == "dead") {
        qDebug() << "I died";
    } else if (messageType == "endofround") {
        qDebug() << "Round over";
    }
}

void MainLoop::parseStateUpdate(const QJsonObject &state)
{
    if (state.isEmpty()) {
        qWarning() << "Got empty gamestate!";
        return;
    }

    m_map->loadMap(state["map"].toObject());

    const QJsonObject me = state["you"].toObject();
    m_currentX = me["x"].toInt();
    m_currentY = me["y"].toInt();

    m_others.clear();
    const QJsonArray others = state["others"].toArray();
    for (const QJsonValue &otherVal : others) {
        const QJsonObject other = otherVal.toObject();
        m_others.insert(other["id"].toInt(), Player(other));
    }
}

void MainLoop::findAction()
{
    int action = qrand() % 4;
    switch(action) {
    case 0:
        m_socket->write("UP\n");
        break;
    case 1:
        m_socket->write("LEFT\n");
        break;
    case 2:
        m_socket->write("RIGHT\n");
        break;
    case 3:
        m_socket->write("DOWN\n");
        break;
    }
}
