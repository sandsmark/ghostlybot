#ifndef AGENT_H
#define AGENT_H

#include "map.h"

#include <qmath.h>
#include <QHash>
#include <random>
#include <QObject>

#define FEAT_CANEATENEMY 0
#define FEAT_ENEMYDANGEROUS 0
#define FEAT_ENEMYDISTANCE 0
#define FEAT_ENEMYPOSITION 0
#define FEAT_PELLETSAVAILABLE 0
#define FEAT_TOTALPELLETSAVAILABLE 0
#define FEAT_SUPERPELLETDISTANCE 0
#define FEAT_PELLETLOCATION 0
#define FEAT_PELLETMANHATTANDISTANCE 0
#define FEAT_LONGESTPATH 0
#define FEAT_GOINGTOEAT 0


struct Features {
    qreal enemiesNearby = 0.;
    qreal eatsFood = 0.;
    qreal pelletDistance = 0.;
#if FEAT_SUPERPELLETDISTANCE
    qreal superPelletDistance = 0.;
#endif
    qreal bias = 1.;
};

inline bool operator==(const Features &f1, const Features &f2)
{
    return f1.enemiesNearby == f2.enemiesNearby && f1.eatsFood == f2.eatsFood && qFuzzyCompare(f1.pelletDistance, f2.pelletDistance);// && f1.superPelletDistance == f2.superPelletDistance;
}

inline uint qHash(const Features &f)
{
    return qHash(f.enemiesNearby) ^ qHash(f.eatsFood) ^ qHash(f.pelletDistance)
#if FEAT_SUPERPELLETDISTANCE
        ^ qHash(f.superPelletDistance)
#endif
        ;
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
#if FEAT_ENEMYDANGEROUS
        EnemyDangerous = 0,
#endif
        EnemyClose,
        EnemyVeryClose,

#if FEAT_PELLETSAVAILABLE
        PelletsAvailable,
#endif

#if FEAT_TOTALPELLETSAVAILABLE
        TotalPelletsAvailable,
#endif

#if FEAT_ENEMYDISTANCE
        EnemyDistance,
#endif

#if FEAT_ENEMYPOSITION
        EnemyUp,
        EnemyDown,
        EnemyRight,
        EnemyLeft,
#endif

#if FEAT_CANEATENEMY
        CanEatEnemy,
#endif

        PelletDistance,

#if FEAT_SUPERPELLETDISTANCE
        SuperPelletDistance,
#endif

#if FEAT_PELLETMANHATTANDISTANCE
        PelletManhattanDistance,
#endif

#if FEAT_PELLETLOCATION
        PelletUp,
        PelletDown,
        PelletRight,
        PelletLeft,
#endif

//        UpBlocked,
//        DownBlocked,
//        LeftBlocked,
//        RightBlocked,
//        FreeNeighbors,

        VictimDistance,
#if FEAT_LONGESTPATH
        LongestPath,
#endif

#if FEAT_GOINGTOEAT
        GoingToEat,
#endif

        FeatureCount
    };
    Q_ENUM(Feature)

    enum Action {
        Up,
        Down,
        Left,
        Right,
        NoAction,
        MaxAction
    };
    Q_ENUM(Action)

    Agent();
    virtual ~Agent();

    int currentX;
    int currentY;

    // epsilon
    qreal explorationRate = 0.10;

    // gamma, importance of future rewards
    qreal discountFactor = 0.89;
    // alpha
    qreal learningRate = 0.5;

    void update(const State &state, const Action action, const State &nextState, const qreal bonuspoints = 0);

    qreal getQValue(const State &state, const Action action, const QVector<qreal> &weights);

    Action getAction(const State &map, bool inverted);

    QVector<qreal> calculateFeatures(const State &state, const Action action) const;

    void store();
    void load();

private:
    QList<Action> getValidActions(const State &state) const;
//    QHash<Features, qreal> m_weights;
    QVector<qreal> m_weightsA;
    QVector<qreal> m_weightsB;

//    std::random_device m_randomDevice;
//    std::mt19937 m_randomGenerator;
};

#endif // AGENT_H
