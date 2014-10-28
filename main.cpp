#include <iostream>
#include <string>
#include <vector>
#include <map>
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
typedef double Distance;
typedef std::map<int, Distance> DistanceMap;
static std::default_random_engine generator;

const float ZONE_RAY = 99.9f;
int RAND_FACTOR = 630;
int RAND_ENEMY_FACTOR = 900;
const int MISPROP_FACTOR = 1500;
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

    template<typename T, typename Z>
    static Z readDistances(const T& t, const Z& z)
    {
        Z newZ = z;
        std::transform(std::begin(t), std::end(t), std::inserter(newZ.distanceMap, std::end(newZ.distanceMap)), [&z]( const typename T::value_type& t_value){
            return std::make_pair(t_value.id, Utils::calculateDistance(t_value, z));
        });
        return newZ;
    }

    template<typename T, typename Z>
    static T fillDistances(const T& t, const Z& z)
    {
        T newT;
        std::transform(std::begin(t), std::end(t), std::back_inserter(newT), [&z](const typename T::value_type& t_value){
            return Utils::readDistances(z, t_value);
        });
        return newT;
    }
};

struct Drone
{

    Drone(int id) : id(id), zone(-1), aimZone(-1)
    {}

    bool isInZone() const
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

    bool hasNoRelatedZone() const
    {
        return aimZone == -1 && zone == -1;
    }

    bool hasAimZone()
    {
        return aimZone != -1;
    }

    bool hasDifferentZoneThanAimZone()
    {
        return isInZone() && hasAimZone() && aimZone != zone;
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


    bool operator<(const Drone& drone)
    {
        return drone.id < id;
    }

    int x;
    int y;
    int id;
    int zone;
    int aimZone;
    DistanceMap distanceMap;
};

istream& operator>>(istream& stream, Drone& drone)
{
    stream >> drone.x >>  drone.y;
    return stream;
}

ostream& operator<<(ostream& stream, const Drone& drone)
{
    stream << "Drone id: " << drone.id << ", x: " << drone.x << ", y: "
            <<  drone.y << ", zone: " << drone.zone << ", aim: " << drone.aimZone << endl;
    stream << "Distances: " << endl;
    for_each(std::begin(drone.distanceMap), std::end(drone.distanceMap), [&stream](const std::pair<int, Distance>& distance){
        stream << "(" << distance.first << "," << distance.second << "),";
    });
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

    bool operator<(const Zone& zone)
    {
        return zone.id < id;
    }

    int x; // corresponds to the position of the center of a zone. A zone is a circle with a radius of 100 units.
    int y;
    int id;
    int ownerId;
    Drones enemyDrones;
    Drones playerDrones;
    Drones aimingDrones;
    DistanceMap distanceMap;
};

ostream& operator<<(ostream& stream, const Zone& zone)
{
    stream << "Zone id: " << zone.id << " , xy: " << zone.x << ", " << zone.y << ", owned: " << zone.ownerId << endl;
    stream << "Enemies: ";
    for_each(std::begin(zone.enemyDrones), std::end(zone.enemyDrones), [&stream](const Drone& drone){
        stream << drone.id << ", ";
    });
    stream << "Player: ";
    for_each(std::begin(zone.playerDrones), std::end(zone.playerDrones), [&stream](const Drone& drone){
        stream << drone.id << ", ";
    });

    stream << "Distances: " << endl;
    for_each(std::begin(zone.distanceMap), std::end(zone.distanceMap), [&stream](const std::pair<int, Distance>& distance){
        stream << "(" << distance.first << "," << distance.second << "),";
    });
    return stream;
}

istream& operator>>(istream& stream, Zone& zone)
{
    stream >> zone.x >> zone.y;
    return stream;
}

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
    Player(int id, bool my) : id(id), my(my)
    {
    }

    bool isMy() const
    {
        return my;
    }

    int id;
    bool my;
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

    static Zones readZonesOwnership(const Zones& zones)
    {
        Zones updatedZones;
        std::transform(std::begin(zones), std::end(zones), std::back_inserter(updatedZones), [](const Zone& zone){
            Zone newZone = zone;
            cin >> newZone.ownerId; cin.ignore();
            return newZone;
        });
        return updatedZones;
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

    static Zone associateDroneWithZone(const Player& player,const Zone& zone, const Drone& drone)
    {
        Zone newZone = zone;
        if (Utils::calculateDistance(drone, zone) < ZONE_RAY)
        {
            cerr << "Association! drone: " << drone.id << endl;
            if(player.isMy())
            {
                newZone.playerDrones.push_back(drone);
            }
            else
            {
                newZone.enemyDrones.push_back(drone);
            }
        }
        return newZone;
    }


    static Zone associatePlayerDronesWithZone(const Zone& zone, const Player& player)
    {
        auto accumulateDrones = std::bind(associateDroneWithZone, player, _1, _2);
        return std::accumulate(std::begin(player.drones), std::end(player.drones), zone, accumulateDrones);
    }

    static Zones associateZonesWithDrones(const Zones &zones, const Players &players)
    {
        Zones zonesWithDrones;
        std::transform(std::begin(zones), std::end(zones), std::back_inserter(zonesWithDrones), [&players](const Zone& zone){
            return std::accumulate(std::begin(players), std::end(players), zone, associatePlayerDronesWithZone);
        });
        return zonesWithDrones;
    }
};



class PlayerUtils
{
public:
    static Player &findMyPlayer(Players& players)
    {
        auto myPlayer = find_if(std::begin(players), std::end(players), [](const Player& player){
            return player.isMy();
        });
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

    static bool isDroneInZone(const Drone& drone, const Zone& zone)
    {
        return (Utils::calculateDistance(drone, zone) < ZONE_RAY);
    }


    static Player associateZonesWithPlayerDrones( const Player& player, const Zone& zone)
    {
        Player newPlayer = player;
        newPlayer.drones.clear();
        std::transform(std::begin(player.drones), std::end(player.drones), std::back_inserter(newPlayer.drones), [&zone](const Drone& drone){
            Drone newDrone = drone;
            if(isDroneInZone(newDrone, zone))
                newDrone.zone = zone.id;
            return newDrone;
        });
        return newPlayer;
    }

    static Players associatePlayersWithZones(const Zones &zones, const Players &players)
    {
        Players playersWithZones;
        std::transform(std::begin(players), std::end(players), std::back_inserter(playersWithZones), [&zones](const Player& player){
            return std::accumulate(std::begin(zones), std::end(zones), player, associateZonesWithPlayerDrones);
        });
        return playersWithZones;
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
            players.emplace_back(Player(i, world.id == i));
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
};

class Strategos
{
public:
    static Drones giveOrders(const Zones& zones, const Player& player)
    {
        Drones newDrones;
        std::transform(std::begin(player.drones), std::end(player.drones), std::back_inserter(newDrones), [](const Drone& drone){
            Drone newDrone = drone;
            newDrone.move(-1, newDrone.x + 1, newDrone.y + 1);
            return newDrone;
        });
        return newDrones;
    }
};

Drones iterateLoop(const Drones& oldDrones, const Zones& zones, const World& world)
{
    Zones zonesWithOwnership = ZoneUtils::readZonesOwnership(zones);
    Players players = WorldUtils::readPlayers(world);
    Players playersWithZones = PlayerUtils::associatePlayersWithZones(zonesWithOwnership, players);
    Player& myPlayer = PlayerUtils::findMyPlayer(playersWithZones);
    std::cerr << "My drones size1: " << myPlayer.drones.size() << endl;

    Zones zonesWithDrones = ZoneUtils::associateZonesWithDrones(zonesWithOwnership, players);
    Zones zonesWithDistances = Utils::fillDistances(zonesWithDrones, myPlayer.drones);

    myPlayer.drones = DronesUtils::updateDronesWithHistoricalData(oldDrones, myPlayer.drones);
    std::cerr << "My drones size2: " << myPlayer.drones.size() << endl;
    myPlayer.drones = Utils::fillDistances(myPlayer.drones, zonesWithDrones);
    std::cerr << "My drones size3: " << myPlayer.drones.size() << endl;

//    ZoneUtils::print(zonesWithDistances);
//    PlayerUtils::print(playersWithZones);

    return Strategos::giveOrders(zonesWithDistances, myPlayer);
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