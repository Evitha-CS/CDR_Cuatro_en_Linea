#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>
#include <cstdlib>

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

    void mostrarTablero(int socket_cliente) {
        std::string tablero_str = toString();
        size_t tablero_size = tablero_str.length();
        send(socket_cliente, &tablero_size, sizeof(tablero_size), 0);
        send(socket_cliente, tablero_str.c_str(), tablero_size, 0);
    }

    bool verificarGanador(char ficha) const {
        for (int fila = 0; fila < 6; ++fila) {
            for (int columna = 0; columna <= 3; ++columna) {
                if (tablero[fila][columna] == ficha &&
                    tablero[fila][columna + 1] == ficha &&
                    tablero[fila][columna + 2] == ficha &&
                    tablero[fila][columna + 3] == ficha) {
                    return true;
                }
            }
        }
        return false;
    }

    std::string toString() const {
        std::string tablero_str;
        for (const auto& fila : tablero) {
            for (char c : fila) {
                tablero_str += c;
                tablero_str += ' ';
            }
            tablero_str += '\n';
        }
        tablero_str += "-------------\n";
        tablero_str += "1 2 3 4 5 6 7\n";
        return tablero_str;
    }
};

int main() {
    int socket_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_servidor == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    sockaddr_in direccion_servidor;
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(7777);
    direccion_servidor.sin_addr.s_addr = INADDR_ANY;

    if (bind(socket_servidor, (struct sockaddr*)&direccion_servidor, sizeof(direccion_servidor)) == -1) {
        std::cerr << "Error al enlazar el socket" << std::endl;
        return 1;
    }

    if (listen(socket_servidor, 5) == -1) {
        std::cerr << "Error al poner el socket en modo escucha" << std::endl;
        return 1;
    }

    std::cout << "Esperando conexión de cliente..." << std::endl;

    sockaddr_in direccion_cliente;
    socklen_t tamano_direccion = sizeof(direccion_cliente);
    int socket_cliente = accept(socket_servidor, (struct sockaddr*)&direccion_cliente, &tamano_direccion);
    if (socket_cliente == -1) {
        std::cerr << "Error al aceptar la conexión del cliente" << std::endl;
        return 1;
    }

    std::cout << "Cliente conectado" << std::endl;

    Tablero tablero;
    srand(time(NULL));

    while (true) {
        

        int columna_cliente;
        int n_bytes = recv(socket_cliente, &columna_cliente, sizeof(columna_cliente), 0);
        if (n_bytes <= 0) {
            std::cerr << "Error al recibir datos del cliente" << std::endl;
            break;
        }

        tablero.mostrarTablero(socket_cliente);

        tablero.agregarFicha(columna_cliente - 1, 'C');
        if (tablero.verificarGanador('C')) {
            tablero.mostrarTablero(socket_cliente);
            send(socket_cliente, "Ganaste", 8, 0);
            break;
        }

        int columna_servidor = rand() % 7 + 1;
        tablero.agregarFicha(columna_servidor - 1, 'S');
        std::string tablero_str = tablero.toString();
        size_t tablero_size = tablero_str.length();
        send(socket_cliente, &tablero_size, sizeof(tablero_size), 0);
        send(socket_cliente, tablero_str.c_str(), tablero_size, 0);
        if (tablero.verificarGanador('S')) {
            tablero.mostrarTablero(socket_cliente);
            send(socket_cliente, "Perdiste", 9, 0);
            break;
        }
    }

    close(socket_cliente);
    close(socket_servidor);
    return 0;
}
