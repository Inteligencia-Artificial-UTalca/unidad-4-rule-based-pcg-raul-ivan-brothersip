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



Map firstMap(int M, int N, int startY, int startX, float&points, float probBif, int w1, int w2, int w3, int w4)
{
    Map map(M, vector<int>(N,NOTHING)); //mapa es instanciado

    queue<Room> pointRooms; //lista

    Room startRoom = {{startY, startX},SIMPLE}; //primer cuarto es simple

    float initialCost = getRoomCost(startRoom.size);
    if (points >= initialCost && canPlaceRoom(map, startRoom.pos.first, startRoom.pos.second, startRoom.size, M, N)) {
        placeRoom(map, startRoom.pos.first, startRoom.pos.second, startRoom.size);  // se agrega
        points -= initialCost; // se quitan el puntaje
        pointRooms.push(startRoom); // se agrega a la cola
    }

    //loop inicial para la creacion
    while(points > 0 && !pointRooms.empty()){
        Room currRoom = pointRooms.front();
        pointRooms.pop();

        int nExp = 1; //posibles expansiones de cuartos
        //calcular posibilidad de qie sea bifurcacion
        if((float)rand() / RAND_MAX < probBif){
            if((float)rand() / RAND_MAX > 0.5f){ nExp = 2;} //si es una o dos
            else{ nExp = 3; }
        }

        for(int i = 0; i < nExp; i++){
            if(points <= 0){break;} //si se acabaron los puntos

            int dirId = rand() % 4; //ID de direccion
            int newY = currRoom.pos.first + Dy[dirId];
            int newX = currRoom.pos.second + Dx[dirId];

            int newRoomType = NOTHING;  //nuevo valor del tipo de cuarto

            //NUEVO METODO DE CALCULAR EL NUEVO TIPO DE CUARTO

            int totalNewType = w1 + w2 + w3 + w4;

            int randomWeight = rand() % totalNewType; //cual cuarto se

            // Determine room type based on weighted selection
            if (randomWeight < w1) { newRoomType = SIMPLE;}
            else if (randomWeight < w1 + w2) { newRoomType = LARGE; }
            else if (randomWeight < w1 + w2 + w3) { newRoomType = WIDE;}
            else { newRoomType = TALL; }

            float costNewRoom = getRoomCost(newRoomType); //nuevo coste del cuarto
            if(points >= costNewRoom && canPlaceRoom(map, newY, newX, newRoomType, M, N))
            {
                placeRoom(map, newY, newX, newRoomType);        //se agrega el nuevo cuarto
                points -= costNewRoom;                           //se quitan el puntaje
                pointRooms.push({{newY,newX}, newRoomType});    //se agrega a la cola
            }
        }
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


int main(){

    srand(time(nullptr));

    int M = 7, N = 13; //ALTO Y ANCHO

    int sM = (M/2); //pos inicial en x
    int sN = (N/2); //pos incial en y

    
    //PROBABILIDADES
    float sRoom = 0.5f; //probabilidad secreto
    float iRoom = 0.0f; //probabilidad items;

    int probSimple = 50;
    int probLarge = 10;
    int probWide = 15;
    int ProbTall = 20;

    int lvl = 2;    //nivel en el que esta
    float points = 5;
    points += ((rand() % 3) + (lvl*2.6f)); //puntos para que gaste en cuartos
    float probBifurcation = 0.3f;

    Map myMap = firstMap(M,N,sM,sN,points,probBifurcation, probSimple, probLarge, probWide, ProbTall);

    cout << endl << "puntaje que sobro: " << points << endl;

    printMap(myMap);
    //crear mapa con tamaño de cuartos
    //crear mapa con tipo de cuartos

    return 0;
}