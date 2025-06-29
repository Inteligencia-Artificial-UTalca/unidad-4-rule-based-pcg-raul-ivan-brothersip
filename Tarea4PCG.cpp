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
using Map = vector<vector<int>>;

/*
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


//para la representacion
const string V_DOOR =     "D";   //en medio de cada pared
const string V_WALL =     "#";   //lugar inaccesible
const string V_SPACE =    "-";   //
const string V_OBJ =      "I";   //item
const string V_GUY =      "S";   //para la tienda
const string V_MYSTERY =  "?";   //secreto
const string V_ENEMY =    "B";   //para el jefe    
const string V_SPIKE =    "V";   //para sacrificio
const string V_STATUE =   "P";   //para maldicion
const string V_KEEPER =   "K";   //para el super secreto
const string V_AWAY =     "E";   //salida secreta
const string V_START =    "@";  //comienzo

//tipo de cuarto, para imprimir el mapa
const int EMPTY = 0;        //cuarto vacio
const int UNAVAILABLE = 1;  //no disponible
const int BOSS = 2;         //jefe
const int ITEM = 3;         //item
const int SHOP = 4;         //tienda
const int SECRET = 5;       //secreto
const int HIDDEN = 6;       //super secreto
const int CURSE = 7;        //maldicion
const int SACRI = 8;        //cuarto de sacrificio
const int EXIT = 9;         //salida secreta

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


int M = 7, N = 13; //ALTO Y ANCHO DEL MAPA

////////////////////////////Estructura base de BSP////////////////////////////////////////////
struct Tallo { //delcaramos la estructura base del bsp que comtempla las coordenadas de inicio y las dimensiones de la region
    int x, y, width, height;
};
//declaramos la estructura de la hoja donde cada tallo contiene una region y gera subhojas
struct Hoja {
    Tallo region; //regiojn que representa que la hoja esta en el mapa
    Hoja* left = nullptr;//puntero de la subhoja izquierda
    Hoja* right = nullptr;//puntero de la subhoja derecha

    Hoja(Tallo r) : region(r) {} //constructor que inicializa la hoja con su region dada
};


/// Drunk/////////
struct Room{
    pair<int, int> pos;     // Posicion (fila, columna)
    int size;               // Tamaño de cuarto (SIMPLE, LARGE, WIDE, TALL)
    int type;               // Tipo de cuarto (BOSS, ITEM, SHOP, etc.)
    int id;                 // ID único para la sala (para BFS y conteo de conexiones)

    int width;              // Ancho real del cuarto
    int height;             // Alto real del cuarto

    // Constructor para inicializar
    Room(pair<int, int> p, int s, int room_id) : pos(p), size(s), type(EMPTY), id(room_id) {
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
Map roomIdMap(M, vector<int>(N, 0));              //mapa de ID
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
    cout << "--- Current Map ---" << endl;
    cout << setw(3);
    for (const auto& row : map) {
        for (int cell : row) {
            // Adapt this to represent your cells meaningfully (e.g., ' ' for empty, '#' for occupied).
            cout << cell << setw(3);
        }
        cout << endl;
    }
    cout << "-------------------" << endl;
}

//genera el camino de cuartos(tamaño), es recursivo este
void generatePathRoomSize(Map& map, int currY, int currX, int currRmType, float& pointsLeft,
                          int mapM, int mapN, int wSimple, int wLarge, int wWide, int wTall,
                          float probBif, int minY, int maxY, int minX, int maxX) {

    // Chequeo límites BSP
    if (currY < minY || currY >= maxY || currX < minX || currX >= maxX) return;

    float roomCost = getRoomCost(currRmType);

    if (pointsLeft < roomCost || !canPlaceRoom(map, currY, currX, currRmType, mapM, mapN)) {
        return;
    }

    // Verificar que la habitación cabe dentro de la región BSP
    int width = 0, height = 0;
    switch (currRmType){
        case SIMPLE: height = 1; width = 1; break;
        case LARGE:  height = 2; width = 2; break;
        case WIDE:   height = 1; width = 2; break;
        case TALL:   height = 2; width = 1; break;
    }
    if (currY + height > maxY || currX + width > maxX) return;

    placeRoom(map, currY, currX, currRmType);
    pointsLeft -= roomCost;

    int nExpansion = 1;
    if ((float)rand() / RAND_MAX < probBif) {
        probBif -= 0.1f;
        nExpansion = ((float)rand() / RAND_MAX > 0.5f) ? 2 : 3;
    } else {
        probBif += 0.1f;
    }

    for (int i = 0; i < nExpansion; i++) {
        if (pointsLeft <= 0) break;

        int dirId = rand() % 4;
        int nextY = currY + Dy[dirId];
        int nextX = currX + Dx[dirId];

        int nextRoomType = SIMPLE;
        int totalW = wSimple + wLarge + wWide + wTall;
        int randomW = rand() % totalW;
        if (randomW < wSimple) nextRoomType = SIMPLE;
        else if (randomW < wSimple + wLarge) nextRoomType = LARGE;
        else if (randomW < wSimple + wLarge + wWide) nextRoomType = WIDE;
        else nextRoomType = TALL;

        float nextRoomCost = getRoomCost(nextRoomType);
        if (pointsLeft >= nextRoomCost && canPlaceRoom(map, nextY, nextX, nextRoomType, mapM, mapN)) {
            // Asegurarse que el próximo paso está dentro de la región BSP
            if (nextY >= minY && nextY < maxY && nextX >= minX && nextX < maxX) {
                generatePathRoomSize(map, nextY, nextX, nextRoomType, pointsLeft,
                                     mapM, mapN, wSimple, wLarge, wWide, wTall, probBif,
                                     minY, maxY, minX, maxX);
            }
        }
    }
}


//FUNCION PRINCIPAL
Map map_RoomSize(int startY, int startX, float& points, int M, int N, int wSimple, int wLarge, int wWide, int wTall, float probBif){
    allRoomsMade.clear(); //se limpia
    currentIdRoom = 0;
    Map map(M, vector<int>(N, NOTHING)); //SE INSTANCIA EL MAPA

    //el primer intento, se hacen los mayores mapas
    generatePathRoomSize(map,startY, startX, SIMPLE, points, M, N, wSimple, wLarge, wWide,wTall, probBif,0,M,0,N); //le pasamos los 4 argumentos extra al integrar bsp

    //PARA QUE USE LA MAYORIA DE PUNTOS
    int maxAttempts = 10;   //INTENTOS MAXIMOS
    int attempts = 0;       //INTENTOS ACTUALES

    while(points > COST_L && attempts < maxAttempts){
        int dirId = rand() % 4;
        int randX = Dx[dirId];
        int randY = Dy[dirId];

        //SI SE PUEDE COLOCAR
        if(canPlaceRoom(map, startY + randY, startX + randX, SIMPLE, M, N)){ //SE INTENTA DE NUEVO 
            generatePathRoomSize(map,startY + randY, startX + randX, SIMPLE, points, M, N, wSimple, wLarge, wWide,wTall, probBif,0,M,0,N); } //le pasamos los 4 argumentos extra al integrar bsp
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

//---------------------------------------------


//regresa los cuartos cercanos
vector<Room*> getNearbyRooms(const vector<Room>& allRooms, int currentCellY, int currentCellX) {
    vector<Room*> nearbyRooms;     //para añadir los cuartos
    vector<int> foundRoomIds; // Para evitar añadir la misma sala varias veces

    for (int i = 0; i < 4; ++i) {
        int nextY = currentCellY + Dy[i];
        int nextX = currentCellX + Dx[i];

        // Verifica que la coordenada del vecino esté dentro de los límites del mapa
        if (nextY >= 0 && nextY < M && nextX >= 0 && nextX < N) {
            int roomIdAtNeighbor = roomIdMap[nextY][nextX];

            if (roomIdAtNeighbor != NOTHING) {      //si esta vacio el mapa

                bool alrAdd = false;
                for (int id : foundRoomIds) {
                    if (id == roomIdAtNeighbor) {
                        alrAdd = true;
                        break;
                    }
                }

                if (!alrAdd) {
                    // Busca el objeto Room correspondiente en allRoomsMade usando el ID
                    for (const Room& room : allRooms) {                 // por cada cuarto
                        if (room.id == roomIdAtNeighbor ) {
                            nearbyRooms.push_back(const_cast<Room*>(&room));
                            foundRoomIds.push_back(roomIdAtNeighbor);   // Marca como añadido
                            break;                                      // Ya encontramos el cuarto, pasa al siguiente vecino
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

/*
//crear el mapa con la especialidad del cuarto( version 1)
Map roomTypeMap(Map& firstMap, int M, int N, int startY, int startX,
    float probItem, float probShop, float probSecret, float probHidden,
    int maxItem, int maxShop, int maxSecret, float probCurse, float probSacri){

    vector<Room> finalRooms; //cuartos que no tienen nada alrededor suyo
    Map map(M, vector<int>(N, UNAVAILABLE)); //se crea un segundo mapa

    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            if(firstMap[i][j] != NOTHING){ map[i][j] = EMPTY;} //se colocan los cuartos del primer mapa
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
        map[bossRoom->pos.first][bossRoom->pos.second] = BOSS;      //se asigna el valor
    }

    //ITEM Y TIENDA------------------------------------------------------------
    int itemAmount = 0;     //cantidad de items
    int shopAmount = 0;     //cantidad de tiendas
    int attempts = 0;       //intentos
    int nAtt = 20;          //numero de intentos maximo

    //cantidad de intentos
    while (attempts < nAtt) {
        for (Room& r : allRoomsMade) { //todos los cuartos

            if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE && isFinalRoom(r.pos.first, r.pos.second)) { //si es un cuarto simple, es final y esta vacio
                if (&r != bossRoom && r.type != BOSS) { // Si no es un nivel de jefe

                    float probItemRand = (float)rand() / RAND_MAX;      //probabilidad
                    if (probItemRand < probItem && itemAmount < maxItem) { // Limite max items
                        r.type = ITEM;
                        itemAmount++;
                    }

                    float probShopRand = (float)rand() / RAND_MAX;
                    if (probShopRand < probShop && shopAmount < maxShop) { // Limite max shops
                        r.type = SHOP;
                        shopAmount++;
                    }
                }
            }
        }
        attempts++;
    }

    // SECRETO------------------------------------------------------------
    int secretAmount = 0;   //cantidad de secretos
    attempts = 0;           //se reinicia la cantidad de intentos, ademas de modificalro
    nAtt = 10;
    while (attempts < nAtt) {
        for (Room& r : allRoomsMade) {
            vector<pair<int, int>> potentialSecretCoords; //posibles cuartos, coordenadas

            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < N; ++j) {
                    if (firstMap[i][j] == NOTHING && isPossibeSecret(i, j)) {
                        potentialSecretCoords.push_back({i, j});        //se añade la posicion
                    }
                }
            }

            //se agregan el posible cuarto, si hay posiciones
            if (!potentialSecretCoords.empty()) {
                for (const auto& coord : potentialSecretCoords) {
                    int y = coord.first;
                    int x = coord.second;

                    if (map[y][x] == UNAVAILABLE) { //si esta disponible el cuarto
                        float probSecretRand = (float)rand() / RAND_MAX;

                        if (probSecretRand < probSecret && secretAmount < maxSecret) { //limitar el maximo de cuartos
                            Room secretRoom({y, x}, SIMPLE, currentIdRoom++); //nuevo ID
                            secretRoom.type = SECRET;
                            allRoomsMade.push_back(secretRoom); // añade a allroomsmade

                            // se agrega a los mapas
                            firstMap[y][x] = SIMPLE;
                            roomIdMap[y][x] = secretRoom.id;
                            map[y][x] = SECRET;
                            secretAmount++;
                        }
                    }
                }
            }
        attempts++;
        }
    }

    
    //super secreto se debe ir por el cuarto anterior al del jefe, de ahi se crea.

    //sacrificio y maldicion son aleatorios, igual que los secretos, pero se intenta una sola vez

    //A rehacer el mapa
    for (const Room& r : allRoomsMade) {
        for (int i = 0; i < r.height; ++i) {
            for (int j = 0; j < r.width; ++j) {
                //si esta en los limites
                if (isInLimits(r.pos.second + j, r.pos.first + i)) {
                    //si es un cuarto con tipo de cosa
                    if (map[r.pos.first + i][r.pos.second + j] == EMPTY ||
                        map[r.pos.first + i][r.pos.second + j] == UNAVAILABLE ||
                        r.type == BOSS || r.type == ITEM || r.type == SHOP || r.type == HIDDEN) {
                        map[r.pos.first + i][r.pos.second + j] = r.type;
                    }
                }
            }
        }
    }

    //al final se regresa el mapa

    // === MALDICIÓN ===
bool placedCurse = false;
if ((float)rand() / RAND_MAX < probCurse) {
    for (Room& r : allRoomsMade) {
        if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE) {
            r.type = CURSE;
            map[r.pos.first][r.pos.second] = CURSE;
            placedCurse = true;
            break;
        }
    }
}

// === SACRIFICIO ===
bool placedSacri = false;
if ((float)rand() / RAND_MAX < probSacri) {
    for (Room& r : allRoomsMade) {
        if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE) {
            r.type = SACRI;
            map[r.pos.first][r.pos.second] = SACRI;
            placedSacri = true;
            break;
        }
    }
}
    
    return map;
}
*/

//crear el mapa con la especialidad del cuarto( version 2)
//se quito la funcion isFinalRoom(i,j) ya que era muy restrictivo
Map roomTypeMap(Map& firstMap, int M, int N, int startY, int startX,
    float probItem, float probShop, float probSecret, float probHidden,
    int maxItem, int maxShop, int maxSecret, float probCurse, float probSacri){

    vector<Room> finalRooms; //cuartos que no tienen nada alrededor suyo
    Map map(M, vector<int>(N, UNAVAILABLE)); //se crea un segundo mapa

    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            if(firstMap[i][j] != NOTHING){ map[i][j] = EMPTY;} //se colocan los cuartos del primer mapa
        }
    }

    //si es cuarto simple
    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            if(firstMap[i][j] == SIMPLE){ //se elimina isFinalRoom
                Room fRoom({i,j}, firstMap[i][j],roomIdMap[i][j]); //se crea nuevo cuarto
                finalRooms.push_back(fRoom);                        //se agrega a la lista
            }
        }
    }

    //si no hay cuartos
    if(finalRooms.size() < 1){
        cout << "Mapa no aceptable" << endl;
        return map;
    }

    //JEFE------------------------------------------------------------
    //buscar el camino mas lejos.
    float maxH = 0;
    Room* bossRoom = nullptr;

    for (Room& r : allRoomsMade) {
        if (firstMap[r.pos.first][r.pos.second] == SIMPLE && (r.pos.first != startY || r.pos.second != startX)) {
            float current_h = heuristic(startX, startY, r.pos.second, r.pos.first);
            if (current_h > maxH) {
                maxH = current_h;
                bossRoom = &r;
            }
        }
    }

    if (bossRoom) {
        bossRoom->type = BOSS; //se asigna el tipo al objeto real
        map[bossRoom->pos.first][bossRoom->pos.second] = BOSS; //se asigna el valor
    }

    //ITEM Y TIENDA------------------------------------------------------------
    int itemAmount = 0;     //cantidad de items
    int shopAmount = 0;     //cantidad de tiendas
    int attempts = 0;       //intentos
    int nAtt = 20;          //numero de intentos maximo

    //cantidad de intentos
    while (attempts < nAtt) {
        for (Room& r : allRoomsMade) { //todos los cuartos

            if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE) { //si es un cuarto simple y esta vacio
                if (&r != bossRoom && r.type != BOSS) { //si no es el cuarto del jefe

                    float probItemRand = (float)rand() / RAND_MAX; //probabilidad
                    if (probItemRand < probItem && itemAmount < maxItem) { //limite max items
                        r.type = ITEM;
                        itemAmount++;
                    }

                    float probShopRand = (float)rand() / RAND_MAX;
                    if (probShopRand < probShop && shopAmount < maxShop) { //limite max shops
                        r.type = SHOP;
                        shopAmount++;
                    }
                }
            }
        }
        attempts++;
    }

    //SECRETO------------------------------------------------------------
    int secretAmount = 0;   //cantidad de secretos
    attempts = 0;           //se reinicia la cantidad de intentos
    nAtt = 10;
    while (attempts < nAtt) {
        for (Room& r : allRoomsMade) {
            vector<pair<int, int>> potentialSecretCoords; //posibles cuartos, coordenadas

            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < N; ++j) {
                    if (firstMap[i][j] == NOTHING && isPossibeSecret(i, j)) {
                        potentialSecretCoords.push_back({i, j}); //se añade la posicion
                    }
                }
            }

            //se agregan el posible cuarto, si hay posiciones
            if (!potentialSecretCoords.empty()) {
                for (const auto& coord : potentialSecretCoords) {
                    int y = coord.first;
                    int x = coord.second;

                    if (map[y][x] == UNAVAILABLE) { //si esta disponible el cuarto
                        float probSecretRand = (float)rand() / RAND_MAX;

                        if (probSecretRand < probSecret && secretAmount < maxSecret) { //limitar el maximo de cuartos
                            Room secretRoom({y, x}, SIMPLE, currentIdRoom++); //nuevo ID
                            secretRoom.type = SECRET;
                            allRoomsMade.push_back(secretRoom); //añade a allRoomsMade

                            //se agrega a los mapas
                            firstMap[y][x] = SIMPLE;
                            roomIdMap[y][x] = secretRoom.id;
                            map[y][x] = SECRET;
                            secretAmount++;
                        }
                    }
                }
            }
            attempts++;
        }
    }

    //A rehacer el mapa
    for (const Room& r : allRoomsMade) {
        for (int i = 0; i < r.height; ++i) {
            for (int j = 0; j < r.width; ++j) {
                //si esta en los limites
                if (isInLimits(r.pos.second + j, r.pos.first + i)) {
                    //si es un cuarto con tipo de cosa
                    if (map[r.pos.first + i][r.pos.second + j] == EMPTY ||
                        map[r.pos.first + i][r.pos.second + j] == UNAVAILABLE ||
                        r.type == BOSS || r.type == ITEM || r.type == SHOP || r.type == HIDDEN) {
                        map[r.pos.first + i][r.pos.second + j] = r.type;
                    }
                }
            }
        }
    }

    //=== MALDICION ===
    bool placedCurse = false;
    if ((float)rand() / RAND_MAX < probCurse) {
        for (Room& r : allRoomsMade) {
            if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE) {
                r.type = CURSE;
                map[r.pos.first][r.pos.second] = CURSE;
                placedCurse = true;
                break;
            }
        }
    }

    //=== SACRIFICIO ===
    bool placedSacri = false;
    if ((float)rand() / RAND_MAX < probSacri) {
        for (Room& r : allRoomsMade) {
            if (r.type == EMPTY && firstMap[r.pos.first][r.pos.second] == SIMPLE) {
                r.type = SACRI;
                map[r.pos.first][r.pos.second] = SACRI;
                placedSacri = true;
                break;
            }
        }
    }

    return map;
}




//hacer el ultimo mapa, el visual
void printFinalMap(const Map& sizeMap, const Map& typeMap, int startY, int startX){

    // Each logical cell (1x1) expands to 4 visual units (3 for interior + 1 for wall/door).
    int visualM = (M * 4) + 1;
    int visualN = (N * 4) + 1;

    //crear un mapa lleno de paredes
    vector<vector<string>> visualGrid(visualM, vector<string>(visualN, V_WALL));


    //primero el cuarto vacio
    for (const Room& r : allRoomsMade) {

        int stFila = r.pos.first * 4;   //comienzo de la fila
        int stCol = r.pos.second * 4;   //comienzo de la columna

        // dimension del cuarto
        int visRoomH = r.height * 4; // e.g., 1x1 room is 3, 2x1 room is 7
        int visRoomW = r.width * 4;  // e.g., 1x1 room is 3, 1x2 room is 7

        // crear cuarto basico (3x3)
        for (int i = 1; i < visRoomH; ++i) {
            for (int j = 1; j < visRoomW; ++j) {
                if (stFila + i < visualM && stCol + j < visualN) {
                    visualGrid[stFila + i][stCol + j] = V_SPACE;
                }
            }
        }

        //mitad del cuarto
        int charY = stFila + 2;
        int charX = stCol + 2;

        //asignamos valor al medio del cuarto
        if (charY < visualM && charX < visualN) {
            if (r.type == BOSS) visualGrid[charY][charX] = V_ENEMY;
            else if (r.type == ITEM) visualGrid[charY][charX] = V_OBJ;
            else if (r.type == SHOP) visualGrid[charY][charX] = V_GUY;
            else if (r.type == SECRET) visualGrid[charY][charX] = V_MYSTERY;
            else if (r.type == HIDDEN) visualGrid[charY][charX] = V_KEEPER;
            else if (r.type == CURSE) visualGrid[charY][charX] = V_STATUE;
            else if (r.type == SACRI) visualGrid[charY][charX] = V_SPIKE;
            else if (r.type == EXIT) visualGrid[charY][charX] = V_AWAY;
            else if (r.pos.first == startY && r.pos.second == startX){
                visualGrid[charY][charX] = V_START;
            }
        }
    }

    // PUERTAS
    for (const Room& r : allRoomsMade) {
        

        for (int cell_y_offset = 0; cell_y_offset < r.height; ++cell_y_offset) {
            for (int cell_x_offset = 0; cell_x_offset < r.width; ++cell_x_offset) {

                int currY = r.pos.first + cell_y_offset;
                int currX = r.pos.second + cell_x_offset;

                // coordenadas para el centro de las puertas
                int baseY = currY * 4;
                int baseX = currX * 4;

                //posibles vecinas para cada cuarto
                for (int i = 0; i < 4; ++i) { // 0:Up, 1:Down, 2:Left, 3:Right
                    int nextY = currY + Dy[i];
                    int nextX = currX + Dx[i];

                    //dentro de limites y sea un cuarto
                    if (isInLimits(nextX, nextY) && sizeMap[nextY][nextX] != NOTHING) {
                        //verificar que el cuarto sea distinto al siguiente

                        if (roomIdMap[nextY][nextX] != r.id) {
                            // poner la puerta

                            int doorY = baseY;
                            int doorX = baseX;

                            if (i == 0) { // Up
                                doorX += 2;
                                if (doorY >= 0 && doorX >= 0 && doorY < visualM && doorX < visualN){
                                    visualGrid[doorY][doorX] = V_DOOR;}
                            } else if (i == 1) { // Down
                                doorY += 4;
                                doorX += 2;
                                if (doorY >= 0 && doorX >= 0 && doorY < visualM && doorX < visualN){
                                    visualGrid[doorY][doorX] = V_DOOR;}
                                    
                            } else if (i == 2) { // Left
                                doorY += 2;
                                if (doorY >= 0 && doorX >= 0 && doorY < visualM && doorX < visualN){
                                    visualGrid[doorY][doorX] = V_DOOR;
                                }
                            } else if (i == 3) { // Right
                                doorY += 2;
                                doorX += 4;
                                if (doorY >= 0 && doorX >= 0 && doorY < visualM && doorX < visualN){
                                    visualGrid[doorY][doorX] = V_DOOR;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    // hacer el mapa
    cout << endl << "---------------    MAPA TYPE     ---------------" << endl;
    for (int i = 0; i < visualM; ++i) {
        for (int j = 0; j < visualN; ++j) {
            cout << visualGrid[i][j] << " ";
        }
        cout << endl;
    }
    cout << "-------------------\n" << endl;
}

//////////////Algoritmo de BSP////////////////////////////////////////////
//funcion de bsp que  se encargara de dividir las hojas si es posible en dos subhojas
bool splitHoja(Hoja* hoja, int minSize) {
    if (hoja->left || hoja->right) return false; //si la hoja ya fue dividida no hace nada
    //decidimos de forma aleatoria si dividirla de forma horizontal o vertical
    bool split = (rand() % 2);
    //ajustamos los cortes para el balance
    if (hoja->region.width > hoja->region.height &&
        hoja->region.width / hoja->region.height >= 1.25)
        split = false;//corte vertical
    else if (hoja->region.height > hoja->region.width &&
             hoja->region.height / hoja->region.width >= 1.25)
        split = true;//corte horizontal

     //calculamos el valor maximo para la division   
    int max = (split ? hoja->region.height : hoja->region.width) - minSize;
    if (max <= minSize) return false;
        //elegimos el punto de division de forma aleatoria
    int SSplit = minSize + rand() % (max - minSize + 1);
        //por ultimo creamos dos hojas nuevas segun la division que se hizo(vertical o horizontal)
    if (split) {//horizontal
        hoja->left  = new Hoja({hoja->region.x, hoja->region.y, hoja->region.width, SSplit});
        hoja->right = new Hoja({hoja->region.x, hoja->region.y + SSplit, hoja->region.width, hoja->region.height - SSplit});
    } else {//vertical
        hoja->left  = new Hoja({hoja->region.x, hoja->region.y, SSplit, hoja->region.height});
        hoja->right = new Hoja({hoja->region.x + SSplit, hoja->region.y, hoja->region.width - SSplit, hoja->region.height});
    }

    return true;
}
//corazon del sistema ya que manejara la logica de bsp para dividir las hojas
void BSP(Map& map, int maxLeaves, int minSize,
         int wSimple, int wLarge, int wWide, int wTall, float probBif) {
    std::vector<Hoja*> leaves; //vector que almacena las hojas actuales del algoritmo
    Hoja* raiz = new Hoja({0, 0, (int)map[0].size(), (int)map.size()}); //hoja raiz que cubre todo el mapa
    leaves.push_back(raiz);

    bool haysplit = true; //verificacion para ver si se dividio 
    while (haysplit && leaves.size() < (size_t)maxLeaves) {
        haysplit = false; //verificacion por si no hay mas divisiones 
        for (size_t i = 0; i < leaves.size(); i++) {
            Hoja* l = leaves[i];
            if (!l->left && !l->right) {
                if (splitHoja(l, minSize)) {//si la division se logro se añaden nuevas hojas al vector
                    leaves.push_back(l->left);
                    leaves.push_back(l->right);
                    haysplit = true; //indicamos si hubo divisiones en cada nivel 
                }
            }
        }
    }

    // Por cada hoja BSP generamos Drunk Agent dentro de esa región
    float points = 5.0f;  // o se ajusta segun se requiera(es la probabilidad que maneja la cantidad de cuartos que se generan)
    for (Hoja* leaf : leaves) {
        generatePathRoomSize(map,
                             leaf->region.y + leaf->region.height / 2, //posicion y de la hoja
                             leaf->region.x + leaf->region.width / 2,  //posicion x de la hoja 
                             SIMPLE, //habitacion inicial
                             points,
                             M, N,
                             wSimple, wLarge, wWide, wTall,
                             probBif,
                             leaf->region.y, leaf->region.y + leaf->region.height, //limites vertical de la hoja
                             leaf->region.x, leaf->region.x + leaf->region.width); //limite horizontal
    }
}


//MAIN----------------------------------------------

//g++ Tarea4PCG.cpp -o wow
//./wow
int main() {
    srand(time(nullptr));

    int sY = (M / 2); // pos inicial en y (fila)
    int sX = (N / 2); // pos inicial en x (columna)

    // PROBABILIDADES
    float sRoom = 0.5f;    // probabilidad secreto (no usado directamente aqui)
    float iRoom = 0.5f;    // probabilidad items (no usado directamente aqui)

    int probSimple = 60;   // peso o probabilidad para sala simple
    int probLarge = 5;     // peso o probabilidad para sala large
    int probWide = 10;     // peso o probabilidad para sala wide
    int probTall = 10;     // peso o probabilidad para sala tall

    int mode = 1;           // el modo de generacion (cambiar para cambiar la generacion)
    int lvl = 2;            // nivel actual
    float points = 100;     // puntos iniciales

    float probItem = 0.5f;  // probabilidad de Item
    int maxItem = 3;        // maximo de items

    float probShop = 0.8f;  // probabilidad de Tienda
    int maxShop = 1;        // maximo de tiendas

    float probSecret = 0.3f; // probabilidad de Secreto
    int maxSecret = 1;       // maximo de secretos

    // POR IMPLEMENTAR
    float probHidden = 0.3f; // probabilidad de super secreto
    float probCurse = 0.5f;  // probabilidad de maldicion 
    float probSacri = 0.5f;  // probabilidad de sacrificio 
    float probExit  = 0.1f;  // probabilidad de salida secreta (no usado aqui)

    switch (mode) {
    case 1:
        std::cout << "Modo 1: Muchas habitaciones grandes\n";
        points = 300;          // Más puntos = más habitaciones
        probSimple = 10;       // Pocos cuartos simples
        probLarge  = 40;       // Alta probabilidad para grandes
        probWide   = 30;
        probTall   = 30;
        break;
    case 2:
        std::cout << "Modo 2: Muchas habitaciones pequeñas\n";
        points = 350;          // Más puntos = más habitaciones
        probSimple = 85;       // Mayoría simples
        probLarge  = 5;        // Muy pocos grandes
        probWide   = 5;
        probTall   = 5;
        break;
    case 3:
        std::cout << "Modo 3: Generacion aleatoria\n";
        points = 50 + rand() % 251; // entre 50 y 300 puntos
        probSimple = rand() % 101;  // 0 a 100
        probLarge  = rand() % 51;   // 0 a 50
        probWide   = rand() % 51;
        probTall   = rand() % 51;
        break;
    default:
        std::cout << "Ingresa una opcion correcta\n";
        break;
}

    points += (rand() % 3);     // randomizado
    points += (lvl * 2.6f);     // puntaje basado en el nivel

    float probBifurcation = 0.7f;

    // ahora el llamado a map_RoomSize incluye limites internos en generatePathRoomSize
    Map sizeMap = map_RoomSize(sY, sX, points, M, N, probSimple, probLarge, probWide, probTall, probBifurcation);

    Map typeMap = roomTypeMap(sizeMap, M, N, sY, sX,
                              probItem, probShop, probSecret, probHidden,
                              maxItem, maxShop, maxSecret, probSacri, probCurse);

    cout << "---------------    MAPA SIZE   ---------------" << endl;
    cout << endl << "puntaje Extra: " << points << endl;
    printMap(sizeMap);

    cout << "---------------    MAPA ID     ---------------" << endl;
    printMap(roomIdMap);

    cout << "---------------    MAPA TYPE     ---------------" << endl;
    printMap(typeMap);

    // imprimir mapa final
    printFinalMap(sizeMap, typeMap, sY, sX);

    cout << "Leyenda\n";
    cout << "D : Puerta\n";
    cout << "# : Pared\n";
    cout << "- : Espacio\n";
    cout << "I : Item\n";
    cout << "S : Tienda\n";
    cout << "? : Secreto\n";
    cout << "B : Jefe\n";
    cout << "V : Sacrificio\n";
    cout << "P : Maldicion\n";
    cout << "K : Super Secreto\n";
    cout << "E : Salida Secreta\n";
    cout << "@ : Comienzo\n";

    return 0;
}
