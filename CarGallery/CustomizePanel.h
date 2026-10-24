#pragma once

#include "Car.h"
#include <vector>

// ImGui window: choose one of the cars and edit its CarConfig
class CustomizePanel
{
public:
    void draw(std::vector<Car>& cars);
    // index of the car being edited; every car is drawn regardless of this
    int selected() const { return current; }

private:
    int current = 0;
};
