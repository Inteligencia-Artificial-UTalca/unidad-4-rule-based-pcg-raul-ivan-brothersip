#include <iostream>
#include <vector>
#include <random>   // Para random
#include <queue>    // para las colas
#include <chrono>   // para tener el seed para el random

// Define Map as a vector of vectors of integers.
//No olvidar g++ -std=c++11 -Wall -Wextra -pedantic Tarea4PCG.cpp -o isaac

using namespace std;
using Map = std::vector<std::vector<int>>;

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


//tamaño de cuarto
const int NOTHING = 0;  //espacio libre
const int SIMPLE = 1;   // cuarto de 1x1
const int LARGE = 2;    // cuarto de 2x2
const int WIDE = 3;     // cuarto de 2x1
const int TALL = 4;     // cuarto de 1x2

const float COST_S = 1.0f;
const float COST_L = 2.0f;
const float COST_W = 1.5f;
const float COST_T = 1.5f;

const int Dy[] = {-1,1,0,0};    //drecciones aleatorias
const int Dx[] = {0,0,-1,1};

struct Room{
    pair<int, int> pos; //posicion
    int size; //tamaño de cuarto

};

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
    if(y < 0 || x < 0 || (y + height) > M || (x + width) > N){ return false; }

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

    //revisa si hay algo en el mapa
    for(int i = y; i < y + height; i++){
        for(int j = x; j < x + width; j++){
            map[i][j] = roomType;
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

void printMap(const Map& map){
    std::cout << "--- Current Map ---" << std::endl;
    for (const auto& row : map) {
        for (int cell : row) {
            // Adapt this to represent your cells meaningfully (e.g., ' ' for empty, '#' for occupied).
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

//genera el camino de cuartos(tamaño), es recursivo este
void generatePathRoomSize(Map& map, int currY, int currX, int currRmType, float& pointsLeft, int mapM, int mapN, int wSimple, int wLarge, int wWide, int wTall, float probBif){

    float roomCost = getRoomCost(currRmType); //obtener el valor del cuarto actual
    //si no quedan puntos o 
    if( pointsLeft < roomCost || !canPlaceRoom(map, currY, currX, currRmType, mapM, mapN)){
        return; //termina el camino
    }

    //colocar el cuarto
    placeRoom(map,currY,currX,currRmType);
    pointsLeft -= roomCost;                 //se resta el puntaje
    
    int nExpansion = 1; //tipo de caminos a realizar
    if((float)rand() / RAND_MAX < probBif){
            if((float)rand() / RAND_MAX > 0.5f){ nExpansion = 2;} //si es una o dos
            else{ nExpansion = 3; }
    }

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


Map map_RoomSize(int startY, int startX, float& points, int M, int N, int wSimple, int wLarge, int wWide, int wTall, float probBif){

    Map map(M, std::vector<int>(N, NOTHING));

    //el primer intento, se hacen los mayores mapas
    generatePathRoomSize(map,startY, startX, SIMPLE, points, M, N, wSimple, wLarge, wWide,wTall, probBif);

    //COSA AHORA PARA PODER USAR TODOS LOS PUNTOS POSIBLES
    int maxAttempts = 50;   //INTENTOS MAXIMOS
    int attempts = 0;       //INTENTOS ACTUALES

    while(points > COST_L && attempts < maxAttempts){
        int randX = rand() % 3 - 1; //INTENTA EN UN RANGO
        int randY = rand() % 3 - 1;

        //SI SE PUEDE COLOCAR
        if(canPlaceRoom(map, startY + randY, startX + randX, SIMPLE, M, N)){
            generatePathRoomSize(map,startY + randY, startX + randX, SIMPLE, points, M, N, wSimple, wLarge, wWide,wTall, probBif); //SE INTENTA DE NUEVO
        }
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

//g++ Tarea4PCG.cpp -o wow
//./wow
int main(){

    srand(time(nullptr));

    int M = 7, N = 13; //ALTO Y ANCHO

    int sY = (M/2); //pos inicial en x
    int sX = (N/2); //pos incial en y

    
    //PROBABILIDADES
    float sRoom = 0.5f; //probabilidad secreto
    float iRoom = 0.0f; //probabilidad items;

    int probSimple = 50;
    int probLarge = 10;
    int probWide = 15;
    int probTall = 20;

    int lvl = 1;    //nivel en el que esta
    float points = 5;
    points += ((rand() % 3) + (lvl*2.6f)); //puntos para que gaste en cuartos
    float probBifurcation = 0.5f;

    Map myMap = map_RoomSize(sY ,sX ,points, M, N, probSimple, probLarge, probWide, probTall, probBifurcation);

    cout << endl << "puntaje que sobro: " << points << endl;

    printMap(myMap);
    //crear mapa con tamaño de cuartos
    //crear mapa con tipo de cuartos

    return 0;
}