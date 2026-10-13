# Camino de Cempasúchil — entrega de modelado

Versión independiente para el 13 de octubre de 2026. Basada en el proyecto
`olveratrech/final-project-cgih-c`, commit `c25047336dcda1f8575f7fc92d6d24ed7eeb78f4`
(checkpoint `estable-arco-florido`). La carpeta y el historial del proyecto completo
no se modifican. Este repositorio tiene su propio historial Git.

## Alcance

Se reutiliza la escena y la geometría exportada existente, incluidas la calaverita,
la flor y el arco poblado. Render OpenGL 3.3, gris mate, luz direccional blanca y
luz ambiental constantes, fondo gris uniforme. Las jerarquías se conservan para
colocar correctamente las piezas, pero no hay edición interactiva.

Sin texturas visibles, metal, reflejos, emisión, cielo con textura, niebla,
transparencias, recortes alfa, animación, selección, inspector ni galería.
El papel picado y las alas muestran su geometría completa: los dibujos y recortes
que antes dependían de texturas no forman parte de la malla. Vidrio y agua son opacos.
Se retiraron las imágenes externas y sus referencias de los glTF propios. Los GLB
de librería se conservan originales; el programa utiliza su geometría y no carga imágenes. No se alteraron las mallas para simular esos detalles.

## Abrir en Xcode (Mac M1)

La carpeta `build-xcode` es local y no se sube a GitHub. En esta máquina ya está
configurada. Abre `build-xcode/ModeladoCempasuchil.xcodeproj`, elige el esquema
**ModeladoCempasuchil**, destino **My Mac**, y pulsa **Cmd + R**.

Para configurar desde una descarga nueva, con CMake y Xcode instalados:

```sh
DEVELOPER_DIR=/Applications/Xcode.app/Contents/Developer cmake -S . -B build-xcode -G Xcode
open build-xcode/ModeladoCempasuchil.xcodeproj
```

También se incluye `Abrir_en_Xcode.command`: busca CMake en PATH, Homebrew,
CMake.app y en la instalación local de MATLAB; genera el proyecto y lo abre.

## Controles

| Acción | Control |
|---|---|
| Avanzar / retroceder / desplazarse lateralmente | W / S / A / D |
| Bajar / subir | Q / E |
| Mirar alrededor | Mantener botón derecho y mover el ratón |
| Acelerar | Shift |
| Ajustar velocidad de recorrido | Rueda del ratón |
| Alternar sólido / alambre | F2 |
| Volver a la posición inicial | Home (Fn + flecha izquierda en Mac) |
| Salir | Esc |

En Mac puede hacer falta **Fn + F2** según los ajustes del teclado.
El botón izquierdo no selecciona ni modifica objetos.

## Compilación portable

```sh
cmake -S . -B build
cmake --build build --config Release
```

Las dependencias están incluidas en `external/`. Se copian `assets/`, `shaders/`
y `readme.txt` junto al ejecutable. Para distribuirlo, conserva esos elementos juntos.
El ejecutable compilado en Mac no es un `.exe`: una entrega de Windows requiere
compilar esta misma fuente en Windows.

## GitHub

Este repositorio independiente está preparado para publicarse en un repositorio
nuevo y vacío, sin sobrescribir `main` del proyecto completo:

```sh
git remote add origin URL_DEL_REPOSITORIO_NUEVO
git push -u origin main
git push origin estable-entrega-modelado
```

No se incluyen archivos compilados, cachés de Xcode ni capturas de prueba.
Licencias y atribuciones: `LICENSE`, `assets/CREDITOS.md` y `external/README.md`.

## Estructura y dependencias

- `src/`: ventana, entrada, cámara, carga de modelos y dibujo de la escena.
- `shaders/`: un shader de vértices y uno de fragmentos (gris mate).
- `assets/models/`: geometría propia y de librería.
- `external/`: GLFW (ventana y entrada), GLAD (OpenGL), GLM (matrices) y cgltf (modelos).

No se incluyen ImGui, stb, menú, capturas, cargador de texturas ni skybox.
Se conservan las jerarquías, el dibujo instanciado y el descarte por cámara para
reutilizar la escena sin perjudicar el rendimiento. La simplificación no convierte
el programa en un ejercicio de solo primitivas: sigue siendo un visor de modelos glTF.

Para ver únicamente los archivos de entrega, omitiendo los generados y las
fuentes internas de terceros: `tree -I 'build*|.git|external'`.
