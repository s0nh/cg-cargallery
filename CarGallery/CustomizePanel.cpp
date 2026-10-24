#include "CustomizePanel.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

void CustomizePanel::draw(std::vector<Car>& cars)
{
    ImGui::Begin("Customize", nullptr, ImGuiWindowFlags_AlwaysAutoResize);  // grows to fit the car list
    if (cars.empty()) {
        ImGui::End();
        return;
    }

    for (int i = 0; i < (int)cars.size(); i++)
        ImGui::RadioButton(cars[i].name.c_str(), &current, i);

    Car& car = cars[current];
    if (!car.loaded()) ImGui::Text("Car model not found");
    // edits config directly; Car::draw reads it every frame, so the change shows immediately
    ImGui::ColorEdit3("Paint", glm::value_ptr(car.config.bodyColor));
    ImGui::End();
}
