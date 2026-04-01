// Copyright 2021 GHA Test Team
#include "../include/TimedDoor.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        std::cout << "Creating timed door with 2 second timeout..." << std::endl;
        TimedDoor door(2);
        Timer timer;
        
        std::cout << "Door is initially closed: " << (door.isDoorOpened() ? "open" : "closed") << std::endl;
        
        std::cout << "Opening the door..." << std::endl;
        door.unlock();
        std::cout << "Door is now open: " << (door.isDoorOpened() ? "open" : "closed") << std::endl;
        
        std::cout << "Starting timer for " << door.getTimeOut() << " seconds..." << std::endl;
        DoorTimerAdapter adapter(door);
        timer.tregister(door.getTimeOut(), &adapter);
        
        std::cout << "Timer completed, door was closed in time" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
    
    return 0;
}
