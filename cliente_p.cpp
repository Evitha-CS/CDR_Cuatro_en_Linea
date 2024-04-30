#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <cstdlib>

class Tablero
{
private:
    std::vector<std::vector<char>> tablero;

public:
    Tablero() : tablero(6, std::vector<char>(7, ' ')) {}

    void agregarFicha(int columna, char ficha)
    {
        for (int fila = 5; fila >= 0; --fila)
        {
            if (tablero[fila][columna] == ' ')
            {
                tablero[fila][columna] = ficha;
                break;
            }
        }
    }

    void mostrarTablero()
    {

        for (int fila = 0; fila < 6; ++fila)
        {
            std::cout << fila + 1 << " "; // Número de fila
            for (int columna = 0; columna < 7; ++columna)
            {
                std::cout << tablero[fila][columna] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "  -------------" << std::endl;
        std::cout << "  1 2 3 4 5 6 7" << std::endl; // Encabezado de las columnas
    }

    char getFicha(int fila, int columna) const
    {
        return tablero[fila][columna];
    }

    std::string toString() const
    {
        std::string matriz;
        for (const auto &fila : tablero)
        {
            for (char c : fila)
            {
                matriz += c;
            }
        }
        return matriz;
    }
};

class JuegoCliente
{
private:
    Tablero tablero;
    int socket_cliente;
    bool comienza_primero;

public:
    JuegoCliente() : socket_cliente(0), comienza_primero(false) {}

    void conectarServidor(const std::string &ip, int puerto)
    {
        struct sockaddr_in direccionServidor;

        if ((socket_cliente = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            std::cerr << "Error creando el socket\n";
            exit(EXIT_FAILURE);
        }

        memset(&direccionServidor, 0, sizeof(direccionServidor));
        direccionServidor.sin_family = AF_INET;
        direccionServidor.sin_port = htons(puerto);

        if (inet_pton(AF_INET, ip.c_str(), &direccionServidor.sin_addr) <= 0)
        {
            std::cerr << "Dirección IP inválida\n";
            exit(EXIT_FAILURE);
        }

        if (connect(socket_cliente, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0)
        {
            std::cerr << "Error de conexión\n";
            exit(EXIT_FAILURE);
        }
    }

    void recibirTurno()
    {
        int primer_turno;
        int bytes_recibidos = recv(socket_cliente, &primer_turno, sizeof(int), 0);
        if (bytes_recibidos == -1)
        {
            std::cerr << "Error al recibir el turno del servidor" << std::endl;
            exit(EXIT_FAILURE);
        }
        else if (bytes_recibidos == 0)
        {
            std::cerr << "El servidor ha cerrado la conexión" << std::endl;
            exit(EXIT_FAILURE);
        }
        else
        {
            comienza_primero = (primer_turno == 1); // 1 para cliente, 0 para servidor
            std::cout << "El turno es para ";
            if (comienza_primero)
            {
                std::cout << "el cliente." << std::endl;
                sleep(3);
            }
            else
            {
                std::cout << "el servidor." << std::endl;
                sleep(3);
                enviarMatriz();
            }
        }
    }

    void jugar()
    {
        while (true)
        {
            if (comienza_primero)
            {
                turnoCliente();
                if (verificarGanador('C'))
                {
                    std::cout << "¡Has ganado!" << std::endl;
                    break;
                }
                turnoServidor();
                if (verificarGanador('S'))
                {
                    std::cout << "¡Has perdido!" << std::endl;
                    break;
                }
            }
            else
            {
                turnoServidor();
                if (verificarGanador('S'))
                {
                    std::cout << "¡Has perdido!" << std::endl;
                    break;
                }
                turnoCliente();
                if (verificarGanador('C'))
                {
                    std::cout << "¡Has ganado!" << std::endl;
                    break;
                }
            }
        }
    }

    void turnoCliente()
    {
        int columna;
        tablero.mostrarTablero();
        std::cout << "\nIngresa la columna (1-7): ";
        std::cin >> columna;
        columna--;
        system("clear");

        if (columna >= 0 && columna < 7 && tablero.getFicha(0, columna) == ' ')
        {
            tablero.agregarFicha(columna, 'C');
            tablero.mostrarTablero(); // Imprimir tablero después de la jugada del cliente
            enviarMatriz();
        }
        else
        {
            std::cout << "\nMovimiento no válido. Inténtalo de nuevo." << std::endl;
            turnoCliente(); // Pedir al cliente que intente nuevamente
        }
    }

    void turnoServidor()
    {
        recibirMatriz();
        system("clear");
        tablero.mostrarTablero();
        std::cout << "\nEl servidor puso una ficha.\n" << std::endl;
        sleep(2);
        system("clear");
    }

    bool verificarGanador(char ficha)
    {
        // Verificar horizontalmente
        for (int fila = 0; fila < 6; ++fila)
        {
            for (int columna = 0; columna <= 3; ++columna)
            {
                if (tablero.getFicha(fila, columna) == ficha &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila, columna + 1) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila, columna + 2) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila, columna + 3))
                {
                    return true;
                }
            }
        }
        // Verificar verticalmente
        for (int fila = 0; fila <= 2; ++fila)
        {
            for (int columna = 0; columna < 7; ++columna)
            {
                if (tablero.getFicha(fila, columna) == ficha &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 1, columna) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 2, columna) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 3, columna))
                {
                    return true;
                }
            }
        }
        // Verificar diagonalmente (de izquierda a derecha)
        for (int fila = 0; fila <= 2; ++fila)
        {
            for (int columna = 0; columna <= 3; ++columna)
            {
                if (tablero.getFicha(fila, columna) == ficha &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 1, columna + 1) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 2, columna + 2) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 3, columna + 3))
                {
                    return true;
                }
            }
        }
        // Verificar diagonalmente (de derecha a izquierda)
        for (int fila = 0; fila <= 2; ++fila)
        {
            for (int columna = 3; columna < 7; ++columna)
            {
                if (tablero.getFicha(fila, columna) == ficha &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 1, columna - 1) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 2, columna - 2) &&
                    tablero.getFicha(fila, columna) == tablero.getFicha(fila + 3, columna - 3))
                {
                    return true;
                }
            }
        }
        return false;
    }

    void enviarMatriz()
    {
        std::string matriz = tablero.toString();
        send(socket_cliente, matriz.c_str(), matriz.size(), 0);
    }

    void recibirMatriz()
    {
        char buffer[1024];
        memset(buffer, '\0', sizeof(char) * 1024);
        int n_bytes = recv(socket_cliente, buffer, 1024, 0);
        if (n_bytes <= 0)
        {
            std::cerr << "Error recibiendo la matriz del servidor\n";
            exit(EXIT_FAILURE);
        }
        tablero = Tablero();
        for (int i = 0; i < n_bytes; ++i)
        {
            tablero.agregarFicha(i % 7, buffer[i]);
        }
        tablero.mostrarTablero();
    }

    ~JuegoCliente()
    {
        close(socket_cliente);
    }
};

int main()
{
    std::string ip_servidor;
    int puerto_servidor;

    std::cout << "Ingresa la dirección IP del servidor: ";
    std::cin >> ip_servidor;
    std::cout << "Ingresa el puerto del servidor: ";
    std::cin >> puerto_servidor;
    system("clear");

    JuegoCliente juego;
    juego.conectarServidor(ip_servidor, puerto_servidor);
    juego.recibirTurno();
    juego.jugar();

    return 0;
}
