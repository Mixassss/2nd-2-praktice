#include "../include/networks.h"

int main() {

    BaseDate airport;

    airport.parse();
    airport.mkdir();

    createServer(airport);
    
    return 0;
}