Night Tales

Night Tales es un juego 2D de exploración y supervivencia ambientado en islas flotantes conectadas por cadenas. 
Cada partida genera un mundo procedural único con islas, biomas y conexiones que varían según la configuración elegida.

Características
1. Generación procedural: 3 layouts × 4 métodos de conexión = 12 combinaciones posibles.
2. Islas únicas: Terreno generado con ruido Perlin, domain warping y máscara radial.
3. Sistema de props: Árboles, flores, piedras y arbustos de moras con recolección y respawn.
4. Inventario y stats: Vida, hambre y cordura. Inventario de 6 slots con stacks.
5. Ciclo día/noche: 10 minutos reales = 1 día de juego.
6. Guardado persistente: Múltiples partidas con nombres únicos en archivos de texto plano.
7. Minimapa y mapa completo: Navegación entre islas y visualización de conexiones.
8. Portabilidad: Código diseñado para compilar en Linux, Windows y macOS.

Tecnologías:
Tecnología      	Versión          	Uso
C++	                17+	      Lenguaje principal
SFML	              3.x      	Gráficos, ventana, input
DeSiGNAR	          1.0	      Contenedores y estructuras de datos
Make	               -	      Compilación (Linux/macOS)

Requisitos:
Compilador C++17 (GCC, Clang o MSVC).
SFML 3.x instalado en el sistema.
DeSiGNAR clonado en la carpeta DeSiGNAR/ del proyecto.
Make (Linux/macOS) o MinGW/CMake (Windows).

Instalación y compilación:
1. Clonar el repositorio
git clone https://github.com/Marcano-Esteban/Night-Tales.git
cd Night-Tales

3. Clonar DeSiGNAR (si no está incluido)
git clone https://github.com/r3mmurd/DeSiGNAR.git

5. Instalar SFML:
Linux (Arch/CachyOS)
sudo pacman -S sfml

Linux (Debian/Ubuntu):
sudo apt install libsfml-dev

macOS (Homebrew):
brew install sfml

Windows
Se recomienda usar vcpkg:
vcpkg install sfml
O descargar los binarios precompilados desde sfml-dev.org.

4. Compilar y ejecutar:
Linux / macOS
make
./TerrainGenerator

Windows (MinGW)
make -f Makefile.mingw
TerrainGenerator.exe

🎯 Controles
Tecla	Acción
WASD	Moverse
E	Interactuar (escalar árbol, recoger piedra, viajar)
M	Abrir/cerrar mapa grande
ESC	Pausa
Rueda del ratón	Zoom

🏗️ Arquitectura
El proyecto sigue una arquitectura en capas con separación de responsabilidades:

text
src/
├── main.cpp               # Punto de entrada, máquina de estados
├── game.cpp/hpp           # Coordinador del gameplay
├── world.cpp/hpp          # Gestión de islas y generación
├── island.cpp/hpp         # Cada isla individual
├── terrain_generator.*    # Generación de terreno (Perlin + warping)
├── noise_generator.*      # Algoritmo de ruido
├── height_map.*           # Mapa de alturas
├── player.cpp/hpp         # Jugador con stats e inventario
├── inventory.cpp/hpp      # Sistema de inventario
├── item.cpp/hpp           # Definición de items
├── prop.cpp/hpp           # Props del mundo (árboles, etc.)
├── save_manager.*         # Guardado/carga de partidas
└── utilyties.*            # Funciones auxiliares
Para más detalles, consulta el Documento de Diseño de Software.

🧪 Uso de DeSiGNAR
El proyecto usa Designar::DynArray (equivalente a std::vector) para los contenedores principales, cumpliendo con el requisito de la asignatura. La jerarquía de islas se implementa como un árbol N-ario autorreferencial (cada Island conoce a su padre, hijas y vecinas), lo que permite combinar generación jerárquica con navegación por grafo.

🗺️ Roadmap
☑ Generación procedural de islas (3 layouts, 4 conexiones).
☑ Sistema de props con recolección y respawn.
☑ Inventario y stats del jugador.
☑ Ciclo día/noche.
☑ Guardado y carga de partidas.
☑ Menús de inicio, pausa y carga.
□ Sprites (reemplazar dibujos primitivos por imágenes).
□ Multijugador local (split-screen).
□ Crafteo (ramas + piedras → herramientas).
□ Combate con monstruos.
□ Menú de opciones (volumen, resolución).
□ Migrar a CMake para compilación multiplataforma más sencilla.

👥 Autor
Esteban Marcano
Universidad de Los Andes (ULA)
GitHub

📄 Licencia
Este proyecto está bajo la licencia MIT. Consulta el archivo LICENSE para más detalles.

📚 Referencias
Documentación de SFML 3
Repositorio de DeSiGNAR
Guía de Atlassian sobre Documentos de Diseño de Software

¡Gracias por visitar Night Tales! Si te gusta el proyecto, no dudes en darle una ⭐ en GitHub.
