#ifndef AGENT_H
#define AGENT_H

#include "map.h"

#include <qmath.h>
#include <QHash>
#include <random>
#include <QObject>

struct Features {
    qreal enemiesNearby = 0.;
    qreal eatsFood = 0.;
    qreal pelletDistance = 0.;
//    qreal superPelletDistance = 0.;
    qreal bias = 1.;
};

inline bool operator==(const Features &f1, const Features &f2)
{
    return f1.enemiesNearby == f2.enemiesNearby && f1.eatsFood == f2.eatsFood && qFuzzyCompare(f1.pelletDistance, f2.pelletDistance);// && f1.superPelletDistance == f2.superPelletDistance;
}

inline uint qHash(const Features &f)
{
    return qHash(f.enemiesNearby) ^ qHash(f.eatsFood) ^ qHash(f.pelletDistance);// ^ qHash(f.superPelletDistance);
}

struct State {
    Map map;
    int x = 0;
    int y = 0;
    int score = 0;
    bool dangerous = false;
};

class Agent
{
    Q_GADGET

public:
    enum Feature {
        Bias = 0,
//        EnemyDangerous = 0,
        EnemyDistance,
        EnemyUp,
        EnemyDown,
        EnemyRight,
        EnemyLeft,
        CanEatEnemy,

        PelletDistance,
        PelletUp,
        PelletDown,
        PelletRight,
        PelletLeft,

        UpBlocked,
        DownBlocked,
        LeftBlocked,
        RightBlocked,

        GoingToEat,

        FeatureCount
    };
    Q_ENUM(Feature)

    enum Action {
        Up,
        Down,
        Left,
        Right,
        MaxAction
    };
    Q_ENUM(Action)

    Agent();
    virtual ~Agent();

    int currentX;
    int currentY;

    // epsilon
    qreal explorationRate = 0.1;
    // gamma
    qreal discountFactor = 0.8;
    // alpha
    qreal learningRate = 0.2;

    void update(const State &state, const Action action, const State &nextState, qreal bonuspoints = 0);

    qreal getQValue(const State &state, const Action action);

    Action getAction(const State &map);

    QVector<qreal> calculateFeatures(const State &state, const Action action) const;

    void store();
    void load();

private:
    QList<Action> getValidActions(const State &state);
//    QHash<Features, qreal> m_weights;
    QVector<qreal> m_weights;

//    std::random_device m_randomDevice;
//    std::mt19937 m_randomGenerator;
};

#endif // AGENT_H
