#ifndef MAP_H
#define MAP_H

#include <QVector>
#include <QPoint>
#include <QJsonObject>
#include <QHash>

struct Player {
    Player (const QJsonObject &data) {
        id = data["id"].toInt();
        x = data["x"].toInt();
        y = data["y"].toInt();
        dangerous = data["isdangerous"].toBool();
    }

    int id;
    int x;
    int y;
    bool dangerous;

    bool isNextTo(const int x_, const int y_) const {
        return ((x == x_ && qAbs(y - y_) <= 2) ||
                (qAbs(x - x_) <= 2 && y == y_));
    }
};


struct Map
{
public:
    enum TileCorner {
        UpperLeft,
        UpperRight,
        BottomLeft,
        BottomRight
    };

    enum Powerup {
        NoPowerup = 0,
        NormalPellet = 1,
        SuperPellet = 2
    };

    enum TileType {
        FloorTile = 0,
        WallTile,
        DoorTile,
        PelletTile,
        SuperPelletTile,
        InvalidTile = -1
    };

    explicit Map();

    bool loadMap(const QJsonObject &data);

    int width() const { return m_width; }
    int height() const { return m_height; }

    bool isNull() const;
    bool isValid() const;

    bool isValidPosition(int x, int y) const;
    bool isWithinBounds(int x, int y) const;

    QString name() const;

    Powerup powerupAt(int x, int y) const;
    Powerup takePowerup(int x, int y);

    int pelletsLeft() const { return m_pelletsLeft; }

    QPoint monsterSpawn() const { return m_monsterSpawn; }


    QHash<int, Player> players;

private:
    TileType tileAt(int x, int y) const;

    int m_width;
    int m_height;
    QVector<Powerup> m_powerups;
    QVector<TileType> m_tiles;
    QPoint m_monsterSpawn;
    QString m_name;
    int m_pelletsLeft;
    int m_totalPellets;
};

#endif // MAP_H
