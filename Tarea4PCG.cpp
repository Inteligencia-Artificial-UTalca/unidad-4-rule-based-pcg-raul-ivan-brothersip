#include <iostream>
#include <vector>
#include <random>   // Para random
#include <iomanip>
#include <queue>    // para las colas
#include <chrono>   // para tener el seed para el random
#include <cmath>    //matematicas aaaaaaaaaaaa

// Define Map as a vector of vectors of integers.
//No olvidar g++ -std=c++11 -Wall -Wextra -pedantic Tarea4PCG.cpp -o isaac

using namespace std;
using Map = std::vector<std::vector<int>>;

/*
//drunk agent para la creacion de cuartos
//celullar automata para ver donde se crea el secret room

//matriz para representar el mapa
//arbol para poder determinar el camino


//GRILLA
// PRIORIDAD

//cuartos tienen 4 posibles direcciones para comenzar, de ahi se crea el cuarto en base a esa posicion,
//eso quiere decir de que tambien esta la posibilidad de crear 2 o 3 cuartos alrededor de un cuarto
//de ahi se instancian el siguiente cuarto y la posibilidad de cuartos alrededor disminuye.
//asi hasta que llegue a su fin.

//se identifican los cuartos finales, es decir que tienen solo 1 cuarto alrededor. ahi se ponen items o tiendas
//cuarto secreto se revisa (en un espacio disponible) alrededor suyo y si hay 2 o mas cuartos alrededor, se crea ahi.
//el jefe se crea en el final, es decir en la casilla mas lejana.
//cuarto super secreto se crea en el cuarto que esta antes del jefe.
//salida secreta se crea en cualquier habitacion excepto el jefe.


// cuarto secreto = esta al lado del final de un cuarto final. debe estar rodeado de min 2
// cuarto super secreto = al lado del (cuarto anterior del) jefe
// boss room = cuarto mas lejos
// tienda = cuartos finales
// items = cuartos finales

// HEURISTICA
// sacrificio
// maldicion

// PROBABILIDAD
// salida secreta (probabilidad)
*/

//tipo de cuarto, para imprimir el mapa
const int EMPTY = 0;
const int UNAVAILABLE = 1;
const int BOSS = 2;
const int ITEM = 3;
const int SHOP = 4;
const int SECRET = 5; 
const int HIDDEN = 6;
const int CURSE = 7;
const int SACRI = 8;
const int EXIT = 9;


/*
//TRES MAPAS
//MAPA 1 = cuartos
//MAPA 2 = tipos de cuartos
//MAPA 3 = representacion grafica del mapa

//Ejemplo de como se ve
//grilla original de 2x8, tipo de cuartos(tamaños), posicion inicial es (2,1)
//   Este
//   v
// 1 1 3 3
// 0 0 1 0

//como se ve con el tipo de cuarto
//aqui se colocan los cuartos secretos, tiendas, items, etc.
//4 0 0 0
//1 5 2 1

//como se ve en completo
// 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
// 1       1       1               1
// 1   T                           1
// 1       1       1               1
// 1 1 1 1 1 1 1 1 1 1   1 1 1 1 1 1
// 1 1 1 1 1       1       1 1 1 1 1
// 1 1 1 1 1   X   1   B   1 1 1 1 1
// 1 1 1 1 1       1       1 1 1 1 1
// 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 

*/

//tamaño de cuarto
const int NOTHING = 0;  //espacio libre
const int SIMPLE = 1;   // cuarto de 1x1
const int LARGE = 2;    // cuarto de 2x2
const int WIDE = 3;     // cuarto de 2x1
const int TALL = 4;     // cuarto de 1x2

const float COST_S = 1.0f; //Coste de los cuartos
const float COST_L = 2.0f;
const float COST_W = 1.5f;
const float COST_T = 1.5f;

const int Dy[] = {-1,1,0,0};    //direcciones aleatorias
const int Dx[] = {0,0,-1,1};



int M = 7, N = 13; //ALTO Y ANCHO

struct Room{
    pair<int, int> pos;     // Posicion (fila, columna)
    int size;               // Tamaño de cuarto (SIMPLE, LARGE, WIDE, TALL)
    int h;                  // Profundidad del cuarto (distancia desde el inicio)
    int type;               // Tipo de cuarto (BOSS, ITEM, SHOP, etc.)
    int id;                 // ID único para la sala (para BFS y conteo de conexiones)

    int width;              // Ancho real del cuarto
    int height;             // Alto real del cuarto

    int connections;        // Número de conexiones a otras salas

    // Constructor para inicializar
    Room(pair<int, int> p, int s, int room_id) : pos(p), size(s), h(0), type(EMPTY), id(room_id), connections(0) {
        switch (s) { // Use 's' for size, not 'roomSize'
            case SIMPLE: height = 1; width = 1; break;
            case LARGE:  height = 2; width = 2; break;
            case WIDE:   height = 1; width = 2; break;
            case TALL:   height = 2; width = 1; break;
            default: height = 0; width = 0; break; // Should be unreachable
        }
    }
};

vector<Room> allRoomsMade; //todos los cuartos hechos
Map roomIdMap(M, std::vector<int>(N, 0));              //mapa de ID
int currentIdRoom = 1;      //id actual del cuarto
vector<Room> finalRooms;    //cuartos finales

//Si esta dentro de los limites
bool isInLimits(int X, int Y){
    if(Y >= 0 && Y < M && X >= 0 && X < N){ return true; }
    return false;
}



//verifica si esta libre el espacio
bool canPlaceRoom(const Map& map, int y, int x, int roomType, int M, int N){
    int width = 0, height = 0; //alto y ancho del cuarto

    switch (roomType){
        case SIMPLE: height = 1; width = 1; break;
        case LARGE:  height = 2; width = 2; break;
        case WIDE:   height = 1; width = 2; break;
        case TALL:   height = 2; width = 1; break;
        default: return false;
    }

    //si esta dentro de los limites
    if(!isInLimits(x,y) || !isInLimits(x + width - 1, y + height -1) ){ return false; }

    //revisa si hay algo en el mapa
    for(int i = y; i < y + height; i++){
        for(int j = x; j < x + width; j++){
            if(map[i][j] != NOTHING){ return false;}
        }
    }
    return true; //se puede colocar el cuarto
}

//Colocar el cuarto
void placeRoom(Map& map, int y, int x, int roomType){
    int width = 0, height = 0; //alto y ancho del cuarto

    switch (roomType){
        case SIMPLE: height = 1; width = 1; break;
        case LARGE:  height = 2; width = 2; break;
        case WIDE:   height = 1; width = 2; break;
        case TALL:   height = 2; width = 1; break;
    }

    Room nextRoom({y,x}, roomType, currentIdRoom++);    //se crea nuevo cuarto(constructor)
    allRoomsMade.push_back(nextRoom);                   //se añade al allroomsmade

    //revisa si hay algo en el mapa
    for(int i = y; i < y + height; i++){
        for(int j = x; j < x + width; j++){
            map[i][j] = roomType;
            roomIdMap[i][j] = nextRoom.id;   //
        }
    }
}

//costo del cuarto
float getRoomCost(int roomType) {
    switch (roomType) {
        case SIMPLE:    return COST_S;
        case LARGE:     return COST_L;
        case WIDE:      return COST_W;
        case TALL:      return COST_T;
        default: return 0.0f;
    }
}

//imprimir el mapa casual
void printMap(const Map& map){
    std::cout << "--- Current Map ---" << std::endl;
    cout << setw(3);
    for (const auto& row : map) {
        for (int cell : row) {
            // Adapt this to represent your cells meaningfully (e.g., ' ' for empty, '#' for occupied).
            std::cout << cell << setw(3);
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

//genera el camino de cuartos(tamaño), es recursivo este
void generatePathRoomSize(Map& map, int currY, int currX, int currRmType, float& pointsLeft, int mapM, int mapN, int wSimple, int wLarge, int wWide, int wTall, float probBif){

    float currBifProb = probBif; //posible nueva bifurcacion

    float roomCost = getRoomCost(currRmType); //obtener el valor del cuarto actual
    //si no quedan puntos o 
    if( pointsLeft < roomCost || !canPlaceRoom(map, currY, currX, currRmType, mapM, mapN)){
        return; } //termina el camino

    //colocar el cuarto
    placeRoom(map,currY,currX, currRmType);
    roomIdMap[currY][currX] = currentIdRoom;
    
    pointsLeft -= roomCost;                 //se resta el puntaje
    
    int nExpansion = 1; //tipo de caminos a realizar
    if((float)rand() / RAND_MAX < probBif){
        probBif -= 0.1f;
            if((float)rand() / RAND_MAX > 0.5f){ nExpansion = 2;} //si es una o dos
            else{ nExpansion = 3; }
    }
    else{ probBif += 0.1f;}

    for(int i = 0; i < nExpansion; i++){
        if(pointsLeft <= 0){ break;} //si se queda sin puntos
    
        int dirId = rand() %4; //tipo de direccion a ir
        int nextY = currY + Dy[dirId];  //proximo Y
        int nextX = currX + Dx[dirId];  //proximo X

        int nextRoomType = SIMPLE;  //proximo tipo de ciarto
        int totalW = wSimple + wLarge + wWide + wTall; //total de posibilidades de cuarto
        int randomW = rand() % totalW;                  //rand cuarto

        //asignacion de cuarto
        if (randomW < wSimple) { nextRoomType = SIMPLE;}
        else if (randomW < wSimple + wLarge) { nextRoomType = LARGE; }
        else if (randomW < wSimple + wLarge + wWide) { nextRoomType = WIDE;}
        else { nextRoomType = TALL; }

        float nextRoomCost = getRoomCost(nextRoomType); //obtener precio del siguiente cuarto

        //proximo cuarto
        if(pointsLeft >= nextRoomCost && canPlaceRoom(map, nextY, nextX, nextRoomType, mapM, mapN)){
            generatePathRoomSize(map, nextY, nextX, nextRoomType, pointsLeft, mapM, mapN, wSimple, wLarge, wWide, wTall, probBif);
        }

    }
}

//FUNCION PRINCIPAL
Map map_RoomSize(int startY, int startX, float& points, int M, int N, int wSimple, int wLarge, int wWide, int wTall, float probBif){
    allRoomsMade.clear(); //se limpia
    currentIdRoom = 0;
    Map map(M, std::vector<int>(N, NOTHING)); //SE INSTANCIA EL MAPA

    //el primer intento, se hacen los mayores mapas
    generatePathRoomSize(map,startY, startX, SIMPLE, points, M, N, wSimple, wLarge, wWide,wTall, probBif);

    //PARA QUE USE LA MAYORIA DE PUNTOS
    int maxAttempts = 10;   //INTENTOS MAXIMOS
    int attempts = 0;       //INTENTOS ACTUALES

    while(points > COST_L && attempts < maxAttempts){
        int dirId = rand() % 4;
        int randX = Dx[dirId];
        int randY = Dy[dirId];

        //SI SE PUEDE COLOCAR
        if(canPlaceRoom(map, startY + randY, startX + randX, SIMPLE, M, N)){ //SE INTENTA DE NUEVO 
            generatePathRoomSize(map,startY + randY, startX + randX, SIMPLE, points, M, N, wSimple, wLarge, wWide,wTall, probBif); }
        attempts++;
    }
    return map;
}

//imprime el primer mapa
void printMapOne(const Map& map){
    for (const auto& row : map) {
        for (int cell : row) {
            // Usamos un simple símbolo para cada tipo de cuarto
            if (cell == NOTHING) cout << " . ";
            else if (cell == SIMPLE) cout << " S ";
            else if (cell == LARGE) cout << " L ";
            else if (cell == WIDE) cout << " W ";
            else if (cell == TALL) cout << " T ";
            else cout << " # "; // Para cualquier otro valor inesperado
        }
        cout << endl;
    }
}

//----------------------------------------------




























//regresa los cuartos cercanos
std::vector<Room*> getNearbyRooms(const std::vector<Room>& allRooms, int currentCellY, int currentCellX) {
    std::vector<Room*> nearbyRooms;
    std::vector<int> foundRoomIds; // Para evitar añadir la misma sala varias veces

    for (int i = 0; i < 4; ++i) {
        int nextY = currentCellY + Dy[i];
        int nextX = currentCellX + Dx[i];

        // Verifica que la coordenada del vecino esté dentro de los límites del mapa
        if (nextY >= 0 && nextY < M && nextX >= 0 && nextX < N) {
            int roomIdAtNeighbor = roomIdMap[nextY][nextX];

            if (roomIdAtNeighbor != NOTHING) { 

                bool alrAdd = false;
                for (int id : foundRoomIds) {
                    if (id == roomIdAtNeighbor) {
                        alrAdd = true;
                        break;
                    }
                }

                if (!alrAdd) {
                    // Busca el objeto Room correspondiente en allRoomsMade usando el ID
                    for (const Room& room : allRooms) { // Itera por referencia constante
                        if (room.id == roomIdAtNeighbor ) {
                            nearbyRooms.push_back(const_cast<Room*>(&room));
                            foundRoomIds.push_back(roomIdAtNeighbor); // Marca como añadido
                            break; // Ya encontramos el cuarto, pasa al siguiente vecino
                        }
                    }
                }
            }
        }
    }

    return nearbyRooms;
}


//si es un cuarto final
bool isFinalRoom(int Y, int X){
    vector<Room*> roomsnear = getNearbyRooms(allRoomsMade, Y, X); //obtener cuartos cercanos

    if(roomsnear.size() == 1){
        return true;
    }
    return false;
}

//si es un posible cuarto secreto
bool isPossibeSecret(int Y, int X){
    vector<Room*> roomsnear = getNearbyRooms(allRoomsMade, Y, X); //obtener cuartos cercanos

    if(roomsnear.size() >= 2){
        return true;
    }
    return false;
}

//heuristica
float heuristic(int startX, int startY, int endX, int endY){
    return sqrt(pow((endX - startX),2) + pow((endY- startY),2));
}

//crear el mapa
Map roomTypeMap(const Map& firstMap, int M, int N, int startY, int startX, float probItem, float probShop, float probSecret){

    vector<Room> finalRooms; //cuartos que no tienen nada alrededor suyo
    Map map(M, std::vector<int>(N, UNAVAILABLE)); //se crea un segundo mapa

    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            if(firstMap[i][j] != NOTHING){ map[i][j] = EMPTY;} //se colocan los cuartos
        }
    }

    //si es cuarto final, es decir si solo tiene un cuarto alrededor
    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            if(firstMap[i][j] == SIMPLE && isFinalRoom(i,j)){
                Room fRoom({i,j}, firstMap[i][j],roomIdMap[i][j]); //se crea nuevo cuarto
                finalRooms.push_back(fRoom);                        //se agrega a la lista
            }
        }
    }

    //si no hay cuartos finales
    if(finalRooms.size() < 1){
        cout << "Mapa no aceptable" << endl;
        return map;
    }

    //JEFE------------------------------------------------------------
    //buscar el camino mas lejos.
    float maxH = 0;
    Room* bossRoom = nullptr;

   for (Room& r : allRoomsMade) { // Iterate by reference to modify the original object
        if (firstMap[r.pos.first][r.pos.second] == SIMPLE && (r.pos.first != startY || r.pos.second != startX)) {

            if (isFinalRoom(r.pos.first, r.pos.second)) {
                float current_h = heuristic(startX, startY, r.pos.second, r.pos.first);
                if (current_h > maxH) {
                    maxH = current_h;
                    bossRoom = &r; // Store pointer to the actual Room object
                }
            }
        }
    }

    if (bossRoom) {
        bossRoom->type = BOSS; // Assign the type to the actual room object

        map[bossRoom->pos.first][bossRoom->pos.second] = BOSS;
    }

    //ITEM Y TIENDA------------------------------------------------------------
    int itemAmount = 0;
    int shopAmount = 0;
    int attempts = 0;
    int nAtt = 50;

    //cantidad de intentos
    while (attempts < nAtt) {
        for (Room& r : allRoomsMade) { // Iterate by reference

            if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE && isFinalRoom(r.pos.first, r.pos.second)) {

                // If it's a final room and not the boss room
                if (&r != bossRoom && r.type != BOSS) { // Compare addresses to ensure it's not the boss room

                    float probItemRand = (float)rand() / RAND_MAX;
                    if (probItemRand < probItem && itemAmount < 3) { // Limit max items
                        r.type = ITEM;
                        itemAmount++;
                    }

                    float probShopRand = (float)rand() / RAND_MAX;
                    if (probShopRand < probShop && shopAmount < 2) { // Limit max shops
                        r.type = SHOP;
                        shopAmount++;
                    }
                }
            }
        }
        attempts++;
    }

    // SECRETO------------------------------------------------------------
    int secretAmount = 0;
    attempts = 0;
    nAtt = 20;
    while (attempts < nAtt) {
        for (Room& r : allRoomsMade) {
            std::vector<std::pair<int, int>> potentialSecretCoords;
            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < N; ++j) {
                    if (firstMap[i][j] == NOTHING && isPossibeSecret(i, j)) {
                        potentialSecretCoords.push_back({i, j});
                    }
                }
            }

            // Now, try to place secret rooms in these potential spots
            if (!potentialSecretCoords.empty()) {
                for (const auto& coord : potentialSecretCoords) {
                    int y = coord.first;
                    int x = coord.second;

                    if (map[y][x] == UNAVAILABLE) { //si esta disponible el cuarto
                        float probSecretRand = (float)rand() / RAND_MAX;
                        if (probSecretRand < probSecret && secretAmount < 10) { //limitar el maximo de cuartos


                            Room secretRoom({y, x}, SIMPLE, currentIdRoom++); //nuevo ID, nuevo 
                            secretRoom.type = SECRET;
                            allRoomsMade.push_back(secretRoom); // Add to allRoomsMade

                            // Mark the map for the secret room
                            map[y][x] = SECRET;
                            secretAmount++;
                        }
                    }
                }
            }
        attempts++;
        }
    }

    //A rehacer
    for (const Room& r : allRoomsMade) {
        for (int i = 0; i < r.height; ++i) {
            for (int j = 0; j < r.width; ++j) {
                // Ensure the coordinates are within map bounds before access
                if (isInLimits(r.pos.second + j, r.pos.first + i)) {
                    if (map[r.pos.first + i][r.pos.second + j] == EMPTY ||
                        map[r.pos.first + i][r.pos.second + j] == UNAVAILABLE ||
                        r.type == BOSS || r.type == ITEM || r.type == SHOP) {
                        map[r.pos.first + i][r.pos.second + j] = r.type;
                    }
                }
            }
        }
    }

    //al final se regresa el mapa
    return map;
}


//----------------------------------------------

//g++ Tarea4PCG.cpp -o wow
//./wow
int main(){

    srand(time(nullptr));

    int sY = (M/2); //pos inicial en x
    int sX = (N/2); //pos incial en y

    
    //PROBABILIDADES
    float sRoom = 0.5f; //probabilidad secreto
    float iRoom = 0.5f; //probabilidad items;

    int probSimple = 60;    //prob simple
    int probLarge = 5;      //prob large
    int probWide = 10;      //prob wide
    int probTall = 10;      //prob tall

    int lvl = 2;            //nivel en el que esta
    float points = 5;       //puntos iniciales

    float probItem = 0.5f;  //probabilidad de Item
    int maxItem = 3;        //maximo de items

    float probShop = 0.8f;  //probabilidad de Tienda
    int maxShop = 3;        //maximo de tiendas

    float probSecret = 0.3f;//probabilidad de Secreto
    int maxSecret = 3;      //maximo de secretos

    points += (rand() % 3); //randomizado
    points += (lvl*2.6f);   //puntaje basado en el nivel
    
    float probBifurcation = 0.7f;

    Map sizeMap = map_RoomSize(sY ,sX ,points, M, N, probSimple, probLarge, probWide, probTall, probBifurcation);
    Map typeMap = roomTypeMap(sizeMap, M, N, sY, sX, probItem, probShop, probSecret);

    
    cout << "---------------    MAPA SIZE   ---------------" << endl;
    cout << endl << "puntaje Extra: " << points << endl;
    printMap(sizeMap);
    /*
    cout << "---------------    MAPA ID     ---------------" << endl;
    printMap(roomIdMap);*/
    
    cout << "---------------    MAPA TYPE     ---------------" << endl;
    printMap(typeMap);
    //crear mapa con tamaño de cuartos
    //crear mapa con tipo de cuartos

    return 0;
}