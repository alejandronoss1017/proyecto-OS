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

    strcpy(cliente.pipeNom, argv[2]);

    bool terminar = false;
    char opcion;
    do
    {
        printf("MENU SELECCION DE PETICION \n");
        printf("1. Conexion \n");
        printf("2. Seguimiento \n");
        printf("3. Tweet \n");
        printf("0. Salir \n");

        scanf(" %c", &opcion);

        switch (opcion)
        {
        case '1':
            cliente.mensaje.tipo = CONEXION;
            cliente.mensaje.conexion.status = 1;
            strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);
            unlink(cliente.pipeNom);
            mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);
            // Primero el write antes del open

            fprintf(stderr, "%d\n", cliente.mensaje.tipo);
            fprintf(stderr, "%s\n", cliente.pipeNom);

            write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
            printf("Enviado solicitud\n");

            if ((fdEspecifico = open(cliente.pipeNom, O_RDONLY)) == -1)
            {
                perror("Error");
                exit(1);
            }
            struct SMensaje temporal;

            int leido;
            printf("%d", fdEspecifico);
            if ((leido = read(fdEspecifico, &temporal, sizeof(struct SMensaje))) < 0)
            {
                perror("Error");
                exit(1);
            }
            else
            {
                fprintf(stderr, "Respuesta del servidor\n");
            }

            if (temporal.conexion.exito == 1)
            {
                printf("Fue exitosa la conexion\n");
            }
            else
            {
                printf("Conexion fallida\n");
            }

            break;
        case '2':
            cliente.mensaje.tipo = 2;
            break;
        case '3':
            cliente.mensaje.tipo = 3;
            break;
        case '0':
            terminar = true;
            break;
        default:
            printf("Seleccione una de las opciones establecidas \n");

            break;
        }
    } while (terminar != true);

    return 0;
}
