// Controles minimos de recorrido y visualizacion de la geometria.
#include "App.h"
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
void App::iniciarInterfaz() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(ventana_, false);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}
void App::dibujarInterfaz() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    if (mostrarInterfaz_) {
        ImGui::SetNextWindowPos(ImVec2(12,12), ImGuiCond_Always);
        ImGui::Begin("Entrega de modelado", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
        ImGui::TextUnformatted("W A S D: recorrer | Q / E: bajar / subir");
        ImGui::TextUnformatted("Boton derecho + raton: mirar");
        ImGui::TextUnformatted("Shift: acelerar | Rueda: velocidad");
        ImGui::TextUnformatted("F2: alambre | F4: ocultar controles | Esc: salir");
        ImGui::Checkbox("Vista de alambre", &renderer_.alambre);
        if (ImGui::Button("Volver al inicio")) aplicarVista(0);
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
