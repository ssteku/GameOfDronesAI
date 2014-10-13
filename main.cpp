#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <math.h>
#include <random>
#include <string>
using namespace std;
using namespace std::placeholders;
/**
* Auto-generated code below aims at helping you parse
* the standard input according to the problem statement.
**/
struct Drone;
struct Zone;
struct Player;

typedef std::vector<Drone> Drones;
typedef std::vector<Zone> Zones;
typedef std::vector<Player> Players;
static std::default_random_engine generator;

const float ZONE_RAY = 99.9f;
int RAND_FACTOR = 630;
int RAND_ENEMY_FACTOR = 900;
const int MISPROP_FACTOR = 1500;

struct Drone
{
    Drone(int id) : id(id), zone(-1), aimZone(-1)
    {}

    bool findIfDroneIsInZone() const
    {
        cerr << "Drone is in zone: " << zone << endl;
        return zone != -1;
    }

    void move(int aimZoneId, int x, int y)
    {
        setAimZone(aimZoneId);
        cerr << "Moving drone: " << id << " to: " << x << ", " << y << endl;
        cout << x << " " << y << endl;
    }

    bool droneHasNoRelatedZone() const
    {
        return aimZone == -1 && zone == -1;
    }

    void setAimZone(int aimZoneId)
    {
        aimZone = aimZoneId;
    }

    void clearAimZone()
    {
        aimZone = -1;
    }

    void setZone(int zoneId)
    {
        zone = zoneId;
        if(zone == aimZone)
        {
            aimZone = -1;
        }
    }

    void clearZone()
    {
        zone = -1;
    }

    int x;
    int y;
    int id;
    int zone;
    int aimZone;
};

istream& operator>>(istream& stream, Drone& drone)
{
    stream >> drone.x >>  drone.y;
    return stream;
}

ostream& operator<<(ostream& stream, const Drone& drone)
{
    stream << "Drone id: " << drone.id << ", x: " << drone.x << ", y: "
            <<  drone.y << ", zone: " << drone.zone << ", aim: " << drone.aimZone;
    return stream;
}

struct Zone
{
    Zone(int id) : id(id), ownerId(-1)
    {

    }
    Zone() : id(-1), ownerId(-1)
    {

    }

    unsigned long  calculateForceMisproportion()const {
        unsigned long misproportion = enemyDrones.size() >= (playerDrones.size()) ?
                enemyDrones.size() - (playerDrones.size()) : 4;
        cerr << "calculateForceMisproportion:" << endl;
        cerr << "   enemyDrones.size(): " << enemyDrones.size() << endl;
        cerr << "   playerDrones.size(): " << playerDrones.size() << endl;
        cerr << "   aimingDrones.size(): " << aimingDrones.size() << endl;
        cerr << "   misproportion: " << misproportion << endl;
        return misproportion;
    }

    int x; // corresponds to the position of the center of a zone. A zone is a circle with a radius of 100 units.
    int y;
    int id;
    int ownerId;
    Drones enemyDrones;
    Drones playerDrones;
    Drones aimingDrones;
};

ostream& operator<<(ostream& stream, const Zone& zone)
{
    stream << "Zone: " << zone.x << ", " << zone.y << ", owned: " << zone.ownerId << endl;
    stream << "Enemies: ";
    for_each(std::begin(zone.enemyDrones), std::end(zone.enemyDrones), [&stream](const Drone& drone){
        stream << drone.id << ", ";
    });
    stream << "Player: ";
    for_each(std::begin(zone.playerDrones), std::end(zone.playerDrones), [&stream](const Drone& drone){
        stream << drone.id << ", ";
    });
    return stream;
}

istream& operator>>(istream& stream, Zone& zone)
{
    stream >> zone.x >> zone.y;
    return stream;
}

class ZoneUtils
{
public:
    static Zones getEmptyEnemyZones(const Zones &zones, unsigned int myPlayerId)
    {
        Zones emptyZones(zones.size());

        auto it = std::copy_if(std::begin(zones), std::end(zones), std::begin(emptyZones), [myPlayerId](const Zone& zone){
            return zone.enemyDrones.empty() && zone.playerDrones.empty() && zone.ownerId != myPlayerId && zone.aimingDrones.size() == 0;
        });
        emptyZones.resize(std::distance(emptyZones.begin(),it));
        return emptyZones;
    }

    static Zones getNotMyZones(const Zones &zones, unsigned int myPlayerId)
    {
        Zones noMyZones(zones.size());

        auto it = std::copy_if(std::begin(zones), std::end(zones), std::begin(noMyZones), [myPlayerId](const Zone& zone){
            return zone.ownerId != myPlayerId;
        });
        noMyZones.resize(std::distance(noMyZones.begin(),it));
        cerr << "notMyZones size : " << noMyZones.size() << endl;
        return noMyZones;
    }

    static void print(const Zones& t)
    {
        for_each(std::begin(t), std::end(t), [](const Zone& x){
            cerr << x << endl;
        });
    }

    template<typename F>
    static const Zone& getClosestZone(const Zones& zones, F distanceFunction)
    {
        return *std::min_element(std::begin(zones), std::end(zones),
                [&distanceFunction](const Zone& zone1, const Zone& zone2){
                    return distanceFunction(zone1) < distanceFunction(zone2);
                });
    }

    static void readZonesOwnership(Zones& zones)
    {
        for_each(std::begin(zones), std::end(zones), [](Zone& zone){
            cin >> zone.ownerId; cin.ignore();
        });
    }

    static const Zone& getZoneWithTheLeastEnemies(const Zones& zones)
    {
        return *std::min_element(std::begin(zones), std::end(zones),
                [](const Zone& zone1, const Zone& zone2){
                    return zone1.enemyDrones.size() < zone2.enemyDrones.size();
                });;
    }

    static Zones getUnownedZones(const Zones& zones)
    {
        Zones unownedZones(zones.size());

        auto it = std::copy_if(std::begin(zones), std::end(zones), std::begin(unownedZones), [](const Zone& zone){
            return zone.ownerId == -1 && zone.aimingDrones.size() == 0;
        });
        unownedZones.resize(std::distance(unownedZones.begin(),it));
        return unownedZones;
    }
};

struct World
{
    static unsigned int p; // number of players in the game (2 to 4 players)
    static unsigned int id; // ID of your player (0, 1, 2, or 3)
    static unsigned int d; // number of drones in each team (3 to 11)
    static unsigned int z; // number of zones on the map (4 to 8)
};

unsigned int World::p = 0;
unsigned int World::id = 0;
unsigned int World::d = 0;
unsigned int World::z = 0;

ostream& operator<<(ostream& stream, World& world)
{
    stream << world.p << ", " << world.id << ", " << world.d << ", " << world.z << endl;
    return stream;
}
istream& operator>>(istream& stream, World& world)
{
    stream >> world.p >>  world.id >>  world.d >>  world.z;
    return stream;
}

struct Player
{
    Player(int id) : id(id)
    {
    }
    int id;
    Drones drones;
};

ostream& operator<<(ostream& stream,const Player& player)
{
    stream << "Player id: " << player.id << ", drones: " << endl;
    for_each(std::begin(player.drones), std::end(player.drones), [&stream](const Drone& drone){
        stream << "    " << drone << endl;
    });
    return stream;
}

class PlayerUtils
{
public:
    static Player &findMyPlayer(const World& world, Players& players)
    {
        auto myPlayer = find_if(std::begin(players), std::end(players), std::bind(PlayerUtils::isMyPlayer, _1, world));
        if (myPlayer == std::end(players))
        {
            cerr << "Couldn't find my player" << endl;
            throw std::logic_error("Couldn't find my player");
        }

        return *myPlayer;
    }

    static void print(const Players& t)
    {
        for_each(std::begin(t), std::end(t), [](const Player& x){
            cerr << x << endl;
        });
    }

    static bool isMyPlayer(const Player& player, const World& world)
    {
        return world.id == player.id;
    }
};

class Utils
{
public:
    static int getRandomInt(int min, int max)
    {
        std::uniform_int_distribution<int> distribution(min, max);
        int randint = distribution(generator);  // generates number in the range 1..6
        cerr << "randint: " << randint << endl;
        return randint;
    }

    template<typename T, typename U>
    static double calculateDistance(const T& t, const U& u)
    {
        return  sqrt((u.x - t.x)*(u.x - t.x) + (u.y - t.y)*(u.y - t.y));
    }

    template<typename T>
    static const typename T::value_type& findElementWithId(const T& t, int id)
    {
        auto currElementIt = find_if(std::begin(t), std::end(t), [id]( const typename T::value_type& element){
            return element.id == id;
        });
        if (currElementIt == std::end(t))
        {
            cerr << "There was no zone with id: " + std::to_string(id) << endl;
            throw std::logic_error("There was no zone with id: " + std::to_string(id));
        }

        return *currElementIt;
    }

    template<typename T>
    static typename T::value_type& findElementReferenceWithId(T& t, int id)
    {
        auto currElementIt = find_if(std::begin(t), std::end(t), [id]( const typename T::value_type& element){
            return element.id == id;
        });
        if (currElementIt == std::end(t))
        {
            cerr << "There was no zone with id: " + std::to_string(id) << endl;
            throw std::logic_error("There was no zone with id: " + std::to_string(id));
        }

        return *currElementIt;
    }

    template<typename T>
    static T removeElementWithId(const T& t, int id)
    {
        T newContainer(t.size());
        auto it = copy_if(std::begin(t), std::end(t), std::begin(newContainer), [id]( const typename T::value_type& element){
            return element.id != id;
        });
        newContainer.resize(std::distance(std::begin(newContainer),it));
        return newContainer;
    }
};

class WorldUtils
{
public:
    static Players readPlayers(const World& world)
    {
        Players players;
        players.reserve(world.p);
        for (int i = 0; i < world.p; i++) {
            players.emplace_back(Player(i));
            for (int j = 0; j < world.d; j++) {
                players.back().drones.emplace_back(Drone(j));
                cin >> players.back().drones.back(); cin.ignore();
            }
        }
        return players;
    }

    static Zones readZones(const World& world)
    {
        Zones zones;
        zones.reserve(world.z);
        for (int i = 0; i < world.z; i++) {
            zones.emplace_back(Zone(i));
            cin >> zones.back(); cin.ignore();
        }
        return zones;
    }
};

class DronesUtils
{
public:
    template<typename F>
    static const Zone& chooseZone(const Zones& zones, const Drone& drone, int playerId, F distanceFunction)
    {
        if(drone.droneHasNoRelatedZone())
        {
            Zones notMyZones = ZoneUtils::getNotMyZones(zones, playerId);
            if(notMyZones.empty())
            {
                return ZoneUtils::getClosestZone(zones, distanceFunction);
            }
            else
            {
                Zone chosenZone = ZoneUtils::getClosestZone(notMyZones, distanceFunction);
                return Utils::findElementWithId<Zones>(zones,  chosenZone.id);
            }
        }
        else
        {
            return Utils::findElementWithId<Zones>(zones,  drone.aimZone);
        }
    }

    static const Zone& chooseZoneWhenInEnemyZone(const Zones &zones, int droneZoneId)
    {
        cerr << "Random move from enemies zone" << endl;
        const Zones& notMyZones = Utils::removeElementWithId<Zones>(zones, droneZoneId);
        const Zone& chosenZone = ZoneUtils::getZoneWithTheLeastEnemies(notMyZones);
        cerr << "chosenZone id: " << chosenZone.id << " xy: " << chosenZone.x << ", " << chosenZone.y << endl;
        return Utils::findElementWithId<Zones>(zones,  chosenZone.id);
    }

    template<typename F>
    static const Zone& chooseZoneWhenInMyZone(const Zones &zones, int droneZoneId, int playerId, F distanceFunction)
    {
        cerr << "Random move from clear of enemies zone" << endl;
        Zones notMyZones = Utils::removeElementWithId<Zones>(zones, droneZoneId);

        Zones notPlayersZones = ZoneUtils::getNotMyZones(notMyZones, playerId);
        if(notPlayersZones.empty())
        {
            Zone chosenZone = ZoneUtils::getClosestZone(notMyZones, distanceFunction);
            return Utils::findElementWithId<Zones>(zones,  chosenZone.id);
        }
        else
        {
            Zone chosenZone = ZoneUtils::getClosestZone(notPlayersZones, distanceFunction);
            return Utils::findElementWithId<Zones>(zones,  chosenZone.id);
        }
    }


    template<typename F>
    static const Zone& animateDroneInZone(const Zones& zones, int droneZoneId, int playerId, F distanceFunction)
    {
        cerr << "animateDroneInZone" << endl;

        const Zone& currZone = Utils::findElementWithId<Zones>(zones,  droneZoneId);

        if(Utils::getRandomInt(1, 1000) > RAND_FACTOR && currZone.enemyDrones.empty())
        {
            return chooseZoneWhenInMyZone(zones, droneZoneId, playerId, distanceFunction);
        }
        else if(Utils::getRandomInt(1, 1000) > RAND_ENEMY_FACTOR)
        {
            return chooseZoneWhenInEnemyZone(zones, droneZoneId);
        }
        else
        {
            return currZone;
        }
    }

    static double calculateValue(const Drone& drone, const Zone& zone)
    {
        double dist = Utils::calculateDistance(drone, zone);
        double value =  zone.calculateForceMisproportion()*MISPROP_FACTOR + dist;
        cerr << "Zone: " << zone.id << " mispr : " << zone.calculateForceMisproportion()*MISPROP_FACTOR << " dist: " << dist << endl;
        return value;
    }

    static void animateDrone(const Players& players, const World& world, Zones& zones, Drone& drone)
    {
        auto distanceFunc = std::bind(Utils::calculateDistance<Drone, Zone>, cref(drone), _1);
        auto valueFunc = std::bind(calculateValue, cref(drone), _1);

        Zones unownedZones = ZoneUtils::getUnownedZones(zones);
        Zones emptyZones = ZoneUtils::getEmptyEnemyZones(zones, world.id);

        if (drone.findIfDroneIsInZone())
        {
            cerr << "findIfDroneIsInZone" << endl;
            Drone originalDrone = drone;
            drone.clearZone();
            const Zone& chosenZone = animateDroneInZone(zones, originalDrone.zone, world.id, valueFunc);
            Zone& originalZone = Utils::findElementReferenceWithId<Zones>(zones,  chosenZone.id);
            originalZone.aimingDrones.push_back(drone);
            drone.move(chosenZone.id, chosenZone.x, chosenZone.y);
        }
        else if(!unownedZones.empty())
        {
            cerr << "unownedZones" << endl;
            drone.clearAimZone();
            const Zone& chosenZone = chooseZone(unownedZones, drone, world.id, distanceFunc);
            Zone& originalZone = Utils::findElementReferenceWithId<Zones>(zones,  chosenZone.id);
            originalZone.aimingDrones.push_back(drone);
            drone.move(chosenZone.id, chosenZone.x, chosenZone.y);

        }
        else if (!emptyZones.empty())
        {
            cerr << "emptyZones" << endl;
            drone.clearAimZone();
            const Zone& chosenZone = chooseZone(emptyZones, drone, world.id, distanceFunc);
            Zone& originalZone = Utils::findElementReferenceWithId<Zones>(zones,  chosenZone.id);
            originalZone.aimingDrones.push_back(drone);
            drone.move(chosenZone.id, chosenZone.x, chosenZone.y);

        }
        else
        {
            cerr << "Default" << endl;
            const Zone& chosenZone = chooseZone(zones, drone, world.id, valueFunc);
            Zone& originalZone = Utils::findElementReferenceWithId<Zones>(zones,  chosenZone.id);
            originalZone.aimingDrones.push_back(drone);
            drone.move(chosenZone.id, chosenZone.x, chosenZone.y);
        }


    }

    static void associateDroneWithZone(const Player& player, Drone& drone, Zone& zone, const World& world)
    {
        if (Utils::calculateDistance(drone, zone) < ZONE_RAY)
        {
            drone.setZone(zone.id);
            cerr << "Association! drone: " << drone.id << endl;
            if(PlayerUtils::isMyPlayer(player, world))
            {
                zone.playerDrones.push_back(drone);
            }
            else
            {
                zone.enemyDrones.push_back(drone);
            }
        }
    }

    static void associatePlayerWithZone(Player& player, Zone& zone, const World& world)
    {
        for_each(std::begin(player.drones), std::end(player.drones),
                std::bind(associateDroneWithZone, cref(player), _1, ref(zone), world));
    }

    static Drones updateDrone(const Drones& oldDrones, Drones resultDrones, const Drone& newDrone)
    {
        Drone oldDrone = Utils::findElementWithId(oldDrones, newDrone.id);
        resultDrones.emplace_back(newDrone);
        resultDrones.back().aimZone = oldDrone.aimZone;;
        return resultDrones;
    }

    static Drones updateDronesWithHistoricalData(const Drones& oldDrones, const Drones& newDrones)
    {
        Drones resultDrones;
        if(oldDrones.empty())
            return newDrones;
        auto updateDronesFunc = bind(updateDrone, oldDrones, _1, _2);
        resultDrones = accumulate(std::begin(newDrones), std::end(newDrones), Drones(), updateDronesFunc);
        return resultDrones;
    }


    static void associateDronesWithZones(Zones &zones, Players &players, const World &world)
    {
        for_each(std::begin(zones), std::end(zones), [&players, &world](Zone& zone){
            for_each(std::begin(players), std::end(players), std::bind(associatePlayerWithZone, _1, ref(zone), world));
        });
    }
};


Drones iterateLoop(const Drones& oldDrones, Zones zones, const World& world)
{
    ZoneUtils::readZonesOwnership(zones);
    Players players = WorldUtils::readPlayers(world);
    DronesUtils::associateDronesWithZones(zones, players, world);
    Player& myPlayer = PlayerUtils::findMyPlayer(world, players);
    myPlayer.drones = DronesUtils::updateDronesWithHistoricalData(oldDrones, myPlayer.drones);
    ZoneUtils::print(zones);
    PlayerUtils::print(players);
    auto droneAnimationFunc = std::bind(DronesUtils::animateDrone, cref(players), cref(world), ref(zones), _1);
    for_each(std::begin(myPlayer.drones), std::end(myPlayer.drones), droneAnimationFunc);

    return myPlayer.drones;
}

int main()
{
    World world;
    cin >> world; cin.ignore();
    std::vector<Zone> zones = WorldUtils::readZones(world);
    Drones oldDrones;
    // game loop
    while (1) {
        oldDrones = iterateLoop(oldDrones, zones, world);
    }
}