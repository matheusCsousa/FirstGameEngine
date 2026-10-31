#include "TransformControls.hpp"
#include <imgui.h>
#include <iostream>
#include <stdexcept>

void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
struct Bounds { ImVec2 min,max; };
Bounds frame(glm::vec3& value,bool scale=false) {
    auto& io=ImGui::GetIO(); io.DeltaTime=1.0f/60; io.DisplaySize={800,400};
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({0,0}); ImGui::SetNextWindowSize({800,400});
    ImGui::Begin("Test",nullptr,ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoMove);
    EditorWidgets::editVector(scale ? "Scale" : "Position",value,0.1f,scale);
    Bounds bounds{ImGui::GetItemRectMin(),ImGui::GetItemRectMax()};
    ImGui::End(); ImGui::Render(); return bounds;
}
void drag(glm::vec3& value,float delta,bool scale=false) {
    auto& io=ImGui::GetIO();
    const auto bounds=frame(value,scale);
    const ImVec2 point{bounds.min.x+30,(bounds.min.y+bounds.max.y)/2};
    io.AddMousePosEvent(point.x,point.y); frame(value,scale);
    io.AddMouseButtonEvent(0,true); frame(value,scale);
    io.AddMousePosEvent(point.x+delta,point.y); frame(value,scale);
    io.AddMouseButtonEvent(0,false); frame(value,scale);
}
void enterValue(glm::vec3& value,const char* text,bool scale=false) {
    auto& io=ImGui::GetIO();
    const auto bounds=frame(value,scale);
    io.AddMousePosEvent(bounds.min.x+30,(bounds.min.y+bounds.max.y)/2); frame(value,scale);
    io.AddKeyEvent(ImGuiMod_Ctrl,true); io.AddMouseButtonEvent(0,true); frame(value,scale);
    io.AddMouseButtonEvent(0,false); io.AddKeyEvent(ImGuiMod_Ctrl,false); frame(value,scale);
    io.AddInputCharactersUTF8(text); frame(value,scale);
    io.AddKeyEvent(ImGuiKey_Enter,true); frame(value,scale);
    io.AddKeyEvent(ImGuiKey_Enter,false); frame(value,scale);
}
int main() {
    ImGui::CreateContext();
    try {
        auto& io=ImGui::GetIO(); io.IniFilename=nullptr; io.MouseDoubleClickTime=0.001f;
        unsigned char* pixels; int width,height;
        io.Fonts->GetTexDataAsRGBA32(&pixels,&width,&height);
        glm::vec3 position{0,2,3}; drag(position,70);
        require(position.x>1 && position.y==2 && position.z==3,"Position drag must move X without pinning other axes to zero");
        drag(position,-140);
        require(position.x<0,"Position drag must permit negative coordinates");
        glm::vec3 rotation{0,45,90}; drag(rotation,100);
        require(rotation.x>0 && rotation.y==45 && rotation.z==90,"Rotation drag must not be clamped to zero");
        enterValue(position,"-12.5");
        require(position.x==-12.5f,"Typed negative position must be accepted");
        glm::vec3 scale{1,2,3}; drag(scale,-200,true);
        require(scale.x>=0.01f && scale.y==2 && scale.z==3,"Scale drag must remain positive");
        enterValue(scale,"-5",true);
        require(scale.x>=0.01f,"Typed negative scale must be clamped");
        ImGui::DestroyContext();
        std::cout<<"Real ImGui transform drag regression checks passed\n"; return 0;
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n'; ImGui::DestroyContext(); return 1; }
}

