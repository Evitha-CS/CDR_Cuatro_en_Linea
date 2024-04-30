/*
 * servidor.cpp
 * 
 * g++ -std=c++11 testS.cpp -o testS
 *
 * ESTE CÓDIGO ES CREADO CON FORK
 */
#include <sys/socket.h> // socket()
#include <arpa/inet.h>  // hton*()
#include <string.h>     // memset()
#include <unistd.h> 
#include <iostream>
#include <sys/wait.h>
#include <cstdlib> // Para generar números aleatorios
#include <ctime>   // Para la semilla del generador de números aleatorios

using namespace std;
//
void jugar(int socket_cliente, struct sockaddr_in direccionCliente) {
    //
    char buffer[1024];
    memset(buffer, '\0', sizeof(char)*1024);
    int n_bytes = 0;
        
    //
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(direccionCliente.sin_addr), ip, INET_ADDRSTRLEN);
    //
    cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Nuevo jugador." << endl;

    // Generar un número aleatorio para decidir quién comienza
    srand(time(NULL)); // Inicializar la semilla
    int primer_turno = rand() % 2; // 0 para el servidor, 1 para el cliente

    // Envía al cliente quién comienza primero (0 para servidor, 1 para cliente)
    send(socket_cliente, &primer_turno, sizeof(int), 0);

    
    //
    while ((n_bytes = recv(socket_cliente, buffer, 1024, 0))) {
        buffer[n_bytes] = '\0';
                
        //
        if (buffer[0] == 'Q') {
            cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Sale del juego." << endl;
            close(socket_cliente);
            break;
        }
                
        //
        switch (buffer[0]) {
            case 'C':       // C columna
                {
                string line(&buffer[0]);
                cout << "[" << ip << ":" << ntohs(direccionCliente.sin_port) << "] Columna: " << line[2] << endl;
                send(socket_cliente, "ok\n", 3, 0);
                break;
                }
            default:
                // instrucción no reconocida.
                send(socket_cliente, "error\n", 6, 0);
        }
    }
}
//
int main(int argc, char **argv) {
    //
    int port = atoi(argv[1]);
    int socket_server = 0;
    // socket address structures.
    struct sockaddr_in direccionServidor, direccionCliente;   
    
    // crea el socket.
    /*
     * domain: 
     *      AF_ LOCAL-> processes on the same host.
     *      AF_INET -> processes on different hosts connected by IP (AF_INET->IPv4, AF_INET6->IPv6)
     * type:
     *      SOCK_STREAM: TCP (reliable, connection-oriented)
     *      SOCK_DGRAM: UDP (unreliable, connectionless)
     * protocol:
     *      Protocol value for Internet Protocol(IP), which is 0.
     */
    cout << "Creating listening socket ...\n";
    if ((socket_server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        cout << "Error creating listening socket\n";
        exit(EXIT_FAILURE);
    }
    
    // configuracion de los atributos de la clase sockaddr_in.
    cout << "Configuring socket address structure ...\n";
    memset(&direccionServidor, 0, sizeof(direccionServidor));
    direccionServidor.sin_family      = AF_INET;
    direccionServidor.sin_addr.s_addr = htonl(INADDR_ANY);
    direccionServidor.sin_port        = htons(port);
    
    //
    cout << "Binding socket ...\n";
    if (bind(socket_server, (struct sockaddr *) &direccionServidor, sizeof(direccionServidor)) < 0) {
        cout << "Error calling bind()\n";
        exit(EXIT_FAILURE);
    }
    
    //
    cout << "Calling listening ...\n";
    if (listen(socket_server, 1024) < 0) {
        cout << "Error calling listen()\n";
        exit(EXIT_FAILURE);
    }
    
    // para obtener info del cliente.
    socklen_t addr_size;
    addr_size = sizeof(struct sockaddr_in);
    
    //
    cout << "Waiting client request ...\n";
    while (true) {
        /*  Wait for a connection, then accept() it  */
        int socket_cliente;
        
        //
        if ((socket_cliente = accept(socket_server, (struct sockaddr *)&direccionCliente, &addr_size)) < 0) {
            cout << "Error calling accept()\n";
            exit(EXIT_FAILURE);
        }
        
        //
        // Creamos un nuevo proceso hijo para manejar al cliente
        pid_t child_pid = fork();
        if (child_pid == 0) { // Proceso hijo
            close(socket_server); // El hijo no necesita el socket del servidor
            jugar(socket_cliente, direccionCliente);
            exit(EXIT_SUCCESS);
        } else if (child_pid < 0) { // Error
            cout << "Error creating child process\n";
            exit(EXIT_FAILURE);
        } else { // Proceso padre
            close(socket_cliente); // El padre no necesita el socket del cliente
        }
    }
        //jugar(socket_cliente, direccionCliente);
    
    //
    return 0;
}