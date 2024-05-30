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
        std::cout << "Ficha " << ficha << " agregada en la columna " << columna << std::endl;
        for (int fila = 5; fila >= 0; --fila) {
            if (tablero[fila][columna] == ' ') {
                tablero[fila][columna] = ficha;
                break;
            }
        }
    }

    void mostrarTablero(int socket_cliente) {
        std::cout << "Enviando tablero al cliente." << std::endl;
        std::string tablero_str = toString();
        size_t tablero_size = tablero_str.length();
        send(socket_cliente, &tablero_size, sizeof(tablero_size), 0);
        send(socket_cliente, tablero_str.c_str(), tablero_size, 0);
    }

    bool verificarGanador(char ficha) const {
        std::cout << "Verificando ganador..." << std::endl;
        for (int fila = 0; fila < 6; ++fila) {
            for (int columna = 0; columna <= 3; ++columna) {
                if (tablero[fila][columna] == ficha &&
                    tablero[fila][columna + 1] == ficha &&
                    tablero[fila][columna + 2] == ficha &&
                    tablero[fila][columna + 3] == ficha) {
                    std::cout << "Tenemos un ganador!" << std::endl;
                    return true;
                }
            }
        }
        std::cout << "Nadie ha ganado aún." << std::endl;
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

void enviarMensaje(int socket_cliente, const std::string& mensaje) {
    std::cout << "Enviando un mensaje al cliente. Contenido: " << mensaje << std::endl;
    size_t mensaje_size = mensaje.length();
    send(socket_cliente, &mensaje_size, sizeof(mensaje_size), 0);
    send(socket_cliente, mensaje.c_str(), mensaje_size, 0);
}

std::string recibirMensaje(int socket_cliente) {
    size_t mensaje_size;
    recv(socket_cliente, &mensaje_size, sizeof(mensaje_size), 0);
    std::vector<char> buffer(mensaje_size);
    recv(socket_cliente, buffer.data(), mensaje_size, 0);
    std::cout << "Recibido un mensaje del cliente. Contenido: " << buffer.data() << std::endl;
    return std::string(buffer.data(), mensaje_size);
}

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
    int rondas = 1;

    while (true) {
        std::cout << "Ronda: " << rondas << std::endl;
        // Enviar tablero al cliente
        tablero.mostrarTablero(socket_cliente);

        // Esperar confirmación del cliente
        recibirMensaje(socket_cliente);

        // Verificar si hay un ganador
        if (tablero.verificarGanador('C')) {
            enviarMensaje(socket_cliente, "Ganaste");
            break;
        } else if (tablero.verificarGanador('S')) {
            enviarMensaje(socket_cliente, "Perdiste");
            break;
        } else {
            enviarMensaje(socket_cliente, "El juego continúa...");
        }

        // Preguntar al cliente su siguiente movimiento
        enviarMensaje(socket_cliente, "Tu turno");

        // Recibir movimiento del cliente
        int columna_cliente = -1; // Inicializar con un valor inválido
        recv(socket_cliente, &columna_cliente, sizeof(columna_cliente), 0);

        // Validar columna recibida
        if (columna_cliente < 1 || columna_cliente > 7) {
            std::cerr << "Movimiento inválido recibido: " << columna_cliente << std::endl;
            enviarMensaje(socket_cliente, "Movimiento inválido. Inténtalo de nuevo.");
            continue;
        }

        // Enviar confirmación de movimiento recibido
        enviarMensaje(socket_cliente, "Movimiento recibido");

        // Agregar ficha del cliente al tablero
        tablero.agregarFicha(columna_cliente - 1, 'C');

        // Mostrar tablero con el nuevo movimiento del cliente
        tablero.mostrarTablero(socket_cliente);

        // Esperar confirmación del cliente
        recibirMensaje(socket_cliente);

        // Verificar si hay un ganador
        if (tablero.verificarGanador('C')) {
            enviarMensaje(socket_cliente, "Ganaste");
            break;
        } else if (tablero.verificarGanador('S')) {
            enviarMensaje(socket_cliente, "Perdiste");
            break;
        } else {
            enviarMensaje(socket_cliente, "El juego continúa...");
        }

        // Jugar en una columna al azar que no esté llena
        int columna_servidor;
        std::cout << "Es mi turno" << std::endl;
        do {
            columna_servidor = rand() % 7 + 1;
        } while (columna_servidor < 1 || columna_servidor > 7);
        tablero.agregarFicha(columna_servidor - 1, 'S');

        rondas++;
    }

    close(socket_cliente);
    close(socket_servidor);
    return 0;
}
