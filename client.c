/**
 * @file client.c
 * @author Camilo Nossa (calejandro_nossa@javeriana.edu.co)
 * @author Diego Pardo (di-pardo@javeriana.edu.co)
 * @author Sara Sierra (svalentinasierra@javeriana.edu.co)
 *
 * @brief
 * @version 0.1
 * @date 2022-11-21
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
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
void *leerPipeEspecifico();
void leerRespuestaFollow();
void leerRespuestaUnfollow();
void imprimirMenuCliente();
void verTweetsPorLeer();
void desconexion();

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
    printf(MAGENTA_T "Desea realizar la solicitud de conexion? " AMARILLO_T "(s/n)\n" RESET_COLOR);
    scanf("%c", &opcion2);
    if (opcion2 == 's')
    {
        realizarConexion();
        leerRespuestaConexion();
        // Creacion del hilo
        pthread_create(&hilo, NULL, &leerPipeEspecifico, NULL);
        do
        {
            imprimirMenuCliente();
            scanf(" %c", &opcion);
            fgetc(stdin);

            switch (opcion)
            {

            case '1':
                /*Mensaje de tipo follow*/
                follow();
                sleep(7);
                break;
            case '2':
                /*Mensaje de tipo follow*/
                unfollow();
                sleep(7);
                break;
            case '3':
                /*Mensaje de tipo unfollow*/
                enviarTweet();
                sleep(7);
                break;
            case '4':
                /*Opcion de tipo unfollow*/
                verTweetsPorLeer();
                sleep(7);
                break;
            case '0':
                // Cierra y elimina el pipe
                desconexion();
                close(cliente.fd);
                unlink(cliente.pipeNom);
                terminar = true;

                // TODO: Poner funcion de desconexion para enviar el mensaje de desconexion
                break;
            default:
                printf("Seleccione una de las opciones establecidas \n");
                break;
            }
        } while (terminar != true);
    }
    else
    {
        printf(ROJO_T "No se puede realizar ninguna solicitud, buen dia. \n" RESET_COLOR);
    }

    return 0;
}

/**
 * Una vez el usuario acepta realizar la conexion se ejecuta la función de conexion,
 * la cual consta de enviar una estructura de tipo mensaje por el pipeGeneral del gestor
 * para que este lo guarde en su arreglo de clientes conectados.
 * En la estructura tipo mensaje se envía:
 *
 * 1.Tipo de mensaje (CONEXION)
 * 2.Estatus de la conexion para saber si esta conectado o no el cliente
 * 3.Nombre de usuario
 * 4.Nombre del pipe asociado al cliente (pipeEspecifico)
 *
 */
void realizarConexion()
{
    cliente.mensaje.tipo = CONEXION;
    cliente.mensaje.conexion.status = 1;
    strcpy(cliente.mensaje.nombreUsuario, cliente.nombreUsuario);
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);

    // Primero el write antes del open
    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf(AMARILLO_T "Solicitud enviada\n" RESET_COLOR);

    unlink(cliente.pipeNom);
    mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);
    if ((fdEspecifico = open(cliente.pipeNom, O_RDONLY)) == -1)
    {
        perror("Error");
        exit(1);
    }
}

/**
 * Esta función se encarga de leer la respuesta del mensaje tipo CONEXION que viene del gestor (estructura temporal) al pipe específico
 */
void leerRespuestaConexion()
{
    int leido;

    if ((leido = read(fdEspecifico, &temporal, sizeof(struct SMensaje))) < 0)
    {
        perror("Error:");
        exit(1);
    }
    /*
        Si la conexion fue exitosa,
        1. El gestor se inicia de acuerdo al modo definido al momento de compilar el programa
        2. Se le asigna un id al cliente que solicita la conexión (idRetorno) , este id será utilizado para operaciones posteriores como follow y unfollow
        3. Se le asigna fd de retorno dentro de la estructura del cliente
    */
    if (temporal.conexion.exito == 1)
    {
        modoGestor = temporal.conexion.modoGestor;
        cliente.idCliente = temporal.conexion.idRetorno;
        cliente.fd = temporal.conexion.fdRetorno;
        if (modoGestor == 'A')
        {
            if (temporal.cantTweetsPorVer != 0)
            {
                printf("===================================================================================== \n");
                printf(MAGENTA_T "\t Estos fueron los Tweets que te perdiste mientras no estabas! \n" RESET_COLOR);
            }

            for (int i = 0; i < temporal.cantTweetsPorVer; i++)
            {
                printf("===================================================================================== \n");
                printf(MAGENTA_T "Tweet de usuario: " RESET_COLOR AMARILLO_T "%d \n" RESET_COLOR, temporal.tweetsPorVer[i].idEmisor);
                printf(VERDE_T "Mensaje: \n" RESET_COLOR);
                printf("%s \n", temporal.tweetsPorVer[i].mensaje);
                printf("\n");
            }
            sleep(3);
        }
        else
        {

            for (int i = 0; i < temporal.cantTweetsPorVer; i++)
            {
                cliente.tweetsPorLeer[i] = temporal.tweetsPorVer[i];
            }
            contTweets = temporal.cantTweetsPorVer;
            if (temporal.cantTweetsPorVer != 0)
            {
                printf("===================================================================================== \n");
                printf(MAGENTA_T "\t Tienes Tweets por leer! \n" RESET_COLOR);
                printf("===================================================================================== \n");
            }
            sleep(3);
        }
        printf(VERDE_T "Fue exitosa la conexion\n" RESET_COLOR);
    }
    /* Si no se cumplen los otros casos, la conexion es fallida*/
    else
    {
        printf(ROJO_T "Conexion fallida\n" RESET_COLOR);
    }
}

/**
 * Esta funcion define los aspectos principales que se pasaran por el pipe general al gestor en el mensaje de tipo SEGUIMIENTO*
 */
void follow()
{
    /*Define de que tipo es el mensaje*/
    cliente.mensaje.tipo = SEGUIMIENTO;

    /* Se define el status, si es 1 es solicitud de seguimiento, de lo contrario es una solicitud de unfollow*/
    cliente.mensaje.seguimiento.status = 1;

    /* Se asigna como emisor del mensaje al cliente actual (idCliente)*/
    cliente.mensaje.idEmisor = cliente.idCliente;

    /* Se define para el seguimiento su fileDescriptor, el del cliente que lo envía*/
    cliente.mensaje.seguimiento.fdSeguir = cliente.fd;

    fprintf(stderr, AZUL_T "Escriba el identificador del usuario que desee seguir\n" RESET_COLOR);

    // se lee el id del cliente al que desea seguir
    scanf("%d", &cliente.mensaje.seguimiento.idReceptor);

    /*Se manda la el mensaje (contruido por tipo, status del seguimiento, fd seguir y emisor) al gestor por el pipe general */
    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));

    // se le avisa al cliente que su solicitud fue enviada al gestor
    printf(AMARILLO_T "Solicitud enviada\n" RESET_COLOR);
}

/**
 * Esta función define los aspectos principales que se pasaran por el pipe general al gestor en el mensaje de tipo SEGUIMIENTO
 * Se define que si bien la solicitud es de tipo SEGUMIENTO, el status que se realizara es de UNFOLLOW
 */
void unfollow()
{
    cliente.mensaje.tipo = SEGUIMIENTO;

    cliente.mensaje.seguimiento.status = 0;

    cliente.mensaje.idEmisor = cliente.idCliente;

    printf(MAGENTA_T "Escriba el identificador del usuario que desee dejar de seguir\n" RESET_COLOR);

    scanf("%d", &cliente.mensaje.seguimiento.idReceptor);

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));

    printf(AMARILLO_T "Solicitud enviada\n" RESET_COLOR);
}

/**
 *Esta función se encarga de leer la respuesta del mensaje tipo SEGUIMIENTO (follow) que viene del gestor(del pipe general) al pipe específico
 */
void leerRespuestaFollow()
{

    // Si el exito es 1, quiere decir que el usuario emisor ya sigue al usuario receptor
    if (temporal.seguimiento.exito == 1)
    {
        printf(VERDE_T "Ahora estas siguiendo al usuario: " RESET_COLOR AMARILLO_T "%d \n" RESET_COLOR, cliente.mensaje.seguimiento.idReceptor);
    }

    // Si el exito es igual a 2, quiere decir que el usuario emisor ya seguía al usuario receptor en el momento que realizó la solicitud
    else if (temporal.seguimiento.exito == 2)
    {
        printf(AMARILLO_T "Ya sigues a este usuario! \n" RESET_COLOR);
    }

    // Si el exito difiere de 1 o 2, quiere decir que la operación fue fallida
    else
    {
        printf(ROJO_T "Operación de seguimiento fallida\n" RESET_COLOR);
    }
}

/**
Esta función se encarga de leer la respuesta del mensaje tipo SEGUIMIENTO (unfollow) que viene del gestor (por el pipel general) al pipe específico
*/
void leerRespuestaUnfollow()
{
    int leido;

    /*Se especifica que el tipo de tipo del mensaje es de tipo SEGUIMIENTO  */
    cliente.mensaje.tipo = SEGUIMIENTO;

    /* Si el exito del mensaje (temporal) es igual a 1, fue exitosa la solicitud de dejar de seguir*/
    if (temporal.seguimiento.exito == 1)
    {
        printf(ROJO_T "Dejaste de seguir al usuario: " RESET_COLOR AMARILLO_T "%d \n" RESET_COLOR, cliente.mensaje.seguimiento.idReceptor);
    }

    /* Si el exito del mensaje (temporal) es igual a 2, ya no se seguía al usuario que se deseaba dejar de seguir*/
    else if (temporal.seguimiento.exito == 2)
    {
        printf(ROJO_T "No  sigues a este usuario! \n" RESET_COLOR);
    }
    else
    // Si el exito difiere de 1 o 2, quiere decir que la operación fue fallida
    {
        printf(ROJO_T "Operación de seguimiento fallida\n" RESET_COLOR);
    }
}

/**
 *Esta funcion define los aspectos principales que se pasaran por el pipe general al gestor en el mensaje de tipo TWEET
 */
void enviarTweet()
{

    cliente.mensaje.tipo = TWEET;

    /* Se asigna como emisor el id del cliente que envia la solicitud por el pipe general*/
    cliente.mensaje.idEmisor = cliente.idCliente;

    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);

    // unlink(cliente.pipeNom);
    // mkfifo(cliente.pipeNom, S_IRUSR | S_IWUSR);

    printf(AZUL_T "Escriba su tweet a continuación:\n" RESET_COLOR);
    fgets(cliente.mensaje.tweet.mensaje, 200, stdin);

    // se escribe el mensaje o se envía al pipe general
    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));
    printf(AMARILLO_T "Tweet enviado\n" RESET_COLOR);
}

/**
 *Esta función se encarga de mostrarle al usuario el tweet a leer
 */
void leerTweet()
{
    system("clear");
    printf("\n");
    printf("===================================================================================== \n");
    printf(MAGENTA_T "Tweet de usuario: " RESET_COLOR AMARILLO_T "%d \n" RESET_COLOR, temporal.tweet.idEmisor);
    printf(VERDE_T "Mensaje: \n" RESET_COLOR);
    printf("%s \n", temporal.tweet.mensaje);
    printf("\n");
    printf("===================================================================================== \n");
    sleep(3);
    imprimirMenuCliente();
}

/**
 *Esta función revisa si existen tweets por leer, cuando el gestor se inicializa en modo desacoplado
 */
void verTweetsPorLeer()
{

    // si el conteo de tweets está en 0, se le avisa al cliente que no tiene nuevos tweets por leer
    if (contTweets == 0)
    {
        printf(ROJO_T "NO tiene nuevos tweets por leer \n" RESET_COLOR);
    }
    // else?
    // si no , se recorre el arreglo de tweets por leer, mostrando el emisor del tweet y el mensaje correspondiente
    for (int i = 0; i < contTweets; i++)
    {
        printf(MAGENTA_T "Tweet de usuario: " RESET_COLOR AMARILLO_T "%d \n" RESET_COLOR, cliente.tweetsPorLeer[i].idEmisor);
        printf(VERDE_T "Mensaje: \n" RESET_COLOR);
        printf("%s \n", cliente.tweetsPorLeer[i].mensaje);
    }
}

/**
Esta función se encarga de leer las solicitudes entrantes que vienen del pipe general a a partir de un while (true) que siempre lo va revisando con respecto al HILO
*/
void *leerPipeEspecifico()
{
    while (true)
    {
        int leido;

        /*Se lee la estructura temporal, la cual es enviada a a partir del pipe general hacia el pipe especifico*/

        leido = read(fdEspecifico, &temporal, sizeof(struct SMensaje));

        // si la lectura es exitosa
        if (leido > 0)
        {
            switch (temporal.tipo)
            {
                // si el mensaje temporal es de tipo SEGUIMIENTO
            case SEGUIMIENTO:
            {
                // si es de tipo follow, se lee llama la función
                if (temporal.seguimiento.status == 1)
                {
                    leerRespuestaFollow();
                }
                else
                // si es de tipo unfollow, se lee llama la funcion
                {
                    leerRespuestaUnfollow();
                }
                break;
            }
            // si el mensaje temporal es de tipo TWEET
            case TWEET:
            {
                // Se verifica si modo del gestor es acoplado
                if (modoGestor == 'A')
                {
                    // si es modo acoplado, los tweets aparecen gracias al hilo que esta constantemente leyendo el pipe especifico, lo que hacen que los tweets aparezcan en la consola del cliente
                    leerTweet();
                }
                else
                {
                    // si el modo es desacoplado, los tweets
                    cliente.tweetsPorLeer[contTweets] = temporal.tweet; // se asignan los tweets que va leyendo del pipe dentro de cada conteo de tweets en el arreglo de tweets por leer
                    contTweets++;
                }

                break;
            }
            }
        }
    }
}

void imprimirMenuCliente()
{
    system("clear");
    printf(VERDE_T "Su " RESET_COLOR AMARILLO_T "ID " RESET_COLOR VERDE_T "asignado es: " RESET_COLOR AMARILLO_T "%d\n" RESET_COLOR, cliente.idCliente);
    printf("===================================================================================== \n");
    printf(AZUL_T "MENU SELECCIÓN DE PETICIÓN \n" RESET_COLOR);
    printf(AMARILLO_T "1" RESET_COLOR
                      ". Follow \n");
    printf(AMARILLO_T "2" RESET_COLOR
                      ". Unfollow \n");
    printf(AMARILLO_T "3" RESET_COLOR
                      ". Tweet \n");
    if (modoGestor == 'D')
    {
        printf(AMARILLO_T "4" RESET_COLOR
                          ". Ver Tweets por leer \n");
    }

    printf(AMARILLO_T "0" RESET_COLOR ". Salir \n");
}

void desconexion()
{
    cliente.mensaje.tipo = CONEXION;
    cliente.mensaje.conexion.status = 0;
    strcpy(cliente.mensaje.nombreUsuario, cliente.nombreUsuario);
    strcpy(cliente.mensaje.conexion.pipeNom, cliente.pipeNom);

    write(fdGeneral, &cliente.mensaje, sizeof(cliente.mensaje));

    printf(ROJO_T "Solicitud de desconexion enviada\n" RESET_COLOR);
}