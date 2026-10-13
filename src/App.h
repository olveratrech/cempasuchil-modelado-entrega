// Aplicacion independiente para la entrega de modelado.
#pragma once
#include "render/Renderer.h"
#include "scene/Camera.h"
#include "scene/Escena.h"
#include <string>
struct GLFWwindow;
struct Opciones {
    int ancho=0, alto=0, msaa=4, frames=4, vista=-1;
    std::string captura;
    bool sinInterfaz=false, alambre=false;
};
class App {
public:
    int ejecutar(const Opciones& opciones);
private:
    bool iniciar();
    void terminar();
    void procesarEntrada(float dt);
    void aplicarVista(int indice);
    void capturar(const std::string& ruta);
    void iniciarInterfaz();
    void dibujarInterfaz();
    static void alTeclear(GLFWwindow*,int,int,int,int);
    static void alDesplazar(GLFWwindow*,double,double);
    static void alPulsar(GLFWwindow*,int,int,int);
    GLFWwindow* ventana_=nullptr;
    Opciones op_;
    Escena escena_;
    Renderer renderer_;
    Camara camara_;
    bool mostrarInterfaz_=true, arrastrando_=false;
    double ultimoX_=0, ultimoY_=0;
    int anchoFb_=1, altoFb_=1, vistaActual_=0;
    float fps_=0, msCuadro_=0, tiempoMensaje_=0;
    std::string mensaje_;
};
