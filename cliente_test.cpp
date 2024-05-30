#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <limits>

class Tablero {
public:
    Tablero(int socket) : socket_cliente(socket) {}

    void mostrarTablero() {
        size_t tablero_size;
        int n_bytes = recv(socket_cliente, &tablero_size, sizeof(tablero_size), 0);
        if (n_bytes <= 0) {
            std::cerr << "Error al recibir el tamaño del tablero del servidor" << std::endl;
            return;
        }
        std::vector<char> tablero_buffer(tablero_size);
        n_bytes = recv(socket_cliente, tablero_buffer.data(), tablero_size, 0);
        if (n_bytes <= 0) {
            std::cerr << "Error al recibir el tablero del servidor" << std::endl;
            return;
        }
        std::string tablero_str(tablero_buffer.data(), tablero_size);
        std::cout << tablero_str << std::endl;
    }

    void enviarMovimiento(int columna) {
        send(socket_cliente, &columna, sizeof(columna), 0);
    }

    void enviarConfirmacion(const std::string& mensaje) {
        size_t mensaje_size = mensaje.length();
        send(socket_cliente, &mensaje_size, sizeof(mensaje_size), 0);
        send(socket_cliente, mensaje.c_str(), mensaje_size, 0);
    }

    std::string recibirMensaje() {
        size_t mensaje_size;
        recv(socket_cliente, &mensaje_size, sizeof(mensaje_size), 0);
        std::vector<char> buffer(mensaje_size);
        recv(socket_cliente, buffer.data(), mensaje_size, 0);
        return std::string(buffer.data(), mensaje_size);
    }

private:
    int socket_cliente; // Descriptor del socket del cliente
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <ip_servidor> <puerto>" << std::endl;
        return 1;
    }

    int socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_cliente == -1) {
        std::cerr << "Error al crear el socket" << std::endl;
        return 1;
    }

    sockaddr_in direccion_servidor;
    direccion_servidor.sin_family = AF_INET;
    direccion_servidor.sin_port = htons(std::stoi(argv[2]));
    inet_pton(AF_INET, argv[1], &direccion_servidor.sin_addr);

    if (connect(socket_cliente, (struct sockaddr*)&direccion_servidor, sizeof(direccion_servidor)) == -1) {
        std::cerr << "Error al conectar con el servidor" << std::endl;
        return 1;
    }

    Tablero tablero(socket_cliente);

    while (true) {
        // Ver el tablero
        tablero.mostrarTablero();

        // Enviar confirmación de que se vio el tablero
        tablero.enviarConfirmacion("Tablero visto");

        // Recibir mensaje del servidor
        std::string mensaje = tablero.recibirMensaje();
        if (mensaje == "Ganaste" || mensaje == "Perdiste") {
            std::cout << mensaje << std::endl;
            break;
        }

        // Recibir turno del servidor
        mensaje = tablero.recibirMensaje();
        if (mensaje == "Tu turno") {
            int columna = 0;
            std::cout << "Tu turno, elige una columna (1-7): ";
            while (!(std::cin >> columna) || columna < 1 || columna > 7) {
                std::cin.clear(); // Limpiar el estado de cin
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignorar la entrada incorrecta
                std::cout << "Movimiento no válido. Inténtalo de nuevo." << std::endl;
                std::cout << "Tu turno, elige una columna (1-7): ";
            }
            std::cout << "Enviando columna: " << columna << std::endl;
            tablero.enviarMovimiento(columna);
        }

        // Enviar confirmación de que se recibió el movimiento
        tablero.enviarConfirmacion("Movimiento visto");

        // Ver el tablero actualizado
        tablero.mostrarTablero();

        // Enviar confirmación de que se vio el tablero
        tablero.enviarConfirmacion("Tablero visto");

        // Recibir mensaje del servidor
        mensaje = tablero.recibirMensaje();
        if (mensaje == "Ganaste" || mensaje == "Perdiste") {
            std::cout << mensaje << std::endl;
            break;
        }
    }

    close(socket_cliente);
    return 0;
}
