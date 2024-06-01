# CDR_Cuatro_en_Linea

Repositorio enfocado para gestionar las versiones del código de la Tarea 1: "Cuatro en Línea" para la asignatura de Comunicación de Datos y Redes.

## Instrucciones del juego

- El objetivo del juego es alinear cuatro fichas sobre un tablero formado por seis filas y siete columnas.
- Al ser un videojuego en red, el jugador (Cliente) deberá jugar contra el servidor por medio de turnos. Ganará la partida el primero que consiga alinear cuatro de sus fichas de forma consecutiva.
- El jugador (Cliente) deberá usar las fichas con el caracter "C", y el servidor usará el caracter "S". De esta manera se podrán identificar las fichas de cada jugador.

## Instrucciones de compilación

1. Ejecute `make` para compilar el código.
2. Ejecute `make clean` para eliminar los archivos generados durante el proceso de compilación.

## Instrucciones de ejecución

1. Ejecute `./servidor <numero_de_puerto>` para ejecutar el código del servidor.
2. Ejecute `./cliente` para ejecurtar el código del servidor.
   - 2.1. Ingrese la dirección IP del servidor a conectarse.
   - 2.2. Ingrese el puerto del servidor a conectarse.
     
