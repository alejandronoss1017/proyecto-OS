/**
 * @file gestor.c
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
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "sgestor.h"
#include <unistd.h>
#include <fcntl.h>
#include "colors.h"
#include <locale.h>
#include <pthread.h>
#include "scliente.h"
#include <errno.h>
extern int errno;

/*
Declaración de funciones utilizadas
*/

int **leerMatriz(char *fileName);
void imprimirMatriz(int **matriz);
void leerTweet();
void atenderConectarCliente();
void atenderSeguimientoCliente();
void atenderTweetCliente();
void atenderDesconexion();
void *imprimirEstadisticas();

/*
    Variables globales
*/

struct SGestor gestor;
int filas = 0;
int columnas = 0;
int contMensajes = 0;
int contClientes = 0;
struct SMensaje temporal;
int leido;

/* Variables para las estadisticas */
int numClientesConectado;
int numTweetsEnviados;
int numTweetsRecibidos;

/**
 *
 * @brief
 *
 * @param argc
 * @param argv[1] Número de usuarios máximo
 * @param argv[2] Relaciones en archivo de texto
 * @param argv[3] Modo del gestor
 * @param argv[4] Tiempo impresión
 * @param argv[5] Nombre del pipe
 *
 */
int main(int argc, char **argv)
{
    pthread_t hilo;

    // Funcion utilizada para poder escribir caracteres especiales en consola
    setlocale(LC_ALL, "");

    // Modo de escritura y lectura del pipe
    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    // 1. Inicializamos num máximo de usuarios
    gestor.numUsuarios = atoi(argv[1]);
    strcpy(gestor.pipeNom, argv[5]);

    // 2. Inicializamos relaciones del sistema
    gestor.relaciones = leerMatriz(argv[2]);

    // 3. Inicializamos modo del gestor
    gestor.modo = *argv[3];

    // 4. Inicializamos tiempo de impresión
    gestor.tiempo = atoi(argv[4]);

    // 5. Inicializamos el pipe general

    unlink(gestor.pipeNom);
    if (mkfifo(gestor.pipeNom, fifo_mode) == -1)
    {
        perror("Gestor mkfifo");
        exit(1);
    }

    // para empezar a atender solicitudes se necesita abrir el pipe unicamente de
    int fdGeneral = open(gestor.pipeNom, O_RDONLY);
    if (fdGeneral == -1)
    {
        perror("pipe");
        exit(0);
    }

    // Instaciacion del manejador para imprimir las estadisticas

    // Se imprime la información del gestor inicializado junto con las relaciones del gestor
    printf(AMARILLO_T "GESTOR INICIALIZADO\n" RESET_COLOR);
    printf("===========================================================\n");
    printf(CYAN_T "Cantidad máxima de usuarios: " RESET_COLOR AMARILLO_T "%d" RESET_COLOR "\n", gestor.numUsuarios);
    printf(CYAN_T "Relaciones cargadas?: " RESET_COLOR AMARILLO_T "%s" RESET_COLOR "\n", (gestor.numUsuarios != 0 ? "Si" : "No"));
    printf(CYAN_T "Modo del gestor: " RESET_COLOR AMARILLO_T "%c" RESET_COLOR "\n", gestor.modo);
    printf(CYAN_T "Tiempo de impresión: " RESET_COLOR AMARILLO_T "%d" RESET_COLOR "\n", gestor.tiempo);
    printf(CYAN_T "Nombre del pipe del gestor: " RESET_COLOR AMARILLO_T "%s" RESET_COLOR "\n", gestor.pipeNom);
    printf("===========================================================\n");
    imprimirMatriz(gestor.relaciones);

    // Creacion del hilo

    pthread_create(&hilo, NULL, &imprimirEstadisticas, NULL);
    // Lectura de pipe

    /*
        Este while (true) representa una lectura constante del pipe general para revisar los mensajes que son enviados por cada cliente al gestor
    */
    while (true)
    {

        if ((leido = read(fdGeneral, &temporal, sizeof(struct SMensaje))) > 0)
        // si la lectura del pipe es exitosa...
        {
            switch (temporal.tipo)
            {
                // si el tipo del mensaje Temporal es de CONEXION se llama la función
            case CONEXION:
                if (temporal.conexion.status == 0)
                {
                    atenderDesconexion();
                }
                else
                {
                    atenderConectarCliente();
                }
                break;

                // si el tipo del mensaje Temporal es de CONEXION se llama la función
            case SEGUIMIENTO:
                /*
                1. se recorre la matriz de seguimiento del cliente
                2. Se busca la fila correspondiente al id del cliente, por ejemplo si el cliente que quiere seguir a otra persona tiene el id 1, se sigue al id 1
                3. Se busca la columna correspondiente al usuario que desea seguir, dado por Id seguidor
                4. Una vez ubicados dentro de la posición, se verifica el número
                5. Si hay un 0, escribir un 1
                6. Si hay un 1, decir que ya lo sigue
                7. Mensaje de exito o fracaso
                */
                atenderSeguimientoCliente();
                break;

            // si el tipo del mensaje Temporal es de CONEXION se llama la función
            case TWEET:
                numTweetsRecibidos++;
                atenderTweetCliente();
                break;

            case '0':
                // TODO: LEER LA DESCONEXION Y RESTAR EL CONTADOR
                break;
            default:

                break;
            }
        }
    }
}

/**
 *Esta función imprime por pantalla el tweet mandado por el emisor
 */
void leerTweet()
{
    printf("========================================================= \n");
    printf(AZUL_T "Tweet entrante del proceso ID: " RESET_COLOR AMARILLO_T "%d\n" RESET_COLOR, temporal.processIdEmisor);
    printf(AZUL_T "Nombre del pipe : " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);
    printf(VERDE_T "%s" RESET_COLOR, temporal.tweet.mensaje);
}

/**
 *Esta función se encarga de la impresión de la matriz de relaciones a partir de la lectura de un archivo
 */
int **leerMatriz(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    char linea[80];
    char *token;

    /*
        Contamos las filas que hay en la matriz
    */
    while (fgets(linea, 79, fp))
    {
        filas++;
    }

    /*
        Contar columnas de la matriz
        el 10 equivale a \n, por lo que el
        archivo debe terminar con eso
    */
    int i = 0;
    while (linea[i] != 10)
    {
        if (linea[i] == '0' || linea[i] == '1')
        {
            columnas++;
        }
        i++;
    }

    /*
        Creación de una matriz con apuntadores
    */
    int **m1 = (int **)calloc(filas, sizeof(int *));

    for (int i = 0; i < filas; i++)
    {
        m1[i] = (int *)calloc(columnas, sizeof(int));
    }

    // Devolvemos al inicio el apuntador del fichero
    rewind(fp);

    for (int i = 0; i < filas; i++)
    {
        token = strtok(linea, " ");
        fgets(linea, 79, fp);
        for (int j = 0; j < columnas; j++)
        {
            if (token != NULL)
            {
                m1[i][j] = atoi(token);
                token = strtok(NULL, " ");
            }
        }
        printf("\n");
    }

    fclose(fp);
    return m1;
}

void imprimirMatriz(int **matriz)
{
    for (int i = 0; i < filas; i++)
    {
        for (int j = 0; j < columnas; j++)
        {
            if (matriz[i][j] == 1)
            {
                printf(AMARILLO_T "%d " RESET_COLOR, matriz[i][j]);
            }
            else
            {
                printf(CYAN_T "%d " RESET_COLOR, matriz[i][j]);
            }
        }
        printf("\n");
    }
}

/**
 *Esta función atiende las peticiones de conexion, provenientes del los mensajes enviados por los clientes a través del pipe general
 */
void atenderConectarCliente()
{
    printf("===========================================================\n");
    fprintf(stderr, MAGENTA_T "Solicitud de conexión entrante del proceso ID:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, temporal.processIdEmisor);
    // fprintf(stderr, "%d\n", temporal.tipo);
    fprintf(stderr, MAGENTA_T "Nombre del pipe: " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);

    bool encontrado = false;
    bool conectado = false;
    int idEncontrado;

    // Se busca si ya existe en nombre de usuario de los clientes con el que actualmente está realizando la conexión
    for (int i = 0; i < contClientes; i++)
    {
        if (strcmp(gestor.clientes[i].nombreUsuario, temporal.nombreUsuario) == 0)
        {
            encontrado = true;
            // se obtiene la posición dentro de los clientes donde esta almacenado este usuario
            idEncontrado = i;

            // se asigna a conectado el cliente
            gestor.clientes[i].conectado = true;
            numClientesConectado++;
        }
    }

    // Si se encuentra el usuario se verifica si esta o no conectado
    if (encontrado)
    {
        fprintf(stderr, MAGENTA_T "Fue encontrado?: " RESET_COLOR AMARILLO_T "Si"
                                  " \n" RESET_COLOR);
        if (conectado)
        {
            printf(MAGENTA_T "Usuario encontrado," RESET_COLOR AMARILLO_T " ya conectado \n" RESET_COLOR);
        }
        else
        {
            printf(MAGENTA_T "Usuario encontrado," RESET_COLOR AMARILLO_T " no conectado \n" RESET_COLOR);
        }
    }

    // Si no existe el usuario
    else
    {
        fprintf(stderr, MAGENTA_T "Fue encontrado?: " RESET_COLOR AMARILLO_T "No"
                                  "\n" RESET_COLOR);
    }

    // Si no fue encontrado, se crea nuevo usuario

    // se le asigna el modo acoplado o no acoplada
    temporal.conexion.modoGestor = gestor.modo;
    if (!encontrado)
    {
        sleep(2);
        gestor.clientes[contClientes].fd = open(temporal.conexion.pipeNom, O_WRONLY);
        printf("El fd es: %d \n", gestor.clientes[contClientes].fd);
        if (gestor.clientes[contClientes].fd < 0)
        {
            printf("Error Number % d\n", errno);
            perror("Error");
        }
        // se le asigna el contador de clientes a la pos actual del contador (es decir, en el orden en el que se van creando)
        gestor.clientes[contClientes].idCliente = contClientes;
        gestor.clientes[contClientes].conectado = true;
        gestor.clientes[contClientes].cantTweetsPorLeer = 0;
        numClientesConectado++;
        strcpy(gestor.clientes[contClientes].mensaje.conexion.pipeNom, temporal.conexion.pipeNom);
        strcpy(gestor.clientes[contClientes].nombreUsuario, temporal.nombreUsuario);

        // Se verifica exitoso el registro con 1, si ya fue registrado con 0
        temporal.conexion.exito = 1;
        temporal.conexion.idRetorno = contClientes;
        temporal.conexion.fdRetorno = gestor.clientes[contClientes].fd;
        write(gestor.clientes[contClientes].fd, &temporal, sizeof(temporal));
        printf("Respuesta enviada! \n");
        contClientes++;
    }
    // Si fue encontrado, se revisa su conexion
    else
    {

        struct SMensaje aux;
        aux.conexion.modoGestor = gestor.modo;
        // Si está conectado, no se puede crear una nueva conexion
        if (conectado)
        {
            sleep(2);
            gestor.clientes[contClientes].fd = open(temporal.conexion.pipeNom, O_WRONLY);
            if (gestor.clientes[contClientes].fd < 0)
            {
                perror("Error");
            }
            aux.conexion.exito = 0;
            aux.conexion.idRetorno = contClientes;
            aux.conexion.fdRetorno = gestor.clientes[contClientes].fd;
            write(gestor.clientes[contClientes].fd, &aux, sizeof(aux));
            printf("Respuesta enviada! \n");
        }
        // Si no está conectado, se crea una nueva conexion con un nombre de usuario ya existente
        else
        {
            sleep(2);
            gestor.clientes[idEncontrado].fd = open(temporal.conexion.pipeNom, O_WRONLY);
            if (gestor.clientes[idEncontrado].fd < 0)
            {
                perror("Error");
            }
            strcpy(gestor.clientes[idEncontrado].mensaje.conexion.pipeNom, aux.conexion.pipeNom);
            aux.conexion.exito = 1;
            aux.conexion.idRetorno = idEncontrado;
            aux.conexion.fdRetorno = gestor.clientes[idEncontrado].fd;
            aux.cantTweetsPorVer = 0;
            if(gestor.clientes[idEncontrado].cantTweetsPorLeer != 0){
               for(int i = 0; i < gestor.clientes[idEncontrado].cantTweetsPorLeer; i++){
                    aux.tweetsPorVer[i] = gestor.clientes[idEncontrado].tweetsPorLeer[i];
                    aux.cantTweetsPorVer++;
               }
            }
            write(gestor.clientes[idEncontrado].fd, &aux, sizeof(aux));
            printf("Respuesta enviada! \n");
        }
    }
}

void atenderDesconexion()
{
    printf("===========================================================\n");
    fprintf(stderr, MAGENTA_T "Solicitud de desconexión entrante del proceso ID:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, temporal.processIdEmisor);
    fprintf(stderr, MAGENTA_T "Nombre del pipe: " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);

    for (int i = 0; i < gestor.numUsuarios; i++)
    {
        if (strcmp(gestor.clientes[i].nombreUsuario, temporal.nombreUsuario) == 0)
        {
            gestor.clientes[i].conectado = 0;
            numClientesConectado--;
            break;
        }
    }
}

/**
Esta función atiende los mensajes de tipo SEGUIMIENTO que son eviados de los clientes al gestor por el pipe general
*/
void atenderSeguimientoCliente()
{
    printf("===========================================================\n");
    fprintf(stderr, MAGENTA_T "Solicitud de Seguimiento entrante del proceso ID:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, temporal.processIdEmisor);
    fprintf(stderr, MAGENTA_T "Nombre del pipe: " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);
    bool seguido = false;
    bool emisorExiste = false;
    struct SMensaje aux;

    // Mirar que no se excedan los limites de la matriz y que exita más de 1, si esto sucede, el seguimiento es fallido
    if (temporal.seguimiento.idReceptor > filas || temporal.seguimiento.idReceptor < 0)
    {
        aux.seguimiento.exito = 0;
    }
    else
    {
        // Opcion de follow
        if (temporal.seguimiento.status == 1)
        {
            aux.seguimiento.status = 1;
            if (gestor.relaciones[temporal.idEmisor][temporal.seguimiento.idReceptor] == 1)
            {
                // caso 2 de exito: cuando ya se sigue un usuario y se le avisa
                aux.seguimiento.exito = 2;
            }
            else
            {
                gestor.relaciones[temporal.idEmisor][temporal.seguimiento.idReceptor] = 1;
                aux.seguimiento.exito = 1;
            }
        }
        else
        {
            // Opcion de unfollow
            aux.seguimiento.status = 0;
            if (gestor.relaciones[temporal.idEmisor][temporal.seguimiento.idReceptor] == 0)
            {
                // caso 2 de exito: cuando ya se no sigue un usuario y se le avisa
                aux.seguimiento.exito = 2;
            }
            else
            {
                gestor.relaciones[temporal.idEmisor][temporal.seguimiento.idReceptor] = 0;
                aux.seguimiento.exito = 1;
            }
        }
    }
    aux.tipo = SEGUIMIENTO;
    // se envia la respeusta de seguimiento al pipe especifico del cliente
    write(gestor.clientes[temporal.idEmisor].fd, &aux, sizeof(aux));
    printf("Respuesta enviada! \n");
}

/**
 * Esta función se encarga de recibir un mensaje de tipo TWEET, revisando a partir de la lectura de la matriz , verificando si los seguidores del emisor del tweet
 */
void atenderTweetCliente()
{
    leerTweet();
    printf("===========================================================\n");
    imprimirMatriz(gestor.relaciones);

    // recorre todos id de los clientes (filas)

    for (int i = 0; i < filas; i++)
    {
        if (gestor.relaciones[i][temporal.idEmisor] == 1)
        {
            struct SMensaje aux;
            aux.idEmisor = temporal.idEmisor;
            aux.processIdEmisor = temporal.processIdEmisor;
            aux.tipo = TWEET;
            aux.tweet.idEmisor = temporal.idEmisor;
            strcpy(aux.tweet.mensaje, temporal.tweet.mensaje);
            if (gestor.clientes[i].conectado == 1)
            {
                write(gestor.clientes[i].fd, &aux, sizeof(aux));
                printf("Respuesta enviada al usuario %d! \n", gestor.clientes[i].idCliente);
            }
            else
            {
                gestor.clientes[i].tweetsPorLeer[gestor.clientes[i].cantTweetsPorLeer].idEmisor = aux.tweet.idEmisor;
                strcpy(gestor.clientes[i].tweetsPorLeer[gestor.clientes[i].cantTweetsPorLeer].mensaje, aux.tweet.mensaje);
                gestor.clientes[i].cantTweetsPorLeer++;
            }
            numTweetsEnviados++;
        }
    }
}

/**
Función que imprime las estadisticas a partir de un hilo
*/
void *imprimirEstadisticas()
{
    while (true)
    {
        printf("=================================================\n");
        printf("Estadisticas Geniales \n");
        printf("Numero de usuarios conectados: %d \n", numClientesConectado);
        printf("Numero de tweets recibidos: %d \n", numTweetsRecibidos);
        printf("Numero de tweets enviados: %d \n", numTweetsEnviados);
        sleep(gestor.tiempo);
    }
}