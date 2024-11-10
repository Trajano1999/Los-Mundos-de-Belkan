# Los Mundos de Belkan

Práctica de la asignatura de **Inteligencia Artificial** del Doble Grado en Ingeniería Informática y Matemáticas de la Universidad de Granada.

## Descripción

Este programa consiste en que el jugador (flecha roja) alcance distintos objetivos (hexágonos rojos) con un tiempo y energía limitados. Asimismo, cada mapa cuenta con distintas zonas (agua, tierra, arena, etc) y cada una cuenta con un nivel de dificultad a ser atravesada. Por tanto, el jugador debe llegar a su destino optimizando el tiempo y la energía.

La parte gráfica se divide en tres partes. La primera es el mapa, que se encuentra a la izquierda de la pantalla; la segunda es la vista en primera persona del jugador, ubicada en la parte superior derecha; y la tercera es el área de información, situadada en la parte inferior derecha.

Además, se distinguen varios niveles de juego:

1. El primer nivel consiste en que, dado un mapa conocido, el jugador busca el camino más eficiente usando los algoritmos de **profundidad**, **anchura** o **coste uniforme**. Una vez alcanza su destino, termina la partida.

2. El segundo nivel es el más complejo e interesante, ya que, dado un determinado tiempo y un nivel de fortaleza, el jugador debe conseguir el máximo número de objetivos posibles. Es importante destacar que la partida terminará cuando el jugador se quede sin tiempo o energía. Además, al inicio de cada juego, el jugador no conocerá el mapa en su totalidad, sino que, a medida que explore diferentes áreas, irá adquiriendo conocimientos que le permitirán trazar rutas más eficientes en futuras partidas. Para ello, se hace uso del **algoritmo A\***.

## Compilación y Ejecución

En primer lugar, tras descargar los archivos del repositorio para compilar este programa en **Linux**, será necesario otorgar los permisos correspondientes y ejecutar el archivo ``install.sh`` de la siguiente forma:

````
chmod +x install.sh
./install.sh
````

Una vez hecho esto, se debe ejecutar el comando ``make`` y esperar a que se complete el proceso de compilación.

Finalmente, existen dos opciones para ejecutar el programa, siendo recomendable optar por la primera de ellas:

1. Ejecutar directamente el siguiente comando, que permite una configuración manual:

    ``./Belkan``

2. Ejecutar el siguiente comando, en el que los parámetros especifican la configuración deseada:

    ``./BelkanSG {mapa} {semilla} {nivel} {origen(F)} {origen(C)} {destino(F)} {destino(C)}``

Con estos pasos completados, se podrá comenzar a disfrutar del juego.
