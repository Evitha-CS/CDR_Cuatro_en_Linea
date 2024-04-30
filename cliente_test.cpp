#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

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

    Tablero tablero(socket_cliente); // Pasamos el descriptor del socket al constructor de Tablero

    while (true) {
        tablero.mostrarTablero();
        int columna;
        std::cout << "Tu turno, elige una columna (1-7): ";
        std::cin >> columna;
        while (columna < 1 || columna > 7) {
            std::cout << "Movimiento no válido. Inténtalo de nuevo." << std::endl;
            std::cout << "Tu turno, elige una columna (1-7): ";
            std::cin >> columna;
        }
        tablero.enviarMovimiento(columna);
    }

    close(socket_cliente);
    return 0;
}
