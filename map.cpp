#include "map.h"

#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonValue>

Map::Map() :
    m_width(0),
    m_height(0),
    m_pelletsLeft(0),
    m_totalPellets(0)
{
}

bool Map::loadMap(const QJsonObject &data)
{
    m_totalPellets = 0;

    m_width = data["width"].toInt();
    m_height = data["height"].toInt();

    QJsonArray tiles = data["content"].toArray();
    if (tiles.isEmpty()) {
        qWarning() << "No map contents!" << data;
        return false;
    }

    m_tiles.clear();
    m_tiles.reserve(m_width * m_height);
    for (int y=0; y<tiles.count(); y++) {
        const QString line = tiles[y].toString();

        if (line.isEmpty()) {
            qWarning() << "Empty line in map!";
            return false;
        }

        if (line.length() != m_width) {
            qWarning() << "Invalid map line at line" << y << "width:" << line.length() << line;
            return false;
        }

        for (int x=0; x<line.length(); x++) {
            switch(line[x].toLatin1()) {
            case '_':
                m_tiles.append(FloorTile);
                break;
            case '|':
                m_tiles.append(WallTile);
                break;
            case '-':
                m_tiles.append(DoorTile);
                break;
            case '.':
                m_tiles.append(PelletTile);
                m_totalPellets++;
                break;
            case 'o':
                m_tiles.append(SuperPelletTile);
                break;
            case '#':
                m_tiles.append(FloorTile);
                break;
            case '@':
                m_tiles.append(FloorTile);
                m_monsterSpawn = QPoint(x, y);
                break;
            default:
                qWarning() << "Invalid tile: '" << line[x] << "'";
                m_tiles.append(InvalidTile);
                break;
            }
        }
    }

    m_powerups.clear();
    m_pelletsLeft = 0;
    for (int y=0; y<m_height; y++) {
        for (int x=0; x<m_width; x++) {
            switch(tileAt(x, y)) {
            case PelletTile:
                m_powerups.append(NormalPellet);
                m_pelletsLeft++;
                break;
            case SuperPelletTile:
                m_powerups.append(SuperPellet);
                break;
            default:
                m_powerups.append(NoPowerup);
            }
        }
    }
    m_pelletsLeft = data["pelletsleft"].toInt();

    return true;
}

bool Map::loadPlayers(const QJsonArray &others)
{
    players.clear();
    for (const QJsonValue &otherVal : others) {
        const QJsonObject other = otherVal.toObject();
        Player player(other);

        if (!isWithinBounds(player.x, player.y)) {
            return false;
        }

        const int pos = player.y * m_width + player.x;
        m_tiles[pos] = OccupiedTile;

        players.insert(player.id, std::move(player));
//        players.insert(other["id"].toInt(), Player(other));
    }

    return true;
}

bool Map::isValid() const
{
    return (!m_tiles.isEmpty() && m_width * m_height == m_tiles.size());
}

bool Map::isWalkable(int x, int y) const
{
    if (Q_UNLIKELY(x < 0)) {
        x = width() - 1;
    }
    if (Q_UNLIKELY(y < 0)) {
        y = height() - 1;
    }
    if (Q_UNLIKELY(x > width() - 1)) {
        x = 0;
    }
    if (Q_UNLIKELY(y > height() - 1)) {
        y = 0;
    }

    const TileType tile = tileAt(x, y);
    return (tile != WallTile && tile != InvalidTile/* && tile != OccupiedTile*/);
}

bool Map::isValidPosition(int x, int y) const
{
    const TileType tile = tileAt(x, y);
    return (tile != WallTile && tile != InvalidTile);
}

bool Map::isWithinBounds(int x, int y) const
{
    if (x < 0 || x >= m_width)
        return false;

    if (y < 0 || y >= m_height)
        return false;

    return true;
}

QString Map::name() const
{
    return m_name;
}

Map::Powerup Map::powerupAt(int x, int y) const
{
    const int pos = y * m_width + x;
    if (pos >= m_powerups.size() || pos < 0) {
        return NoPowerup;
    }
    return m_powerups[pos];
}

Map::Powerup Map::takePowerup(int x, int y)
{
    if (!isWithinBounds(x, y)) {
        return NoPowerup;
    }
    const int pos = y * m_width + x;
    if (pos >= m_powerups.size()) {
        return NoPowerup;
    }
    Powerup powerup = m_powerups[pos];
    if (powerup == NoPowerup) {
        return powerup;
    }

    if (powerup == NormalPellet) {
        m_pelletsLeft--;
    }

    m_powerups[pos] = NoPowerup;
    return powerup;
}

Map::TileType Map::tileAt(int x, int y) const
{
    const int pos = y * m_width + x;
    if (pos >= m_tiles.length() || pos < 0) {
        return InvalidTile;
    }
    return m_tiles[pos];
}
