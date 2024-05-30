#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdlib>

class Tablero {
private:
    std::vector<std::vector<char>> tablero;

public:
    Tablero() : tablero(6, std::vector<char>(7, ' ')) {}

    void mostrarTablero() {
        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 7; ++j) {
                std::cout << "|" << tablero[i][j];
            }
            std::cout << "|\n";
        }
        std::cout << " 1 2 3 4 5 6 7\n";
    }

    std::string toString() {
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

class JuegoCliente {
private:
    int socket_cliente;
    Tablero tablero;
    bool turno_cliente;

public:
    void conectarServidor(const std::string &ip_servidor, int puerto_servidor) {
        struct sockaddr_in direccionServidor;

        if ((socket_cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            std::cerr << "Error creando el socket\n";
            exit(EXIT_FAILURE);
        }

        direccionServidor.sin_family = AF_INET;
        direccionServidor.sin_port = htons(puerto_servidor);

        if (inet_pton(AF_INET, ip_servidor.c_str(), &direccionServidor.sin_addr) <= 0) {
            std::cerr << "Dirección inválida o no soportada\n";
            exit(EXIT_FAILURE);
        }

        if (connect(socket_cliente, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0) {
            std::cerr << "Error en la conexión con el servidor\n";
            exit(EXIT_FAILURE);
        }
    }

    void recibirMatrizYEstado() {
        char buffer[1024];
        memset(buffer, '\0', sizeof(char) * 1024);
        int n_bytes = recv(socket_cliente, buffer, 1024, 0);
        if (n_bytes <= 0) {
            std::cerr << "Error recibiendo la matriz del servidor\n";
            exit(EXIT_FAILURE);
        }

        std::string mensaje(buffer, n_bytes);
        std::string matriz = mensaje.substr(0, 42); // Los primeros 42 caracteres son la matriz
        int estado = mensaje[42] - '0'; // El último carácter es el estado del juego

        tablero.fromString(matriz);
        
        if (estado == 1) {
            system("clear");
            std::cout << "¡Has ganado!" << std::endl;
            close(socket_cliente);
            exit(EXIT_SUCCESS);
        } else if (estado == 2) {
            system("clear");
            std::cout << "¡Has perdido!" << std::endl;
            close(socket_cliente);
            exit(EXIT_SUCCESS);
        }
    }

    void recibirTurnoInicial() {
        int turno_inicial;
        int n_bytes = recv(socket_cliente, &turno_inicial, sizeof(turno_inicial), 0);
        std::cout << turno_inicial<<"Turno inicial"<<std::endl;
        if (n_bytes <= 0) {
            std::cerr << "Error recibiendo el turno inicial del servidor\n";
            exit(EXIT_FAILURE);
        }

        turno_cliente = (turno_inicial == 1);
        if (!turno_cliente) {
            std::cout << "El servidor comienza el juego. Esperando jugada...\n";
            sleep(2);
            system("clear");
            recibirMatrizYEstado();
            turno_cliente = true;
        } else {
            std::cout << "Comienzas tú.\n";
            sleep(2);
            system("clear");
        }
    }

    void enviarColumna(int columna) {
        std::string mensaje = std::to_string(columna);
        send(socket_cliente, mensaje.c_str(), mensaje.size(), 0);
    }

    void jugar() {
        recibirTurnoInicial();
        while (true) {
            if (turno_cliente) {
                tablero.mostrarTablero();
                std::cout << "\nIngresa la columna (1-7): ";
                int columna;
                std::cin >> columna;
                columna--;

                system("clear");
                enviarColumna(columna);
                turno_cliente = false; // Esperar la jugada del servidor
            } else {
                std::cout << "Es el turno del servidor.\n";
                sleep(1);
                recibirMatrizYEstado();
                turno_cliente = true; // Ahora es el turno del cliente
                system("clear");
            }
        }
    }

    ~JuegoCliente() {
        close(socket_cliente);
    }
};

int main() {
    std::string ip_servidor;
    int puerto_servidor;

    std::cout << "Ingresa la dirección IP del servidor: ";
    std::cin >> ip_servidor;
    std::cout << "Ingresa el puerto del servidor: ";
    std::cin >> puerto_servidor;
    system("clear");

    JuegoCliente juego;
    juego.conectarServidor(ip_servidor, puerto_servidor);
    juego.jugar();

    return 0;
}
