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
const float ZONE_RAY = 100.0f;
int RAND_FACTOR = 400;
const int MISPROP_FACTOR = 10;

struct Drone
{
    Drone(int id) : id(id), zone(-1), aimZone(-1)
    {}
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

struct World
{
    int p; // number of players in the game (2 to 4 players)
    int id; // ID of your player (0, 1, 2, or 3)
    int d; // number of drones in each team (3 to 11)
    int z; // number of zones on the map (4 to 8)
};

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

void readZonesOwnership(Zones& zones)
{
    for_each(std::begin(zones), std::end(zones), [](Zone& zone){
        cin >> zone.ownerId; cin.ignore();
    });
}

Zones readZones(const World& world)
{
    Zones zones;
    zones.reserve(world.z);
    for (int i = 0; i < world.z; i++) {
        zones.emplace_back(Zone(i));
        cin >> zones.back(); cin.ignore();
    }
    return zones;
}

Players readPlayers(const World& world)
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

template<typename T, typename U>
double calculateDistance(const T& t, const U& u)
{
    auto dist =  sqrt((u.x - t.x)*(u.x - t.x) + (u.y - t.y)*(u.y - t.y));
    //cerr << "Distance: " << dist << endl;
}

bool isMyPlayer(const Player& player, const World& world)
{
    return world.id == player.id;
}

Player findMyPlayer(const World& world, Players players)
{
    auto myPlayer = find_if(std::begin(players), std::end(players), std::bind(isMyPlayer, _1, world));
    if (myPlayer == std::end(players))
    {
        cerr << "Couldn't find my player" << endl;
        throw std::logic_error("Couldn't find my player");
    }

    return *myPlayer;
}

Zones getUnownedZones(const Zones& zones)
{
    Zones unownedZones(zones.size());

    auto it = std::copy_if(std::begin(zones), std::end(zones), std::begin(unownedZones), [](const Zone& zone){
            return zone.ownerId == -1;
    });
    unownedZones.resize(std::distance(unownedZones.begin(),it));
    return unownedZones;
}

template<typename T>
typename T::value_type findElementWithId(const T& t, int id)
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

Zones getEmptyZones(const Zones& zones, const Players& players, const World& world)
{
    Zones emptyZones(zones.size());

    auto it = std::copy_if(std::begin(zones), std::end(zones), std::begin(emptyZones), [&world](const Zone& zone){
        return zone.enemyDrones.empty() && zone.playerDrones.empty() && zone.ownerId != world.id;
    });
    emptyZones.resize(std::distance(emptyZones.begin(),it));
    return emptyZones;
}

Zones getNotDronesZones(const Zones& zones, const Drone& drone)
{
    Zones notDronesZones(zones.size());

    auto it = std::copy_if(std::begin(zones), std::end(zones), std::begin(notDronesZones),
            [&drone](const Zone& zone){
                return calculateDistance(drone, zone) > ZONE_RAY;
    });

    notDronesZones.resize(std::distance(notDronesZones.begin(),it));
    return notDronesZones;
}

bool findIfDroneIsInZone(const Drone& drone)
{
    cerr << "Drone is in zone: " << drone.zone << endl;
    return drone.zone != -1;
}

int getRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> distribution(min, max);
    int randint = distribution(generator);  // generates number in the range 1..6
    cerr << "randint: " << randint << endl;
    return randint;
}

template<typename F>
Zone getClosestZone(const Zones& zones, F distanceFunction)
{
    return *std::min_element(std::begin(zones), std::end(zones),
        [&distanceFunction](const Zone& zone1, const Zone& zone2){
            return distanceFunction(zone1) < distanceFunction(zone2);
    });
}

template<typename F>
void moveDrone(const Zones& zones, Drone& drone, F distanceFunction)
{
    auto closestZone = getClosestZone(zones, distanceFunction);
    if(drone.aimZone == -1 && drone.zone == -1)
    {
        cerr << "getClosestZone" << endl;
        drone.aimZone = closestZone.id;
        cout << closestZone.x << " " << closestZone.y << endl;
    }
    else
    {
        cerr << "findElementWithId aimZone: " << drone.aimZone << endl;
        Zone currZone = findElementWithId<Zones>(zones,  drone.aimZone);
        drone.aimZone = currZone.id;
        cout << currZone.x << " " << currZone.y << endl;
    }
}



template<typename F>
void animateDroneInZone(const Zones& zones, Drone& drone, F distanceFunction)
{


    Zone currZone = findElementWithId<Zones>(zones,  drone.zone);
    cerr << "findIfDroneIsInZone" << endl;
    if(drone.zone == drone.aimZone)
    {
        drone.aimZone = -1;
    }
    if(getRandomInt(1, 1000) > RAND_FACTOR && currZone.enemyDrones.empty())
    {
        cerr << "Random move from clear of enemies zone" << endl;
        drone.zone = -1;
        moveDrone(zones, drone, distanceFunction);
    }
    // else if(getRandomInt(1, 1000) > RAND_FACTOR*9)
    // {
    //     cerr << "Random move from enemies zone" << endl;
    //     const Zone& zone = *std::min_element(std::begin(zones), std::end(zones),
    //     [](const Zone& zone1, const Zone& zone2){
    //         return zone1.enemyDrones.size() < zone2.enemyDrones.size();
    //     });;

    //     drone.aimZone = zone.id;
    //     cout << zone.x << " " << zone.y << endl;
    // }
    else
    {
        cerr << "Random move" << endl;
        // drone.zone = -1;
        // moveDrone(zones, drone, distanceFunction);
        cout << currZone.x << " " << currZone.y << endl;
    }
}

int  calculateForceMisproportion(const Zone& zone)
{
    return abs(zone.playerDrones.size() - zone.enemyDrones.size());
}

int calculateValue(const Drone& drone, const Zone& zone)
{
    return calculateForceMisproportion(zone)*MISPROP_FACTOR + calculateDistance(drone, zone);
}

void animateDrone(const Zones& zones, const Zones& unownedZones, const Zones& emptyZones, Drone& drone)
{
    auto distanceFunc = std::bind(calculateDistance<Drone, Zone>, cref(drone), _1);
    auto valueFunc = std::bind(calculateValue, cref(drone), _1);


    if (findIfDroneIsInZone(drone))
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



void associateDroneWithZone(Player& player, Drone& drone, Zone& zone, const World& world)
{
    if (calculateDistance(drone, zone) < ZONE_RAY)
    {
        drone.zone = zone.id;
        cerr << "Association! drone: " << drone.id << endl;
        if(isMyPlayer(player, world))
        {
            zone.playerDrones.push_back(drone);
        }
        else
        {
            zone.enemyDrones.push_back(drone);
        }
    }
}

void associatePlayerWithZone(Player& player, Zone& zone, const World& world)
{
    for_each(std::begin(player.drones), std::end(player.drones),
        std::bind(associateDroneWithZone, ref(player), _1, ref(zone), world));
}

void associateDronsWithZones(Zones& zones, Players& players, const World& world)
{
    for_each(std::begin(zones), std::end(zones), [&players, &world](Zone& zone){
        for_each(std::begin(players), std::end(players), std::bind(associatePlayerWithZone, _1, ref(zone), world));
    });
}

void clearZones(Zones& zones)
{
    for_each(std::begin(zones), std::end(zones), [](Zone& zone){
        zone.ownerId = -1;
        zone.enemyDrones.clear();
        zone.playerDrones.clear();
    });
}


void print(const Players& t)
{
    for_each(std::begin(t), std::end(t), [](const Player& x){
        cerr << x << endl;
    });
}

void print(const Zones& t)
{
    for_each(std::begin(t), std::end(t), [](const Zone& x){
        cerr << x << endl;
    });
}

Drones updateDrone(const Drones& oldDrones, Drones resultDrones, const Drone& newDrone)
{
    Drone oldDrone = findElementWithId(oldDrones, newDrone.id);
    resultDrones.emplace_back(newDrone);
    resultDrones.back().aimZone = oldDrone.aimZone;;
    return resultDrones;
}

Drones updateDronesWithHistoricalData(const Drones& oldDrones, const Drones& newDrones)
{
    Drones resultDrones;
    if(oldDrones.empty())
        return newDrones;
    auto updateDronesFunc = bind(updateDrone, oldDrones, _1, _2);
    resultDrones = accumulate(std::begin(newDrones), std::end(newDrones), Drones(), updateDronesFunc);
    return resultDrones;
}

int main()
{
    World world;
    cin >> world; cin.ignore();
    std::vector<Zone> zones = readZones(world);
    Drones oldDrones;
    // game loop
    while (1) {
        clearZones(zones);
        readZonesOwnership(zones);
        Players players = readPlayers(world);
        associateDronsWithZones(zones, players, world);
        Player myPlayer = findMyPlayer(world, players);
        myPlayer.drones = updateDronesWithHistoricalData(oldDrones, myPlayer.drones);
        print(zones);
        print(players);
        Zones unownedZones = getUnownedZones(zones);
        Zones emptyZones = getEmptyZones(zones, players, world);
        cerr << "Unowned zones : " << unownedZones.size() << endl;
        auto droneAnimationFunc = std::bind(animateDrone, cref(zones), cref(unownedZones), cref(emptyZones), _1);
        // RAND_FACTOR *= 1.1;
        for_each(std::begin(myPlayer.drones), std::end(myPlayer.drones), droneAnimationFunc);
        oldDrones = myPlayer.drones;
    }
}