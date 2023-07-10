# Los Mundos de Belkan

Practica de la asignatura de **Inteligencia Artificial**.

## Descripción

Este programa consiste en que nuestro jugador (flecha roja) debe alcanzar distintos objetivos (hexágono rojo) en el menor tiempo y con la mayor energía posible. La idea es que dado un mapa, distinguimos por colores distintas zonas como agua, tierra, arena, etc, con distinto nivel de dificultad al pasar sobre ellas, y nuestro usuario debe llegar a su destino final minimizando el cansancio y maximizando el tiempo. Procedo a explicar la vista de interfaz y posteriormente explicaré los distintos niveles.

La parte gráfica se divide en tres partes. La primera es el mapa en el que nos situamos (a la izquierda de la pantalla), luego la vista en primera persona del jugador (arriba a la derecha) y finalmente toda la informaciń asociada (abajo a la derecha).

Como he comentado antes, se distinguen varios niveles de juego:
    
1. El primer nivel consiste en que, dado un mapa conocido, el usuario busca el camino más eficiente usando los algoritmos de **profundidad**, **anchura** o **coste uniforme**. Una vez llegue a su destino, termina la partida.

2. El segundo nivel es el más interesante, ya que consiste que dado un determinado tiempo y un nivel de fortaleza, deberá obtener el máximo número de objetivos posibles. Recordemos que la partida terminará cuando se quede sin tiempo o sin energía. Para este segundo nivel, he utilizado el **algoritmo A***. Otro punto a tener en cuenta, es que el jugador al comienzo de cada juego, desconocerá el mapa por completo y según pase por distintas zonas, irá aprendiendo para usar ese conocimiento en el futuro para hacer rutas más eficientes.

## Compilación y Ejecución

En primer lugar, tras descargar los archivos del repositorio, para compilar este programa en **Linux**, deberemos darle permisos y ejecutar el archivo `install.sh` de la siguiente forma :

`chmod +x install.sh`

`./install.sh`

Una vez hecho esto, debemos realizar un `make`, y esperar a que compile.

Finalmente, para ejecutar el programa, tenemos dos opciones:
    
1. Podemos directamente ejecutar el comando :

    `./Belkan`
    
    Esto dará inicio al juego, y deberemos configurarlo manualmente.

2. Podemos ejecutar :

    `./BelkanSG {mapa} {semilla} {nivel} {origen(F)} {origen(C)} {destino(F)} {destino(C)}`

    Una vez hecho esto podremos disfrutar del juego, aunque yo recomiendo la primera ejecución.