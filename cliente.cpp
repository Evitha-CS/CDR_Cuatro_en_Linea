#include <iostream>
#include <vector>
#include <string>

class Tablero {
private:
    std::vector<std::vector<char>> tablero;
public:
    Tablero() : tablero(6, std::vector<char>(7, ' ')) {}

    void agregarFicha(int columna, char ficha) {
        for (int fila = 5; fila >= 0; --fila) {
            if (tablero[fila][columna] == ' ') {
                tablero[fila][columna] = ficha;
                break;
            }
        }
    }

    void mostrarTablero() {
        for (const auto& fila : tablero) {
            for (char c : fila) {
                std::cout << c << ' ';
            }
            std::cout << std::endl;
        }
        std::cout << "-------------" << std::endl;
        std::cout << "1 2 3 4 5 6 7" << std::endl;
    }

    char getFicha(int fila, int columna) const {
        return tablero[fila][columna];
    }
};

class Jugador {
private:
    char ficha;
public:
    Jugador(char f) : ficha(f) {}

    char getFicha() const {
        return ficha;
    }
};

class Juego {
private:
    Tablero tablero;
    Jugador jugador1{'S'};
    Jugador jugador2{'C'};
    Jugador* turno;
public:
    Juego() : turno(&jugador1) {}

    void jugar() {
        while (true) {
            tablero.mostrarTablero();
            int columna;
            std::cout << "Jugador " << turno->getFicha() << ", elige una columna (1-7): ";
            std::cin >> columna;

            if (columna >= 1 && columna <= 7 && tablero.getFicha(0, columna - 1) == ' ') {
                tablero.agregarFicha(columna - 1, turno->getFicha());
                if (verificarGanador()) {
                    std::cout << "¡Jugador " << turno->getFicha() << " ha ganado!" << std::endl;
                    break;
                }
                turno = (turno == &jugador1) ? &jugador2 : &jugador1;
            } else {
                std::cout << "Movimiento no válido. Inténtalo de nuevo." << std::endl;
            }
        }
    }

    bool verificarGanador() {
        // Verificar horizontalmente
        for (int fila = 0; fila < 6; ++fila) {
            for (int columna = 0; columna <= 3; ++columna) {
                if (tablero.getFicha(fila, columna) != ' ' &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila, columna + 1) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila, columna + 2) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila, columna + 3)) {
                    return true;
                }
            }
        }

        // Aquí puedes agregar más lógica para verificar verticalmente, diagonalmente, etc.

        return false;
    }
};

int main() {
    Juego juego;
    juego.jugar();
    return 0;
}
