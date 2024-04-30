#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#include <vector>

void elegirTurno(int socket_cliente)
{
    srand(time(NULL));             // Inicializar la semilla
    int primer_turno = rand() % 2; // 0 para el servidor, 1 para el cliente
    send(socket_cliente, &primer_turno, sizeof(int), 0);
}

void jugar(int socket_cliente, struct sockaddr_in direccionCliente)
{
    char buffer[1024];
    memset(buffer, '\0', sizeof(char) * 1024);
    int n_bytes = 0;

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionCliente.sin_addr), ip, INET_ADDRSTRLEN);
    std::cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Nuevo jugador." << std::endl;

    while (true)
    {
        if (recv(socket_cliente, buffer, 1024, 0) <= 0)
        {
            std::cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Sale del juego." << std::endl;
            close(socket_cliente);
            exit(EXIT_SUCCESS);
        }

        std::string matriz(buffer);

        // Verificar si la matriz está completamente vacía
        bool matriz_vacia = true;
        for (char c : matriz)
        {
            if (c != ' ')
            {
                matriz_vacia = false;
                break;
            }
        }

        // Si la matriz está completamente vacía, coloca una "S" en una posición aleatoria
        if (matriz_vacia)
        {
            int indice_aleatorio = rand() % matriz.size();
            matriz[indice_aleatorio] = 'S';
        }
        else
        {
            // Si hay al menos una ficha "C", coloca una "S" en una posición aleatoria vacía
            std::vector<int> posiciones_vacias;
            for (int i = 0; i < matriz.size(); ++i)
            {
                if (matriz[i] == ' ')
                {
                    posiciones_vacias.push_back(i);
                }
            }

            if (!posiciones_vacias.empty())
            {
                int indice_aleatorio = rand() % posiciones_vacias.size();
                matriz[posiciones_vacias[indice_aleatorio]] = 'S';
            }
        }

        send(socket_cliente, matriz.c_str(), matriz.size(), 0);
    }
}

int main(int argc, char **argv)
{
    int port = atoi(argv[1]);
    int socket_server = 0;
    struct sockaddr_in direccionServidor, direccionCliente;

    std::cout << "Creating listening socket ...\n";
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Error creando el socket\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Configuring socket address structure ...\n";
    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port = htons(port);

    std::cout << "Binding socket ...\n";
    if (bind(socket_server, (struct sockaddr *)&direccionServidor, sizeof(direccionServidor)) < 0)
    {
        std::cerr << "Error llamando a bind()\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "Calling listen ...\n";
    if (listen(socket_server, 1024) < 0)
    {
        std::cerr << "Error llamando a listen()\n";
        exit(EXIT_FAILURE);
    }

    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);

    std::cout << "Waiting for client request ...\n";
    while (true)
    {
        int socket_cliente;

        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&direccionCliente, &addr_size)) < 0)
        {
            std::cerr << "Error llamando a accept()\n";
            exit(EXIT_FAILURE);
        }

        pid_t child_pid = fork();
        if (child_pid == 0)
        { // Proceso hijo
            close(socket_server);
            elegirTurno(socket_cliente);
            jugar(socket_cliente, direccionCliente);
            exit(EXIT_SUCCESS);
        }
        else if (child_pid < 0)
        { // Error
            std::cerr << "Error creando el proceso hijo\n";
            exit(EXIT_FAILURE);
        }
        else
        { // Proceso padre
            close(socket_cliente);
        }
    }

    return 0;
}
