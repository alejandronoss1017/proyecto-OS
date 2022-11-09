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
#include <locale.h>

// Declaración de la firma de las distintas funciones
void realizarConexion(struct SCliente cliente, int fdGeneral, int fdEspecifico);
void enviarTweet(struct SCliente cliente, int fdGeneral, int fdEspecifico);
void follow(struct SCliente cliente, int fdGeneral, int fdEspecifico);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    /*
     * 1. Nombre del pipe general creado por el gestor
     * 2. Nombre de otro pipeEspecifico
     * 3. Nombre del usuario
     */
    int fdGeneral;
    int fdEspecifico;
    int idSeguir;

    int opt;

    struct SCliente cliente;

    /*
        Implementación de las flags del para la inicializarían del programa

        TODO: Caso donde haga falta uno de los argumento, es decir tiene que ser los 3 obligados

        http://www.catb.org/~esr/writings/taoup/html/ch10s05.html
    */
    while ((opt = getopt(argc, argv, "i:b:u:")) != -1)
    {
        switch (opt)
        {
        // Caso -o, significa que hay Buffer or block size
        case 'b':
            // Intenta abrir el pipe que se pasa después de la flag de -b
            printf("PipeGeneral : %s\n", optarg);
            fdGeneral = open(optarg, O_WRONLY);
            if (fdGeneral == -1)
            {
                perror("pipe: ");
                exit(0);
            }
            break;
        // Caso -i, significa que es inicializar
        case 'i':
            // Inicializamos el processId y el processIdEmisor del cliente con el del proceso
            strcpy(cliente.pipeNom, optarg);
            cliente.processId = (int)getpid();
            cliente.mensaje.processIdEmisor = cliente.processId;
            printf("pipeEspecifico : %s\n", optarg);
            break;
        // Caso -u, significa usuario
        case 'u':
            strcpy(cliente.nombreUsuario, optarg);
            printf("usuario : %s\n", optarg);
            break;

        case ':':
            printf("La opción %d requiere un argumento\n", opt);
            exit(1);
            break;
        case '?':
            printf("unknown option: %c\n", optopt);
            break;
        }
    }

    sleep(2);

    bool terminar = false;
    char opcion;
    char opcion2;
    system("clear");

    printf("===================================================================================== \n");
    printf(AZUL_T "     ________  __       __  ______  ________  ________  ________  _______   \n" RESET_COLOR);
    printf(AZUL_T "    /        |/  |  _  /  |/      |/        |/        |/        |/       \  \n" RESET_COLOR);
    printf(AZUL_T "    $$$$$$$$/ $$ | / \ $$ |$$$$$$/ $$$$$$$$/ $$$$$$$$/ $$$$$$$$/ $$$$$$$  | \n" RESET_COLOR);
    printf(AZUL_T "       $$ |   $$ |/$  \$$ |  $$ |     $$ |      $$ |   $$ |__    $$ |__$$ | \n" RESET_COLOR);
    printf(AZUL_T "       $$ |   $$ /$$$  $$ |  $$ |     $$ |      $$ |   $$    |   $$    $$<  \n" RESET_COLOR);
    printf(AZUL_T "       $$ |   $$ $$/$$ $$ |  $$ |     $$ |      $$ |   $$$$$/    $$$$$$$  | \n" RESET_COLOR);
    printf(AZUL_T "       $$ |   $$$$/  $$$$ | _$$ |_    $$ |      $$ |   $$ |_____ $$ |  $$ | \n" RESET_COLOR);
    printf(AZUL_T "       $$ |   $$$/    $$$ |/ $$   |   $$ |      $$ |   $$       |$$ |  $$ | \n" RESET_COLOR);
    printf(AZUL_T "       $$/    $$/      $$/ $$$$$$/    $$/       $$/    $$$$$$$$/ $$/   $$/  \n" RESET_COLOR);
    printf(AZUL_T "                                                                           \n" RESET_COLOR);
    printf("===================================================================================== \n");
    printf("Desea realizar la solicitud de conexion? \n");
    scanf("%c", &opcion2);
    if (opcion2 == 's')
    {
        realizarConexion(cliente, fdGeneral, fdEspecifico);
        system("clear");
        do
        {
            printf("MENU SELECCIÓN DE PETICIÓN \n");
            printf("1. Seguimiento \n");
            printf("2. Tweet \n");
            printf("0. Salir \n");

            scanf(" %c", &opcion);
            fgetc(stdin);

            switch (opcion)
            {
            /*
                El caso de CONEXION realiza...
            */
            case '1':
                follow(cliente, fdGeneral, fdEspecifico);
                break;
            case '2':
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
    }
    else
    {
        printf("No se puede realizar ninguna solicitud, buen dia. \n");
    }

    return 0;
}

void realizarConexion(struct SCliente cliente, int fdGeneral, int fdEspecifico)
{
    cliente.mensaje.tipo = CONEXION;
    cliente.mensaje.conexion.status = 1;
    strcpy(cliente.mensaje.nombreUsuario, cliente.nombreUsuario);
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);

    // Primero el write antes del open

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Solicitud enviada\n");

    unlink(cliente.pipeNom);
    mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

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

void follow(struct SCliente cliente, int fdGeneral, int fdEspecifico)
{
    cliente.mensaje.tipo = SEGUIMIENTO;
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);
    system("clear");

    unlink(cliente.pipeNom);
    mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

    printf("Escriba el identificador del usuario que desee seguir\n");
    scanf("%d\n", cliente.mensaje.seguimiento.idReceptor);
    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Solicitud enviada\n");
    sleep(3);
}

void enviarTweet(struct SCliente cliente, int fdGeneral, int fdEspecifico)
{
    // Envió del nombre del pipeEspecifico por el mensaje
    // Sigue siendo necesario la opcion de CONEXION?
    cliente.mensaje.tipo = TWEET;
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);
    system("clear");

    // unlink(cliente.pipeNom);
    // mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

    printf("Escriba su tweet a continuación:\n");
    fgets(cliente.mensaje.tweet.mensaje, 200, stdin);

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Tweet enviado\n");
    sleep(3);
}
