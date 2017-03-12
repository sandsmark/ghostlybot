#include "agent.h"

#include "map.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QMetaEnum>

#include <queue>

Agent::Agent()
//    :
//    m_weights({-4.53914, 0.391323, 0.304335, -0.304335})
//    m_weights(4, 1)
{
    m_weights.resize(FeatureCount);
    load();
//    m_weights[PelletDistance] = -1.;
//    m_weights.resize(4);
}

Agent::~Agent()
{
//    for (const qreal w : m_weights) {
//        qDebug() << w;
//    }
}

static QString getFeatureName(Agent::Feature key)
{
    static QMetaEnum metaEnum = QMetaEnum::fromType<Agent::Feature>();
    return QString::fromUtf8(metaEnum.valueToKey(key));
}

void Agent::store()
{
    QJsonObject object;

    if (m_weights.length() < FeatureCount) {
        qDebug() << "missing features" << m_weights.length() << (int)FeatureCount;
        return;
    }

    for (int i=0; i<m_weights.length(); i++) {
        Feature feature = (Feature)(i);
        qDebug() << getFeatureName(feature) << m_weights[i];
        object[getFeatureName(feature)] = m_weights[i];
    }

    QFile weightFile("weights.txt");
    if (!weightFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open weight file for writing";
        return;
    }

    weightFile.write(QJsonDocument(object).toJson());
}

void Agent::load()
{
    QFile weightFile("weights.txt");
    if (!weightFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open weight file for reading";
//        m_weights[PelletDistance] = -1;
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(weightFile.readAll());
    QJsonObject object = doc.object();
    for (int i=0; i<FeatureCount; i++) {
        Feature feature = (Feature)i;
        m_weights[i] = object[getFeatureName(feature)].toDouble();
        qDebug() << getFeatureName(feature) << m_weights[i];
    }
}

void Agent::update(const State &state, const Agent::Action action, const State &nextState, const qreal bonuspoints)
{
//    if (bonuspoints) {
//        qDebug() << bonuspoints;
//    }

    const qreal prevValue = getQValue(state, action);
    Q_ASSERT(!qIsNaN(prevValue));
//    const qreal currentValue = (1. - learningRate) * prevValue;
    const qreal reward = bonuspoints + nextState.score - state.score;
//    if (nextState.score && state.score) {
//    reward += nextState.score - state.score;
//    }
//    if (!reward) {
//        return;
//    }
//    qreal reward = nextState.score - state.score + bonuspoints;
//    if (reward) {
//        qDebug() << "REWARD:" << reward;
//    }
    const qreal nextValue = learningRate * (reward + discountFactor * getQValue(nextState, getAction(state)));
    Q_ASSERT(!qIsNaN(nextValue));
    const qreal difference = nextValue - prevValue;
    Q_ASSERT(!qIsNaN(difference));


    QVector<qreal> features = calculateFeatures(state, action);
//    qDebug() << f.eatsFood << f.enemiesNearby << f.pelletDistance << f.superPelletDistance;

    for (int i=0; i<m_weights.size(); i++) {
//        if (difference) {
//            qDebug() << difference;
//        }
        m_weights[i] += learningRate * difference * features[i];
    }
//    if (!getValidActions(state).isEmpty()) {
//        m_weights[f] = currentValue + nextValue;
//    } else {
//        m_weights[f] = currentValue + learningRate * reward;
//    }
}

qreal Agent::getQValue(const State &state, const Action action)
{
    const QVector<qreal> f = calculateFeatures(state, action);

    return std::inner_product(f.begin(), f.end(), m_weights.begin(), 0.);
}

Agent::Action Agent::getAction(const State &state)
{
    QList<Action> actions = getValidActions(state);
    std::random_device randomDevice;
    std::mt19937 randomGenerator(randomDevice());
    std::shuffle(actions.begin(), actions.end(), randomGenerator);
//    qDebug() << "Action count:" << actions.count();

    if (actions.isEmpty()) {
        qDebug() << "No valid actions!";
        // YOLO
        return Up;
    }

    if (qrand() < explorationRate * RAND_MAX) {
//        qDebug() << "Going random yolo";
        return actions[qrand() % actions.count()];
    }

//    qDebug() << "============";
    qreal bestValue = -1;
    Action bestAction = Up;
    for (const Action &candidate : actions) {
        const qreal value = getQValue(state, candidate);
        if (value > bestValue) {
            bestValue = value;
            bestAction = candidate;
        }
    }
//    QVector<qreal> features = calculateFeatures(state, bestAction);
//    qDebug() << "Chose" <<bestAction;
//    qDebug() << "============";
//    qDebug() << bestAction << bestValue << features[PelletDistance] << '\n';


//    if (bestValue > 0) {
//        qDebug() << bestAction;
//    }
//    qDebug() << "Chose action:" << bestAction << bestValue;
//    if (calculateFeatures(state, MaxAction).pelletDistance < calculateFeatures(state, bestAction).pelletDistance) {
//        qWarning() << "Moving away!";
//    }
    return bestAction;
}

struct PathPoint {
    PathPoint() {}
    PathPoint(int _x, int _y) : x(_x), y(_y) {}
    int x = 0;
    int y = 0;
    qreal pathLength = 0;
//    qreal distance = 0;

    int origX = 0;
    int orgiY = 0;

    // Lower distance is better
    bool operator<(const PathPoint &other) const {
//        return pathLength + distance < other.pathLength + other.distance;
        return other.pathLength < pathLength;
//        return other.pathLength + other.distance < pathLength + distance;
    }
    bool operator==(const PathPoint &other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const PathPoint &other) const {
        return x != other.x || y != other.y;
    }
};

uint qHash(const PathPoint &point) {
    return point.x * 1000 + point.y;
}


QVector<qreal> Agent::calculateFeatures(const State &state, const Agent::Action action) const
{
    QVector<qreal> features(FeatureCount, 0);

    int nextX = state.x;
    int nextY = state.y;
//    qDebug() << action;
    switch(action) {
    case Up:
        nextY--;
        break;
    case Down:
        nextY++;
        break;
    case Left:
        nextX--;
        break;
    case Right:
        nextX++;
        break;
    default:
        break;
//        qWarning() << "Invalid action!";
    }

//    for (const Player &enemy : state.map.players) {
//        if (enemy.dangerous && enemy.isNextTo(nextX, nextY)) {
//            f.enemiesNearby += 1.;
//        }
//    }

    const qreal maxDistance = state.map.height() + state.map.width();
    // TODO: search smarter out from current pos or something

    QHash<PathPoint, PathPoint> cameFrom;

    PathPoint currentPosition(nextX, nextY);

    // STL is a steaming pile of shit
    std::priority_queue<PathPoint> queue;
    queue.push(currentPosition);

    QSet<PathPoint> visited;
    PathPoint closestPellet(-1, -1);
    closestPellet.pathLength = maxDistance;
    PathPoint closestEnemy(-1, -1);
    closestEnemy.pathLength = maxDistance;
    bool closestDangerous = false;
    PathPoint closestVictim(-1, -1);
    closestVictim.pathLength = maxDistance;

    while (!queue.empty()) {
        const PathPoint current = queue.top();
        queue.pop();

        if (state.map.powerupAt(current.x, current.y) != Map::NoPowerup) {
            if (current.pathLength < closestPellet.pathLength) {
                closestPellet = current;
            }
//            break;
        }

        // FIXME
        for (const Player &enemy : state.map.players) {
            if (enemy.x != current.x || enemy.y != current.y) {
                continue;
            }
            if (current.pathLength >= closestEnemy.pathLength) {
                continue;
            }
            if (enemy.dangerous) {
                if (current.pathLength < closestEnemy.pathLength) {
                    closestEnemy = current;
                    closestDangerous = enemy.dangerous;
                }
            } else if (state.dangerous) {
                if (current.pathLength < closestVictim.pathLength) {
                    closestVictim = current;
                }
            }
        }

        visited.insert(current);
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                // Don't travel diagonally
                if (dx && dy) {
                    continue;
                }

                const int nx = current.x + dx;
                const int ny = current.y + dy;

                if (!state.map.isValidPosition(nx, ny)) {
                    continue;
                }

                PathPoint neighbor(nx, ny);
                if (visited.contains(neighbor)) {
                    continue;
                }
                neighbor.pathLength = current.pathLength + 1;

                if (neighbor.pathLength > closestPellet.pathLength && neighbor.pathLength > closestEnemy.pathLength) {
                    continue;
                }

                if (cameFrom.contains(neighbor) && cameFrom[neighbor].pathLength < current.pathLength) {
                    continue;
                }

                cameFrom[neighbor] = current;

                queue.push(neighbor);
            }
        }
    }

    features[VictimDistance] = closestVictim.pathLength / maxDistance;
    if (state.map.powerupAt(nextX, nextY) == Map::NoPowerup) {
        features[PelletDistance] = closestPellet.pathLength / maxDistance;
    } else {
        features[PelletDistance] = 0;
    }
//    qDebug() << closestPellet.x;
    if (cameFrom.contains(closestPellet)) {
        features[PelletDistance] = closestPellet.pathLength / maxDistance;

//        while (cameFrom[closestPellet].x != nextX || cameFrom[closestPellet].y != nextY) {
////            qDebug() << closestPellet.x << closestPellet.y;
//            closestPellet = cameFrom[closestPellet];
//            if (!cameFrom.contains(closestPellet)) {
//                qWarning() << "Invalid pellet path";
//                break;
//            }
//        }

//        qDebug() << action;
//        if (closestPellet.x > nextX) {
//            qDebug() << "Pellet down";
////            features[PelletRight] = 1.;
//        } else if (closestPellet.x < nextX) {
////            features[PelletLeft] = 1.;
//            qDebug() << "Pellet up";
//        } else if (closestPellet.y > nextY) {
////            features[PelletDown] = 1.;
//            qDebug() << "Pellet right";
//        } else if (closestPellet.y < nextY) {
////            features[PelletUp] = 1.;
//            qDebug() << "Pellet left";
//        } else {
//            qWarning() << "Don't know which direction the pellet is in";
//        }
    } else if (closestPellet != currentPosition) {
        qWarning() << "Unable to find the closest pellet" << closestEnemy.x << closestEnemy.y << nextX << nextY;
    }

    if (closestEnemy.pathLength < 10) {
        //            qWarning() << "DANGER WILL ROBINSON";
        features[EnemyClose] = 1.;
    }

    if (closestEnemy.pathLength < 2) {
        features[EnemyVeryClose] = 1.;
        //            qWarning() << "VERY DANGER WILL ROBINSON";
    }

    //        features[EnemyDangerous] = closestDangerous ? 1. : 0.;
    features[EnemyDistance] = closestEnemy.pathLength / maxDistance;

    if (cameFrom.contains(closestEnemy)) {
        if (closestEnemy.pathLength < 10) {
//            qWarning() << "DANGER WILL ROBINSON";
            features[EnemyClose] = 1.;
        }

        if (closestEnemy.pathLength < 2) {
            features[EnemyVeryClose] = 1.;
//            qWarning() << "VERY DANGER WILL ROBINSON";
        }

//        features[EnemyDangerous] = closestDangerous ? 1. : 0.;
        features[EnemyDistance] = closestEnemy.pathLength / maxDistance;

//        while (cameFrom[closestEnemy].x != nextX || cameFrom[closestEnemy].y != nextY) {
//            closestEnemy = cameFrom[closestEnemy];
//            if (!cameFrom.contains(closestEnemy)) {
//                qWarning() << "Invalid enemy path";
//                break;
//            }
//        }

//        if (closestEnemy.x > nextX) {
//            features[EnemyRight] = 1.;
//        } else if (closestEnemy.x < nextX) {
//            features[EnemyLeft] = 1.;
//        } else if (closestEnemy.y > nextY) {
//            features[EnemyDown] = 1.;
//        } else if (closestEnemy.y < nextY) {
//            features[EnemyUp] = 1.;
//        } else {
//            qWarning() << "Don't know which direction the enemy is in";
//        }
//    } else if (closestEnemy != currentPosition) {
//        qWarning() << "Unable to find the closest enemy" << closestEnemy.x << closestEnemy.y << nextX << nextY;
    }

//    features[LeftBlocked]    = state.map.isValidPosition(nextX - 1, nextY) ? 0. : 1.;
//    features[RightBlocked]  = state.map.isValidPosition(nextX + 1, nextY) ? 0. : 1.;
//    features[UpBlocked]  = state.map.isValidPosition(nextX, nextY - 1) ? 0. : 1.;
//    features[DownBlocked] = state.map.isValidPosition(nextX, nextY + 1) ? 0. : 1.;

//    {
//        State nextState = state;
//        nextState.x = nextX;
//        nextState.y = nextY;
//        features[FreeNeighbors] = getValidActions(nextState).count() / 4.;
//    }

//    features[CanEatEnemy] = state.dangerous ? 1. : 0.;

//    features[GoingToEat] = state.map.powerupAt(nextX, nextY) != Map::NoPowerup;

    features[Bias] =  1.;

//    for (int i=0; i<features.length(); i++) {
//        const Feature f = (Feature)i;
//        qDebug() << getFeatureName(f) << features[i];
//    }

    return features;

//    for (int x=0; x<state.map.width(); x++) {
//        for (int y=0; y<state.map.height(); y++) {
//            Map::Powerup newPowerup = state.map.powerupAt(x, y);
//            if (newPowerup == Map::NoPowerup) {
//                continue;
//            }

//            //manhattan yo
//            const qreal distance = qSqrt((x - nextX) * (x - nextX) + (y - nextY) * (y - nextY));
////            const qreal distance = (qAbs(x - nextX) + qAbs(y - nextY)) / area;
//            if (newPowerup == Map::NormalPellet && distance < f.pelletDistance) {
//                f.pelletDistance = distance;
//            }
////            if (newPowerup == Map::SuperPellet && distance < f.superPelletDistance) {
////                f.superPelletDistance = distance;
////            }
//        }
//    }
//    if (f.enemiesNearby < 1 && state.map.powerupAt(nextX, nextY) != Map::NoPowerup) {
//        f.eatsFood = 1.;
//    }


//    return f;
    //    return QVector<qreal>({f.enemiesNearby, f.eatsFood, f.pelletDistance, f.bias });
}

QList<Agent::Action> Agent::getValidActions(const State &state) const
{
    QList<Action> actions;
    if (state.map.isWalkable(state.x - 1, state.y)) {
        actions.append(Left);
    }
    if (state.map.isWalkable(state.x + 1, state.y)) {
        actions.append(Right);
    }
    if (state.map.isWalkable(state.x, state.y - 1)) {
        actions.append(Up);
    }
    if (state.map.isWalkable(state.x, state.y + 1)) {
        actions.append(Down);
    }

    return actions;
}
