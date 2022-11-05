#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "scliente.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "colors.h"

// Declaracion de la firma de las distintas funciones
void realizarConexion(struct SCliente cliente, int fdGeneral, int fdEspecifico);
void enviarTweet(struct SCliente cliente, int fdGeneral, int fdEspecifico);

int main(int argc, char **argv)
{
    /*
     * 1. Nombre del pipe general creado por el gestor
     * 2. Nombre de otro pipe?
     */
    int fdGeneral;
    int fdEspecifico;
    fdGeneral = open(argv[1], O_WRONLY);

    printf("PIpe general abierto\n");

    if (fdGeneral == -1)
    {
        perror("pipe: ");
        exit(0);
    }

    struct SCliente cliente;

    // Inicializamos el ID y el idEmisor del cliente con el del proceso
    cliente.idCliente = (int)getpid();
    cliente.mensaje.idEmisor = cliente.idCliente;

    strcpy(cliente.pipeNom, argv[2]);

    bool terminar = false;
    char opcion;
    do
    {
        // System(clear) no funciona en el IDE pero en la consola perfecto
        system("clear");
        printf("MENU SELECCION DE PETICION \n");
        printf("1. Conexion \n");
        printf("2. Seguimiento \n");
        printf("3. Tweet \n");
        printf("0. Salir \n");

        scanf(" %c", &opcion);
        fgetc(stdin);

        switch (opcion)
        {
        /*
            El caso de CONEXION realiza...
        */
        case '1':
            realizarConexion(cliente, fdGeneral, fdEspecifico);
            break;
        case '2':
            cliente.mensaje.tipo = 2;
            break;
        case '3':
            /*
                El caso TWEET va a crear el pipe del cliente para comunicarse con el
                gestor, este escribira un mensaje por pantalla que posteriormente
                sera enviado al gestor.
            */
            enviarTweet(cliente, fdGeneral, fdEspecifico);

            break;
        case '0':
            // Cierra y elimina el pipe
            close(cliente.pipeNom);
            unlink(cliente.pipeNom);
            terminar = true;
            break;
        default:
            printf("Seleccione una de las opciones establecidas \n");

            break;
        }
    } while (terminar != true);

    return 0;
}

void realizarConexion(struct SCliente cliente, int fdGeneral, int fdEspecifico)
{
    cliente.mensaje.tipo = CONEXION;
    cliente.mensaje.conexion.status = 1;
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);
    unlink(cliente.pipeNom);
    mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

    // Primero el write antes del open

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Solicitud enviada\n");

    if ((fdEspecifico = open(cliente.pipeNom, O_RDONLY)) == -1)
    {
        perror("Error");
        exit(1);
    }
    struct SMensaje temporal;

    int leido;

    if ((leido = read(fdEspecifico, &temporal, sizeof(struct SMensaje))) < 0)
    {
        perror("Error");
        exit(1);
    }
    else
    {
        fprintf(stderr, "Respuesta del servidor\n");
        sleep(3);
    }

    if (temporal.conexion.exito == 1)
    {
        printf("Fue exitosa la conexion\n");
        sleep(3);
    }
    else
    {
        printf("Conexion fallida\n");
        sleep(3);
    }
    sleep(3);
}

void enviarTweet(struct SCliente cliente, int fdGeneral, int fdEspecifico)
{
    // Envio del nombre del pipeEspecifico por el mensaje
    // Sigue siendo necesario la opcion de CONEXION?
    cliente.mensaje.tipo = TWEET;
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);
    system("clear");

    unlink(cliente.pipeNom);
    mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

    printf("Escriba su tweet acontinuacion:\n");
    fgets(cliente.mensaje.tweet.mensaje, 200, stdin);

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Tweet enviado\n");
    sleep(3);
}
