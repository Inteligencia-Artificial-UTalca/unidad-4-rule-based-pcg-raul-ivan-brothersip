#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator

// Define Map as a vector of vectors of integers.
//No olvidar g++ -std=c++11 -Wall -Wextra -pedantic RuleBasedPCG.cpp -o test
// You can change 'int' to whatever type best represents your cells (e.g., char, bool).
using Map = std::vector<std::vector<int>>;


const int MAX = 1;
const int MIN = -1;

/**
 * @brief Prints the map (matrix) to the console.
 * @param map The map to print.
 */
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

int oneOrTwo(int a, int b);

Map randomizeMap (const Map& currentMap, int W, int H){

    Map newMap = currentMap;
    for(int a = 0; a < H; a++){
        
        for(int b = 0; b < W; b++){
            newMap[a][b] = oneOrTwo(0,1);
        }
    }

    return newMap;
}


/**
 * @brief Function to implement the Cellular Automata logic.
 * It should take a map and return the updated map after one iteration.
 * @param currentMap The map in its current state.
 * @param W Width of the map.
 * @param H Height of the map.
 * @param R Radius of the neighbor window (e.g., 1 for 3x3, 2 for 5x5).
 * @param U Threshold to decide if the current cell becomes 1 or 0.
 * @return The map after applying the cellular automata rules.
 */
Map cellularAutomata(const Map& currentMap, int W, int H, int R, double U) {
    Map newMap = currentMap; // Initially, the new map is a copy of the current one

    //recorrer el mapa
    for(int y = 0; y < H; y++){
        for(int x = 0; x < W; x++){
            int mapVecLive = 0; //vecinos que estan "vivos"

            //revisa alrededor de un "nodo"
            for(int dx = -R; dx <= R; dx++){
                for(int dy = -R; dy <= R; dy++){
                    if(dx == 0 && dy == 0){continue;} //si esta en el nodo que esta revisando

                    int vecX = x + dx; //vecino en x e y
                    int vecY = y + dy;

                    //si esta dentro de los limites
                    if(vecX >= 0 && vecX < W && vecY >= 0 && vecY < H){
                        if(currentMap[vecY][vecX] == 1){ mapVecLive++; } //si es un nodod vivo, la cantidad aumenta
                    }
                }
            }

            int totalPosN = (R*2 + 1)*(R*2 + 1) - 1; //calcular el area. Si R = 1, entonces el area es 9
            float threLive = static_cast<float>(mapVecLive) / totalPosN; // la cantidad de 1, en un rango de 0.0 a 1.0;

            //si es un 1
            if(currentMap[y][x] == 1){
                if(threLive >= U){ //si esta encima del minimo posible
                    newMap[y][x] = 1; }     //esta "vivo"
                else {
                    newMap[y][x] = 0; }  //esta "muerto"
            } else {
                //si es un 0
                if(threLive > U){ //si tiene suficiente vecinos vivos
                    newMap[y][x] = 1; //nace
                } else {
                    newMap[y][x] = 0; //sigue muerto
                }
            }
        }
    }

    // TODO: IMPLEMENTATION GOES HERE for the cellular automata logic.
    // Iterate over each cell and apply the transition rules.
    // Remember that updates should be based on the 'currentMap' state
    // and applied to the 'newMap' to avoid race conditions within the same iteration.

    return newMap;
}




/**
 * @brief Function to implement the Drunk Agent logic.
 * It should take a map and parameters controlling the agent's behavior,
 * then return the updated map after the agent performs its actions.
 *
 * @param currentMap The map in its current state.
 * @param W Width of the map.
 * @param H Height of the map.
 * @param J The number of times the agent "walks" (initiates a path).
 * @param I The number of steps the agent takes per "walk".
 * @param roomSizeX Max width of rooms the agent can generate.
 * @param roomSizeY Max height of rooms the agent can generate.
 * @param probGenerateRoom Probability (0.0 to 1.0) of generating a room at each step.
 * @param probIncreaseRoom If no room is generated, this value increases probGenerateRoom.
 * @param probChangeDirection Probability (0.0 to 1.0) of changing direction at each step.
 * @param probIncreaseChange If direction is not changed, this value increases probChangeDirection.
 * @param agentX Current X position of the agent (updated by reference).
 * @param agentY Current Y position of the agent (updated by reference).
 * @return The map after the agent's movements and actions.
 */
Map drunkAgent(const Map& currentMap, int W, int H, int J, int I, int roomSizeX, int roomSizeY,
               double probGenerateRoom, double probIncreaseRoom,
               double probChangeDirection, double probIncreaseChange,
               int& agentX, int& agentY) {
    Map newMap = currentMap; // The new map is a copy of the current one

    int sq = newMap[0][0];

    double prbRoom = probGenerateRoom;  //auxiliar valor cuarto
    double prbRoomIn = prbRoom;         //valor inicial

    double prbDir = probChangeDirection; //auxiliar probabilidad cuarto
    double prbDirIn = prbDir;


    float minVal = 0.0f;
    float maxVal = 1.0f; //valor maximo

    //direccion
    int dx = 1, dy = 0;
    //direccion elegida (azar)
    int dir = rand() % 2;

    switch(dir){
    case 0:
        dx = oneOrTwo(-1, 1); dy = 0;
        break;
    case 1:
        dx = 0; dy = oneOrTwo(-1, 1); 
        break;
    default:
        dx = 0; dy = 1;
        break;
    }

    int stepDist = (rand() % 3) -1;
    
    ///cantidad de iteraciones que camina
    for (int a = 0; a < J; a++) {
        for (int b = 0; b < (I + stepDist); b++) {
            if ((agentX + dx > 0 && agentX + dx < W - 1) &&
                (agentY + dy > 0 && agentY + dy < H - 1)) {

                agentX += dx;
                agentY += dy;
                newMap[agentY][agentX] = 1;

                // Intentar generar habitación
                double randRoom = static_cast<double>(rand()) / RAND_MAX;
                if (randRoom < prbRoom) {

                    int roomdifX = (rand() % 3) -1; //cambiar tamaño de cuarto, rango de -1 a 1
                    int roomdifY = (rand() % 3) -1; 


                    for (int ry = -(roomSizeY + roomdifY); ry <= (roomSizeY + roomdifY); ry++) {
                        for (int rx = -(roomSizeX + roomdifX); rx <= (roomSizeX + roomdifX); rx++) {
                            int newX = agentX + rx;
                            int newY = agentY + ry;
                            if (newX > 0 && newX < W && newY > 0 && newY < H) {
                                newMap[newY][newX] = 1;
                            }
                        }
                    }
                    prbRoom = prbRoomIn; // resetear probabilidad
                } else {
                    if (prbRoom + probIncreaseRoom <= 1.0)
                        prbRoom += probIncreaseRoom;
                }

                // Intentar cambiar dirección
                double randDir = static_cast<double>(rand()) / RAND_MAX;
                if (randDir < prbDir) {
                    if (dx == 0) {
                        dy = 0;
                        dx = oneOrTwo(-1, 1);
                    } else {
                        dx = 0;
                        dy = oneOrTwo(-1, 1);
                    }
                    prbDir = prbDirIn;
                } else {
                    if (prbDir + probIncreaseChange <= 1.0)
                        prbDir += probIncreaseChange;
                }
            }
        }
        //printMap(newMap);     //para corroborar que esta siendo hecho el mapa
    }

    //direccion
    //camina
    //habitacion o no?
    //si crea habitacion, baja la prob, sino, aumenta la prob de crear habitacion
    //nueva direccion


    // TODO: IMPLEMENTATION GOES HERE for the Drunk Agent logic.
    // The agent should move randomly.
    // You'll need a random number generator.
    // Consider:
    // - How the agent moves (possible steps).
    // - What it does if it encounters a border or an obstacle (if applicable).
    // - How it modifies the map (e.g., leaving a trail, creating rooms, etc.).
    // - Use the provided parameters (J, I, roomSizeX, roomSizeY, probabilities)
    //   to control its behavior.

    return newMap;
}


//metodo que regresa un valor u el otro
int oneOrTwo(int a, int b){
    int choice = rand() % 2;
    if (choice == 0) return a;
    return b;
}



int main() {

    srand(time(0));   //randomizador

    std::cout << "--- CELLULAR AUTOMATA AND DRUNK AGENT SIMULATION ---" << std::endl;

    // --- Initial Map Configuration ---
    int mapRows = 20;
    int mapCols = 20;
    Map myMap(mapRows, std::vector<int>(mapCols, 0)); // Map initialized with zeros

    // TODO: IMPLEMENTATION GOES HERE: Initialize the map with some pattern or initial state.
    // For example, you might set some cells to 1 for the cellular automata
    // or place the drunk agent at a specific position.

    // Drunk Agent's initial position
    int drunkAgentX = mapRows/2;
    int drunkAgentY = mapCols/2;
    // If your agent modifies the map at start, you could do it here:
    // myMap[drunkAgentX][drunkAgentY] = 2; // Assuming '2' represents the agent

    std::cout << "\nInitial map state:" << std::endl;
    printMap(myMap);

    // --- Simulation Parameters ---
    int numIterations = 5; // Number of simulation steps

    // Cellular Automata Parameters
    int ca_W = mapCols;
    int ca_H = mapRows;
    int ca_R = 1;      // Radius of neighbor window
    double ca_U = 0.5; // Threshold

    // Drunk Agent Parameters
    int da_W = mapCols;
    int da_H = mapRows;
    int da_J = 6;      // Number of "walks"
    int da_I = 8;     // Steps per walk
    int da_roomSizeX = 2;   //tamaño de los cuartos (eje X)
    int da_roomSizeY = 1;   //tamaño de los cuartos (eje Y)
    double da_probGenerateRoom = 0.01;
    double da_probIncreaseRoom = 0.01;
    double da_probChangeDirection = 0.5;
    double da_probIncreaseChange = 0.03;

    /* myMap = drunkAgent(myMap, da_W, da_H, da_J, da_I, da_roomSizeX, da_roomSizeY,
                           da_probGenerateRoom, da_probIncreaseRoom,
                           da_probChangeDirection, da_probIncreaseChange,
                           drunkAgentX, drunkAgentY); */

    

    Map nMap = myMap;
    Map celMap = randomizeMap(myMap, ca_W, ca_H);
    
    
    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < numIterations; ++iteration) {
        std::cout << "\n--- Iteration " << iteration + 1 << " ---" << std::endl;

        int agX = drunkAgentX;
        int agY = drunkAgentY;

        // TODO: IMPLEMENTATION GOES HERE: Call the Cellular Automata and/or Drunk Agent functions.
        // The order of calls will depend on how you want them to interact.

        // Example: First the cellular automata, then the agent
        myMap = cellularAutomata(celMap, ca_W, ca_H, ca_R, ca_U);
        celMap = myMap;
        
        /*nMap = drunkAgent(myMap, da_W, da_H, da_J, da_I, da_roomSizeX, da_roomSizeY,
                           da_probGenerateRoom, da_probIncreaseRoom,
                           da_probChangeDirection, da_probIncreaseChange,
                           agX, agY);
                           */

        printMap(myMap);

        // You can add a delay to visualize the simulation step by step
        // #include <thread> // For std::this_thread::sleep_for
        // #include <chrono> // For std::chrono::milliseconds
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;
    return 0;
}