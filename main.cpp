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

const float ZONE_RAY = 99.0f;
int RAND_FACTOR = 733;
int RAND_ENEMY_FACTOR = 950;
const int MISPROP_FACTOR = 100;

struct Drone
{
    Drone(int id) : id(id), zone(-1), aimZone(-1)
    {}

    bool findIfDroneIsInZone() const
    {
        cerr << "Drone is in zone: " << zone << endl;
        return zone != -1;
    }

    void move(int aimZone, int x, int y)
    {
        this->aimZone = aimZone;
        cout << x << " " << y << endl;
    }

    bool droneHasNoRelatedZone()
    {
        return aimZone == -1 && zone == -1;
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
            <<  drone.y << ", zone: " << drone.zone;
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
        return enemyDrones.size() > playerDrones.size() ? enemyDrones.size() - playerDrones.size() : 2000000;
    }

    int x; // corresponds to the position of the center of a zone. A zone is a circle with a radius of 100 units.
    int y;
    int id;
    int ownerId;
    Drones enemyDrones;
    Drones playerDrones;
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
            return zone.enemyDrones.empty() && zone.playerDrones.empty() && zone.ownerId != myPlayerId;
        });
        emptyZones.resize(std::distance(emptyZones.begin(),it));
        return emptyZones;
    }

    static void print(const Zones& t)
    {
        for_each(std::begin(t), std::end(t), [](const Zone& x){
            cerr << x << endl;
        });
    }

    template<typename F>
    static Zone getClosestZone(const Zones& zones, F distanceFunction)
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

    static void clearZones(Zones& zones)
    {
        for_each(std::begin(zones), std::end(zones), [](Zone& zone){
            zone.ownerId = -1;
            zone.enemyDrones.clear();
            zone.playerDrones.clear();
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
            return zone.ownerId == -1;
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
    static Player findMyPlayer(const World& world, Players players)
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
    static typename T::value_type findElementWithId(const T& t, int id)
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
    static void moveDrone(const Zones& zones, Drone& drone, F distanceFunction)
    {
        if(drone.droneHasNoRelatedZone())
        {
            auto closestZone = ZoneUtils::getClosestZone(zones, distanceFunction);
            drone.move(closestZone.id, closestZone.x, closestZone.y);
        }
        else
        {
            Zone currZone = Utils::findElementWithId<Zones>(zones,  drone.aimZone);
            drone.move(currZone.id, currZone.x, currZone.y);
        }
    }

    static void moveFromEnemyZone(const Zones& zones, Drone& drone)
    {
        cerr << "Random move from enemies zone" << endl;
        drone.zone = -1;
        Zones notMyZones = Utils::removeElementWithId<Zones>(zones, drone.zone);
        const Zone& zone = ZoneUtils::getZoneWithTheLeastEnemies(notMyZones);
        drone.move(zone.id, zone.x, zone.y);
    }

    template<typename F>
    static void moveFromMyZone(const Zones& zones, Drone& drone, F distanceFunction)
    {
        cerr << "Random move from clear of enemies zone" << endl;

        drone.aimZone = -1;
        Zones notMyZones = Utils::removeElementWithId<Zones>(zones, drone.zone);
        drone.zone = -1;
        moveDrone(notMyZones, drone, distanceFunction);
    }


    template<typename F>
    static void animateDroneInZone(const Zones& zones, Drone& drone, F distanceFunction)
    {
        Zone currZone = Utils::findElementWithId<Zones>(zones,  drone.zone);
        cerr << "findIfDroneIsInZone" << endl;
        if(drone.zone == drone.aimZone)
        {
            drone.aimZone = -1;
        }
        if(Utils::getRandomInt(1, 1000) > RAND_FACTOR && currZone.enemyDrones.empty())
        {
            moveFromMyZone(zones, drone, distanceFunction);
        }
        else if(Utils::getRandomInt(1, 1000) > RAND_ENEMY_FACTOR)
        {
            moveFromEnemyZone(zones, drone);
        }
        else
        {
            cerr << "Random move" << endl;
            cout << currZone.x << " " << currZone.y << endl;
        }
    }

    static double calculateValue(const Drone& drone, const Zone& zone)
    {
        return zone.calculateForceMisproportion()*MISPROP_FACTOR + Utils::calculateDistance(drone, zone);
    }

    static void animateDrone(const Players& players, const World& world, const Zones& zones, Drone& drone)
    {
        auto distanceFunc = std::bind(Utils::calculateDistance<Drone, Zone>, cref(drone), _1);
        auto valueFunc = std::bind(calculateValue, cref(drone), _1);

        Zones unownedZones = ZoneUtils::getUnownedZones(zones);
        Zones emptyZones = ZoneUtils::getEmptyEnemyZones(zones, world.id);


        if (drone.findIfDroneIsInZone())
        {
            cerr << "findIfDroneIsInZone" << endl;
            animateDroneInZone(zones, drone, valueFunc);
        }
        else if(!unownedZones.empty())
        {
            cerr << "unownedZones" << endl;
            drone.aimZone = -1;
            moveDrone(unownedZones, drone, distanceFunc);
        }
        else if (!emptyZones.empty())
        {
            cerr << "emptyZones" << endl;
            drone.aimZone = -1;
            moveDrone(emptyZones, drone, distanceFunc);
        }
        else
        {
            cerr << "Default" << endl;
            moveDrone(zones, drone, valueFunc);
        }
    }

    static void associateDroneWithZone(const Player& player, Drone& drone, Zone& zone, const World& world)
    {
        if (Utils::calculateDistance(drone, zone) < ZONE_RAY)
        {
            drone.zone = zone.id;
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
    Player myPlayer = PlayerUtils::findMyPlayer(world, players);
    myPlayer.drones = DronesUtils::updateDronesWithHistoricalData(oldDrones, myPlayer.drones);
    ZoneUtils::print(zones);
    PlayerUtils::print(players);
    auto droneAnimationFunc = std::bind(DronesUtils::animateDrone, cref(players), cref(world), cref(zones), _1);
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