#include "App.h"

#include "Registro.h"
#include "Rutas.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stb_image_write.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <limits>
#include <vector>

namespace fs = std::filesystem;

static App* g_app = nullptr;

int App::ejecutar(const Opciones& opciones) {
    op_ = opciones;
    if (!iniciar()) {
        terminar();
        return 1;
    }

    int cuadros = 0;
    double anterior = glfwGetTime(), acumulado = 0.0;
    int cuadrosFps = 0;
    while (!glfwWindowShouldClose(ventana_)) {
        double ahora = glfwGetTime();
        float dt = static_cast<float>(std::min(ahora - anterior, 0.1));
        anterior = ahora;
        acumulado += dt;
        ++cuadrosFps;
        if (acumulado >= 0.5) {
            fps_ = static_cast<float>(cuadrosFps / acumulado);
            msCuadro_ = static_cast<float>(acumulado * 1000.0 / cuadrosFps);
            acumulado = 0.0;
            cuadrosFps = 0;
        }

        glfwPollEvents();
        glfwGetFramebufferSize(ventana_, &anchoFb_, &altoFb_);
        if (anchoFb_ == 0 || altoFb_ == 0) {  // ventana minimizada
            glfwWaitEvents();
            continue;
        }
        procesarEntrada(dt);
        if (tiempoMensaje_ > 0.0f) tiempoMensaje_ -= dt;

        renderer_.dibujar(escena_.raiz(), camara_, anchoFb_, altoFb_, nullptr);

        dibujarInterfaz();

        ++cuadros;
        if (!op_.captura.empty() && cuadros == op_.frames) {
            capturar(op_.captura);
            glfwSetWindowShouldClose(ventana_, GLFW_TRUE);
        }
        glfwSwapBuffers(ventana_);
    }
    terminar();
    return 0;
}

bool App::iniciar() {
    g_app = this;
    glfwSetErrorCallback([](int codigo, const char* desc) { registro::error("GLFW %d: %s", codigo, desc); });
    if (!glfwInit()) {
        registro::fatal("No se pudo iniciar GLFW.");
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    if (op_.msaa > 0) glfwWindowHint(GLFW_SAMPLES, op_.msaa);
    if (op_.ancho <= 0 || op_.alto <= 0) {
        // tamaño inicial: 80 % del área de trabajo del monitor principal
        int x = 0, y = 0, w = 1280, h = 720;
        if (GLFWmonitor* mon = glfwGetPrimaryMonitor()) glfwGetMonitorWorkarea(mon, &x, &y, &w, &h);
        op_.ancho = std::max(960, w * 4 / 5);
        op_.alto = std::max(540, h * 4 / 5);
    }
    ventana_ = glfwCreateWindow(op_.ancho, op_.alto, "Camino de Cempasuchil | Entrega de modelado",
                               nullptr, nullptr);
    if (!ventana_) {
        registro::fatal("No se pudo crear la ventana. Se requiere una tarjeta gráfica compatible con OpenGL 3.3.");
        return false;
    }
    glfwMakeContextCurrent(ventana_);
    glfwSwapInterval(1);  // sincronía vertical: evita trabajo de más en la GPU
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        registro::fatal("No se pudieron cargar las funciones de OpenGL.");
        return false;
    }
    registro::info("OpenGL %s | GLSL %s | %s", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION),
                   glGetString(GL_RENDERER));
    if (op_.msaa > 0) glEnable(GL_MULTISAMPLE);

    if (!renderer_.iniciar()) {
        registro::fatal("No se pudieron compilar los shaders (carpeta shaders/).");
        return false;
    }
    auto t0 = std::chrono::steady_clock::now();
    if (!escena_.cargar()) {
        registro::fatal("No se pudieron cargar los modelos (carpeta assets/).");
        return false;
    }
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    registro::info("Carga de geometria: %.0f ms", ms);

    renderer_.alambre = op_.alambre;
    // ImGui instala sus callbacks primero; los nuestros reemplazan teclado, rueda y
    // botones del ratón y le reenvían los eventos.
    iniciarInterfaz();
    glfwSetKeyCallback(ventana_, alTeclear);
    glfwSetScrollCallback(ventana_, alDesplazar);
    glfwSetMouseButtonCallback(ventana_, alPulsar);
    mostrarInterfaz_ = !op_.sinInterfaz;
    aplicarVista(op_.vista >= 1 ? op_.vista - 1 : 0);
    return true;
}

void App::terminar() {
    if (ImGui::GetCurrentContext()) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    escena_.liberar();
    renderer_.liberar();
    if (ventana_) glfwDestroyWindow(ventana_);
    ventana_ = nullptr;
    glfwTerminate();
}

// ---------------------------------------------------------------------------
// Entrada
// ---------------------------------------------------------------------------
// Navegacion libre: no hay seleccion ni edicion de objetos.
void App::procesarEntrada(float dt) {
    double x, y;
    glfwGetCursorPos(ventana_, &x, &y);
    float dx = float(x - ultimoX_), dy = float(y - ultimoY_);
    ultimoX_ = x; ultimoY_ = y;
    if (arrastrando_) camara_.girar(dx, dy);
    if (ImGui::GetIO().WantCaptureKeyboard) return;
    auto tecla = [&](int k) { return glfwGetKey(ventana_, k) == GLFW_PRESS; };
    glm::vec3 d(0.0f);
    if (tecla(GLFW_KEY_W)) d.z += 1;
    if (tecla(GLFW_KEY_S)) d.z -= 1;
    if (tecla(GLFW_KEY_D)) d.x += 1;
    if (tecla(GLFW_KEY_A)) d.x -= 1;
    if (tecla(GLFW_KEY_E)) d.y += 1;
    if (tecla(GLFW_KEY_Q)) d.y -= 1;
    if (d != glm::vec3(0)) camara_.desplazar(glm::normalize(d), dt, tecla(GLFW_KEY_LEFT_SHIFT) ? 3.0f : 1.0f);
}

void App::alTeclear(GLFWwindow* v, int tecla, int codigo, int accion, int mods) {
    ImGui_ImplGlfw_KeyCallback(v, tecla, codigo, accion, mods);
    if (accion != GLFW_PRESS || ImGui::GetIO().WantCaptureKeyboard) return;
    App& a = *g_app;
    switch (tecla) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(v, GLFW_TRUE); break;
        case GLFW_KEY_F2: a.renderer_.alambre = !a.renderer_.alambre; break;
        case GLFW_KEY_F4: a.mostrarInterfaz_ = !a.mostrarInterfaz_; break;
        case GLFW_KEY_HOME: a.aplicarVista(0); break;
        default: break;
    }
}

void App::alDesplazar(GLFWwindow* v, double dx, double dy) {
    ImGui_ImplGlfw_ScrollCallback(v, dx, dy);
    if (!ImGui::GetIO().WantCaptureMouse) g_app->camara_.acercar(float(dy));
}

void App::alPulsar(GLFWwindow* v, int boton, int accion, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(v, boton, accion, mods);
    if (boton != GLFW_MOUSE_BUTTON_RIGHT) return;
    App& a = *g_app;
    if (accion == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse) {
        a.arrastrando_ = true;
        glfwSetInputMode(v, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(v, &a.ultimoX_, &a.ultimoY_);
    } else if (accion == GLFW_RELEASE) {
        a.arrastrando_ = false;
        glfwSetInputMode(v, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

void App::aplicarVista(int indice) {
    const auto& vistas = escena_.vistas();
    if (indice < 0 || indice >= static_cast<int>(vistas.size())) return;
    vistaActual_ = indice;
    camara_.posicion = vistas[static_cast<size_t>(indice)].posicion;
    camara_.mirarHacia(vistas[static_cast<size_t>(indice)].objetivo);
}

void App::capturar(const std::string& ruta) {
    std::vector<unsigned char> px(static_cast<size_t>(anchoFb_) * altoFb_ * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, anchoFb_, altoFb_, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    stbi_flip_vertically_on_write(1);
    bool ok = stbi_write_png(ruta.c_str(), anchoFb_, altoFb_, 3, px.data(), anchoFb_ * 3) != 0;
    if (ok) {
        registro::info("Captura guardada en %s", ruta.c_str());
        mensaje_ = "Captura guardada: " + ruta;
    } else {
        registro::error("No se pudo guardar la captura en %s", ruta.c_str());
        mensaje_ = "No se pudo guardar la captura";
    }
    tiempoMensaje_ = 4.0f;
}
