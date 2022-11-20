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

struct SCliente cliente;

/* Variables GLobales */
int fdGeneral;
int fdEspecifico;
struct SMensaje temporal;
char modoGestor;
int contTweets = 0;

// Declaración de la firma de las distintas funciones
void realizarConexion();
void enviarTweet();
void follow();
void unfollow();
void leerPipeEspecifico();
void leerRespuestaFollow();
void leerRespuestaUnfollow();

/*
 * 1. Nombre del pipe general creado por el gestor
 * 2. Nombre de otro pipeEspecifico
 * 3. Nombre del usuario
 */
int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    int idSeguir;

    pthread_t hilo;

    int opt;

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
        realizarConexion();
        leerRespuestaConexion();
        // Creacion del hilo
        pthread_create(&hilo, NULL, &leerPipeEspecifico, NULL);
        do
        {
            system("clear");
            printf("Id asignado: %d\n", cliente.idCliente);
            printf("===================================================================================== \n");
            printf("MENU SELECCIÓN DE PETICIÓN \n");
            printf("1. Follow \n");
            printf("2. Unfollow \n");
            printf("3. Tweet \n");
            if (modoGestor == 'D')
            {
                printf("4. Ver Tweets por leer \n");
            }

            printf("0. Salir \n");

            scanf(" %c", &opcion);
            fgetc(stdin);

            switch (opcion)
            {
            /*
                El caso de CONEXION realiza...
            */
            case '1':
                follow();
                sleep(7);
                break;
            case '2':
                unfollow();
                sleep(7);
                break;
            case '3':
                enviarTweet();
                sleep(7);
                break;
            case '4':
                verTweetsPorLeer();
                sleep(7);
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

void realizarConexion()
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
}

void leerRespuestaConexion()
{
    int leido;

    if ((leido = read(fdEspecifico, &temporal, sizeof(struct SMensaje))) < 0)
    {
        perror("Error:");
        exit(1);
    }

    if (temporal.conexion.exito == 1)
    {
        modoGestor = temporal.conexion.modoGestor;
        cliente.idCliente = temporal.conexion.idRetorno;
        cliente.fd = temporal.conexion.fdRetorno;
        printf("Fue exitosa la conexion\n");
    }
    else
    {
        printf("Conexion fallida\n");
    }
}

void follow()
{
    cliente.mensaje.tipo = SEGUIMIENTO;
    cliente.mensaje.seguimiento.status = 1;
    cliente.mensaje.idEmisor = cliente.idCliente;
    cliente.mensaje.seguimiento.fdSeguir = cliente.fd;

    fprintf(stderr, "Escriba el identificador del usuario que desee seguir\n");

    scanf("%d", &cliente.mensaje.seguimiento.idReceptor);

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Solicitud enviada\n");
}

void leerRespuestaFollow()
{
    if (temporal.seguimiento.exito == 1)
    {
        printf("Ahora estas siguiendo al usuario: %d \n", cliente.mensaje.seguimiento.idReceptor);
    }
    else if (temporal.seguimiento.exito == 2)
    {
        printf("Ya sigues a este usuario! \n");
    }
    else
    {
        printf("Operación de seguimiento fallida\n");
    }
}

void unfollow()
{
    cliente.mensaje.tipo = SEGUIMIENTO;
    cliente.mensaje.seguimiento.status = 0;

    if (temporal.seguimiento.exito == 1)
    {
        printf("Dejaste de seguir al usuario: %d \n", cliente.mensaje.seguimiento.idReceptor);
    }
    else if (temporal.seguimiento.exito == 2)
    {
        printf("No  sigues a este usuario! \n");
    }
    else
    {
        printf("Operación de seguimiento fallida\n");
    }
}

void leerRespuestaUnfollow()
{
    int leido;

    if (temporal.seguimiento.exito == 1)
    {
        printf("Dejaste de seguir al usuario: %d \n", cliente.mensaje.seguimiento.idReceptor);
    }
    else if (temporal.seguimiento.exito == 2)
    {
        printf("No  sigues a este usuario! \n");
    }
    else
    {
        printf("Operación de seguimiento fallida\n");
    }
}

void enviarTweet()
{
    // Envió del nombre del pipeEspecifico por el mensaje
    // Sigue siendo necesario la opcion de CONEXION?
    cliente.mensaje.tipo = TWEET;
    cliente.mensaje.idEmisor = cliente.idCliente;
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);

    // unlink(cliente.pipeNom);
    // mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

    printf("Escriba su tweet a continuación:\n");
    fgets(cliente.mensaje.tweet.mensaje, 200, stdin);

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf("Tweet enviado\n");
}

void leerTweet()
{
    printf("Tweet de usuario: %d \n", temporal.tweet.idEmisor);
    printf("Mensaje: \n");
    printf("%s \n", temporal.tweet.mensaje);
}

void verTweetsPorLeer()
{
    if (contTweets == 0)
    {
        printf("NO tiene nuevos tweets por leer \n");
    }

    for (int i = 0; i < contTweets; i++)
    {
        printf("Tweet de usuario: %d \n", cliente.tweetsPorLeer[i].idEmisor);
        printf("Mensaje: \n");
        printf("%s \n", cliente.tweetsPorLeer[i].mensaje);
    }
}

void leerPipeEspecifico()
{
    while (true)
    {
        int leido;
        leido = read(fdEspecifico, &temporal, sizeof(struct SMensaje));
        if (leido > 0)
        {
            switch (temporal.tipo)
            {
            case SEGUIMIENTO:
            {
                if (temporal.seguimiento.status == 1)
                {
                    leerRespuestaFollow();
                }
                else
                {
                    leerRespuestaUnfollow();
                }
                break;
            }
            case TWEET:
            {
                if (modoGestor == 'A')
                {
                    leerTweet();
                }
                else
                {
                    cliente.tweetsPorLeer[contTweets] = temporal.tweet;
                    contTweets++;
                }

                break;
            }
            }
        }
    }
}
