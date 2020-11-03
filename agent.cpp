#include "agent.h"

#include "map.h"

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QMetaEnum>

#include <queue>
#include <unordered_set>
#include <unordered_map>

Agent::Agent()
//    :
//    m_weights({-4.53914, 0.391323, 0.304335, -0.304335})
//    m_weights(4, 1)
{
    m_weightsA.resize(FeatureCount);
    m_weightsB.resize(FeatureCount);
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

    for (int i=0; i<m_weightsA.length(); i++) {
        Feature feature = (Feature)(i);
        qDebug() << feature << m_weightsA[i];
        object[getFeatureName(feature) + 'A'] = m_weightsA[i];
        object[getFeatureName(feature) + 'B'] = m_weightsB[i];
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
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(weightFile.readAll());
    QJsonObject object = doc.object();
    for (int i=0; i<FeatureCount; i++) {
        Feature feature = (Feature)i;
        m_weightsA[i] = object[getFeatureName(feature) + 'A'].toDouble();
        m_weightsB[i] = object[getFeatureName(feature) + 'B'].toDouble();
        qDebug() << getFeatureName(feature) << m_weightsA[i];
    }
}

void Agent::update(const State &state, const Agent::Action action, const State &nextState, const qreal bonuspoints)
{
//    if (bonuspoints) {
//        qDebug() << bonuspoints;
//    }

    bool inverted = qrand() > RAND_MAX/2;
    QVector<qreal> &weights = inverted ? m_weightsA : m_weightsB;

    const qreal prevValue = getQValue(state, action, weights);
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
//        qDebug() << "REWARD:" << reward << calculateFeatures(state, action)[VictimDistance] << m_weights[VictimDistance];
//    }
    const qreal nextValue = learningRate * (reward + discountFactor * getQValue(nextState, getAction(state, inverted), weights));
    Q_ASSERT(!qIsNaN(nextValue));
    const qreal difference = nextValue - prevValue;
    if (qIsNaN(difference) || qIsInf(difference)) {
        qDebug() << difference << nextValue << prevValue;
        Q_ASSERT(false);
    }


    QVector<qreal> features = calculateFeatures(state, action);
//    qDebug() << f.eatsFood << f.enemiesNearby << f.pelletDistance << f.superPelletDistance;

    for (int i=0; i<weights.size(); i++) {
        weights[i] += learningRate * difference * features[i];
    }
//    m_weights[VictimDistance] = 1.;
//    if (!getValidActions(state).isEmpty()) {
//        m_weights[f] = currentValue + nextValue;
//    } else {
//        m_weights[f] = currentValue + learningRate * reward;
//    }
}

qreal Agent::getQValue(const State &state, const Action action, const QVector<qreal> &weights)
{
    const QVector<qreal> features = calculateFeatures(state, action);

//    qDebug() << "Chose" <<bestAction << bestValue;
//    qDebug() << '\n';
//    qreal sum = 0;
//    for (int i=0; i<features.length(); i++) {
//        const qreal val = features[i] * m_weights[i];
//        if (qIsInf(val)) {
//            qDebug() << action << Feature(i) << features[i] << m_weights[i];
//            Q_ASSERT(false);
//        }
//        sum += val;
//    }
//    return sum;
//    qDebug() << '\n';

    return std::inner_product(features.begin(), features.end(), weights.begin(), 0.);
}

Agent::Action Agent::getAction(const State &state, bool inverted)
{
    QVector<qreal> &weights = inverted ? m_weightsB : m_weightsA;

    QList<Action> actions = getValidActions(state);
    std::random_device randomDevice;
    std::minstd_rand0 randomGenerator(randomDevice());
//    std::mt19937 randomGenerator(randomDevice());
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
    Action bestAction = actions.takeFirst();
    qreal bestValue = getQValue(state, bestAction, weights);
    for (const Action &candidate : actions) {
        const qreal value = getQValue(state, candidate, weights);
//        qDebug() << candidate << value;
        if (value > bestValue) {
            bestValue = value;
            bestAction = candidate;
        }
    }
//    QVector<qreal> features = calculateFeatures(state, bestAction);
//    qDebug() << "Chose" << bestAction << bestValue;
//    for (int i=0; i<features.length(); i++) {
//        qDebug() << Feature(i)  << features[i] << features[i] * m_weights[i];
//    }
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
    int pellets = 0;
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
    return (qHash(point.y) * 101) + qHash(point.x);
}

namespace std
{
    template <>
    struct hash<PathPoint>
    {
        size_t operator()(PathPoint const &point) const noexcept
        {
            return ((point.y) * 101) + (point.x);
//            return (
//                (51 + std::hash<int>()(point.x)) * 51
//                + std::hash<int>()(point.y)
//            );
        }
    };
}



QVector<qreal> Agent::calculateFeatures(const State &state, const Agent::Action action) const
{
    QVector<qreal> features(FeatureCount, 0);

    int nextX = state.x;
    int nextY = state.y;

    const int mapWidth = state.map.width();
    const int mapHeight = state.map.height();
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
    if (nextX < 0) {
        nextX = mapWidth - 1;
    }
    if (nextX > mapWidth - 1) {
        nextX = 0;
    }

    if (nextY < 0) {
        nextY = mapHeight - 1;
    }
    if (nextY > mapHeight - 1) {
        nextY = 0;
    }

//    for (const Player &enemy : state.map.players) {
//        if (enemy.dangerous && enemy.isNextTo(nextX, nextY)) {
//            f.enemiesNearby += 1.;
//        }
//    }

    const qreal maxDistance = state.map.height() + state.map.width();
    // TODO: search smarter out from current pos or something

    std::unordered_map<PathPoint, PathPoint> cameFrom;

    const PathPoint currentPosition(state.x, state.y);

    const PathPoint candidatePosition(nextX, nextY);

    // STL is a steaming pile of shit
    std::priority_queue<PathPoint> toVisit;
    toVisit.push(candidatePosition);

    std::unordered_set<PathPoint> visited;
//    QSet<PathPoint> visited;
    PathPoint closestPellet(-1, -1);
    closestPellet.pathLength = maxDistance;

//    PathPoint closestSuperPellet(-1, -1);
//    closestSuperPellet.pathLength = maxDistance;

    PathPoint closestEnemy(-1, -1);
    closestEnemy.pathLength = maxDistance;
    bool closestDangerous = false;
    PathPoint closestVictim(-1, -1);
    closestVictim.pathLength = maxDistance;
    PathPoint richestPoint(-1, -1);
    richestPoint.pellets = 0;

    // Trick to avoid dead-end
//    qreal pelletsAvailable = 0;
    const qreal totalPellets = state.map.pelletsLeft();
    qreal longestPath = 0;

    while (!toVisit.empty()) {
        const PathPoint current = toVisit.top();
        toVisit.pop();

        const Map::Powerup powerup = state.map.powerupAt(current.x, current.y);

        if (powerup != Map::NoPowerup) {
//            pelletsAvailable++;
            if (current.pathLength < closestPellet.pathLength) {
                closestPellet = current;
            }
//            if (powerup == Map::SuperPellet) {
//                if (current.pathLength < closestSuperPellet.pathLength) {
//                    closestSuperPellet = current;
//                }
//            }
        }
        if (current.pellets > richestPoint.pellets) {
            richestPoint = current;
        }

        bool pathBlocked = false;
        // should maybe improve this, aka. me fix
        for (const Player &enemy : state.map.players) {
            if (enemy.x != current.x || enemy.y != current.y) {
                continue;
            }

            if (enemy.dangerous && !state.dangerous) {
                if (current.pathLength < closestEnemy.pathLength) {
                    closestEnemy = current;
                    closestDangerous = enemy.dangerous;
                }
            } else if (state.dangerous && !enemy.dangerous) {
                if (current.x == nextX && current.y == nextY) {
                    qWarning() << "WE HAVE HIM IN OUR SIGHTS" << current.pathLength;
                }
                if (current.pathLength < closestVictim.pathLength) {
                    closestVictim = current;
                }
            }

            pathBlocked = true;
            break;
        }

        visited.insert(current);
        if (pathBlocked) {
            continue;
        }

        // We can't walk through this tile
        if (state.map.tileAt(current.x, current.y) == Map::OccupiedTile && !state.dangerous) {
            continue;
        }
        longestPath = std::max(longestPath, current.pathLength);

        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                // Don't travel diagonally
                if (dx && dy) {
                    continue;
                }

                int nx = current.x + dx;
                int ny = current.y + dy;
                if (nx > mapWidth - 1) {
                    nx = 0;
                } else if (nx < 0) {
                    nx = mapWidth - 1;
                }
                if (ny > mapHeight - 1) {
                    ny = 0;
                } else if (ny < 0) {
                    ny = mapHeight - 1;
                }

                if (state.map.tileAt(nx, ny) == Map::WallTile) {
                    continue;
                }

//                if (!state.map.isValidPosition(nx, ny)) {
//                    continue;
//                }

                PathPoint neighbor(nx, ny);
//                if (visited.contains(neighbor)) {
//                if (visited.count(neighbor)) {
//                    continue;
//                }
//                if (neighbor == currentPosition) {
//                    continue;
//                }

                neighbor.pathLength = current.pathLength + 1;
                neighbor.pellets = current.pellets;
                if (state.map.powerupAt(nx, ny) != Map::NoPowerup) {
                    neighbor.pellets++;
                }

//                if (neighbor.pathLength > closestPellet.pathLength) {
//                    continue;
//                }
//                if (neighbor.pathLength > closestVictim.pathLength && neighbor.pathLength > 40) {
//                    continue;
//                }
//                if (neighbor.pathLength > closestEnemy.pathLength && neighbor.pathLength > 20) {
//                    continue;
//                }

//                if (cameFrom.contains(neighbor) && cameFrom[neighbor].pathLength < current.pathLength) {
                if (cameFrom.count(neighbor) && cameFrom[neighbor].pathLength < current.pathLength) {
                    continue;
                }

                cameFrom[neighbor] = current;

                toVisit.push(neighbor);
            }
        }
    }
//    qDebug() << "Longest path:" << longestPath;
//    features[LongestPath] = longestPath / maxDistance;

    //    if (richestPoint.pellets) {
    //        features[PelletsAvailable] = richestPoint.pellets / qreal(state.map.pelletsLeft());
    //    }
    //    features[PelletsAvailable] = pelletsAvailable / totalPellets;

    //    if (cameFrom.contains(closestVictim)) {
    if (closestVictim.x != -1) {
        features[VictimDistance] = closestVictim.pathLength / maxDistance;
    }
    //        if (closestVictim.pathLength < 3) {
    //            qDebug() << closestVictim.pathLength;
    //        }
    //    }

    //    features[PelletDistance] = 1.;
    //    features[PelletManhattanDistance] = 1.;
    //    if (state.map.powerupAt(nextX, nextY) == Map::NoPowerup && cameFrom.contains(closestPellet)) {
    //    if (cameFrom.contains(closestPellet)) {
    if (closestPellet.x != -1) {
        features[PelletDistance] = closestPellet.pathLength / maxDistance;
    }
//    if (closestSuperPellet.x != -1) {
//        features[SuperPelletDistance] = closestSuperPellet.pathLength / maxDistance;
//    }
    //        if (qIsNull(features[PelletDistance]) && !(qIsNull(features[PelletDistance]) * m_weights[PelletDistance] || features[PelletDistance] == 0)) {
    //            qWarning() << closestPellet.pathLength;
//        }
//    }
//        features[PelletManhattanDistance] -= (qAbs(closestPellet.x - nextX) + qAbs(closestPellet.y - nextY)) / maxDistance;
//    } else {
//        features[PelletDistance] = 0;
//    }
//    qDebug() << closestPellet.x;
//    if (cameFrom.contains(closestPellet)) {
//        features[PelletDistance] = closestPellet.pathLength / maxDistance;
//        features[PelletManhattanDistance] = (qAbs(closestPellet.x - nextX) + qAbs(closestPellet.y - nextY)) / maxDistance;

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
//    } else if (closestPellet != currentPosition) {
//        qWarning() << "Unable to find the closest pellet" << closestEnemy.x << closestEnemy.y << nextX << nextY;
//    }

//    if (closestEnemy.pathLength < 10) {
//        //            qWarning() << "DANGER WILL ROBINSON";
//        features[EnemyClose] = 1.;
//    }

//    if (closestEnemy.pathLength < 2) {
//        features[EnemyVeryClose] = 1.;
//        //            qWarning() << "VERY DANGER WILL ROBINSON";
//    }

    //        features[EnemyDangerous] = closestDangerous ? 1. : 0.;
//    features[EnemyDistance] = closestEnemy.pathLength / maxDistance;

//    if (cameFrom.contains(closestEnemy)) {
    if (cameFrom.count(closestEnemy)) {
//        qDebug() << "We have an enemy";
        if (closestEnemy.pathLength < 3) {
//            qWarning() << "DANGER WILL ROBINSON";
            features[EnemyClose] = 1.;
        }

        if (closestEnemy.pathLength < 2) {
            features[EnemyVeryClose] = 1.;
//            qWarning() << "VERY DANGER WILL ROBINSON";
        }

//        features[EnemyDangerous] = closestDangerous ? 1. : 0.;
//        features[EnemyDistance] = 1. - closestEnemy.pathLength / maxDistance;

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

//    features[FreeNeighbors] += state.map.isValidPosition(nextX - 1, nextY) ? 0. : 0.25;
//    features[FreeNeighbors] += state.map.isValidPosition(nextX + 1, nextY) ? 0. : 0.25;
//    features[FreeNeighbors] += state.map.isValidPosition(nextX, nextY - 1) ? 0. : 0.25;
//    features[FreeNeighbors] += state.map.isValidPosition(nextX, nextY + 1) ? 0. : 0.25;

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

    for(qreal &f : features) {
        f /= 10.;
    }
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
