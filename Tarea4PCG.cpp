#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator

// Define Map as a vector of vectors of integers.
//No olvidar g++ -std=c++11 -Wall -Wextra -pedantic Tarea4PCG.cpp -o isaac
// You can change 'int' to whatever type best represents your cells (e.g., char, bool).
using Map = std::vector<std::vector<int>>;
using namespace std;

//INPIRACIONES---------------------
// ISAAC
// NUCLEAR THRONE
// SPELUNKY
// ENTER THE GUNGEON
// SOUL KNIGHT
// HADES
// CULT OF THE LAMB


//drunk agent
//celullar automata

//matriz para representar el mapa
//arbol para poder determinar el camino


//GRILLA
// 0 = cuarto normal
// 1 = inaccsesible
// 2 = jefe
// 3 = item
// 4 = tienda
// 5 = cuarto secreto
// 6 = cuarto super secreto
// 7 = sacrificio
// 8 = maldicion
// 9 = salida secreta


// CUARTO
// 0 = nada
// 1 = paredes
// 2 = JEFE
// 3 = item
// 4 = tienda
// 5 = sacrificio
// 6 = maldicion
// 8 = entradas
// 9 = salida secreta

// PRIORIDAD
// cuarto secreto = esta al lado del final de un cuarto final. debe estar rodeado de min 2
// cuarto super secreto = al lado del (cuarto anterior del) jefe
// boss room = cuarto mas lejos
// tienda = cuartos finales
// items = cuartos finales

// HEURISTICA
// sacrificio
// maldicion

// PROBABILIDAD
// habitacion doble (probabilidad)
// salida secreta (probabilidad)


// 1 1 8 1 1
// 1 0 0 0 1
// 8 0 0 0 8
// 1 0 0 0 1
// 1 1 8 1 1 


//ROOM


void printMap(const Map& map) {
    
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


int main(){

    int M = 21, N = 15;

    int sM = (M/2) + 1;
    int sN = (N/2) + 1;

    //PROBABILIDADES
    float sRoom = 0.5f;

    int rTotal = 7; //cantidad de cuartos
    int itemT = 4; //cantidad de items

    Map myMap(M, vector<int>(N,0)); //mapa es insanciado


    // ramdom(0,2) + 5 + level*2.6 |||||||||||||  si 7.5 = 7



    return 0;
}