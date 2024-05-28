#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <vector>

class Tablero {
private:
    std::vector<std::vector<char>> tablero;

public:
    Tablero() : tablero(6, std::vector<char>(7, ' ')) {}

    char getFicha(int fila, int columna) {
        return tablero[fila][columna];
    }

    void agregarFicha(int columna, char ficha) {
        for (int fila = 5; fila >= 0; --fila) {
            if (tablero[fila][columna] == ' ') {
                tablero[fila][columna] = ficha;
                break;
            }
        }
    }

    bool verificarGanador(char ficha) {
        // Verificar filas
        for (int fila = 0; fila < 6; ++fila) {
            for (int columna = 0; columna < 4; ++columna) {
                if (tablero[fila][columna] == ficha && tablero[fila][columna + 1] == ficha &&
                    tablero[fila][columna + 2] == ficha && tablero[fila][columna + 3] == ficha) {
                    return true;
                }
            }
        }

        // Verificar columnas
        for (int columna = 0; columna < 7; ++columna) {
            for (int fila = 0; fila < 3; ++fila) {
                if (tablero[fila][columna] == ficha && tablero[fila + 1][columna] == ficha &&
                    tablero[fila + 2][columna] == ficha && tablero[fila + 3][columna] == ficha) {
                    return true;
                }
            }
        }

        // Verificar diagonales
        for (int fila = 0; fila < 3; ++fila) {
            for (int columna = 0; columna < 4; ++columna) {
                if (tablero[fila][columna] == ficha && tablero[fila + 1][columna + 1] == ficha &&
                    tablero[fila + 2][columna + 2] == ficha && tablero[fila + 3][columna + 3] == ficha) {
                    return true;
                }
            }
        }

        for (int fila = 3; fila < 6; ++fila) {
            for (int columna = 0; columna < 4; ++columna) {
                if (tablero[fila][columna] == ficha && tablero[fila - 1][columna + 1] == ficha &&
                    tablero[fila - 2][columna + 2] == ficha && tablero[fila - 3][columna + 3] == ficha) {
                    return true;
                }
            }
        }

        return false;
    }

    std::string toString() const {
        std::string matriz;
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 7; ++j) {
                matriz += tablero[i][j];
            }
        }
        return matriz;
    }

    void fromString(const std::string &matriz) {
        for (int i = 0; i < 42; ++i) {
            tablero[i / 7][i % 7] = matriz[i];
        }
    }
};

void elegirTurno(int socket_cliente) {
    srand(time(NULL));
    int primer_turno = rand() % 2;
    send(socket_cliente, &primer_turno, sizeof(int), 0);
}

void enviarMatrizYEstado(int socket_cliente, const Tablero &tablero, int estado) {
    std::string matriz = tablero.toString();
    matriz += std::to_string(estado); // Añadir el estado al final de la matriz
    send(socket_cliente, matriz.c_str(), matriz.size(), 0);
}

void jugar(int socket_cliente, struct sockaddr_in direccionCliente) {
    char buffer[1024];
    memset(buffer, '\0', sizeof(char) * 1024);

    Tablero tablero;
    int turno = 0; // 0 para el servidor, 1 para el cliente
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionCliente.sin_addr), ip, INET_ADDRSTRLEN);
    std::cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Nuevo jugador." << std::endl;

    elegirTurno(socket_cliente);
    //recv(socket_cliente, &turno, sizeof(turno), 0);
    enviarMatrizYEstado(socket_cliente, tablero, 0);

    while (true) {
        if (turno == 1) {
            int n_bytes = recv(socket_cliente, buffer, 1024, 0);
            if (n_bytes <= 0) {
                std::cerr << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Cliente desconectado.\n";
                close(socket_cliente);
                exit(EXIT_FAILURE);
            }

            int columna = buffer[0] - '0';
            tablero.agregarFicha(columna, 'C');

            if (tablero.verificarGanador('C')) {
                enviarMatrizYEstado(socket_cliente, tablero, 1);
                std::cout << "El cliente ha ganado.\n";
                close(socket_cliente);
                exit(EXIT_SUCCESS);
            }
            turno = 0;
        } else {
            srand(time(NULL));
            int columna;
            do {
                columna = rand() % 7;
            } while (tablero.getFicha(0, columna) != ' ');

            tablero.agregarFicha(columna, 'S');

            if (tablero.verificarGanador('S')) {
                enviarMatrizYEstado(socket_cliente, tablero, 2);
                std::cout << "El servidor ha ganado.\n";
                close(socket_cliente);
                exit(EXIT_SUCCESS);
            }
            enviarMatrizYEstado(socket_cliente, tablero, 0);
            turno = 1;
        }
    }
}

int main(int argc, char **argv) {
    int port = atoi(argv[1]);
    int socket_server = 0;
    struct sockaddr_in direccionServidor, direccionCliente;

    std::cout << "Creando socket de escucha...\n";
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Error creando el socket\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Configurando la estructura de la dirección del socket...\n";
    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port = htons(port);

    std::cout << "Llamando a bind()...\n";
    if (bind(socket_server, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
        std::cerr << "Error en bind()\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Llamando a listen()...\n";
    if (listen(socket_server, 1024) < 0) {
        std::cerr << "Error en listen()\n";
        exit(EXIT_FAILURE);
    }

    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);

    std::cout << "Esperando solicitudes de clientes...\n";
    while (true) {
        int socket_cliente;

        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&direccionCliente, &addr_size)) < 0) {
            std::cerr << "Error en accept()\n";
            exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();
        if (child_pid == 0) { // Proceso hijo
            close(socket_server);
            jugar(socket_cliente, direccionCliente);
            exit(EXIT_SUCCESS);
        } else if (child_pid < 0) { // Error
            std::cerr << "Error creando el proceso hijo\n";
            exit(EXIT_FAILURE);
        } else { // Proceso padre
            close(socket_cliente);
        }
    }

    return 0;
}
