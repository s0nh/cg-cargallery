#pragma once

#include "Car.h"
#include <vector>

// ImGui window: choose which car stands on the pedestal and edit its CarConfig
class CustomizePanel
{
public:
    void draw(std::vector<Car>& cars);
    int selected() const { return current; }

private:
    int current = 0;
};
