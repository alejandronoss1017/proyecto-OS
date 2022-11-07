#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "sgestor.h"
#include <unistd.h>
#include <fcntl.h>
#include "colors.h"
#include <locale.h>
#include "scliente.h"

// ./gestor
// Parámetros:
// 1: número de usuarios máximo
// 2: relaciones en archivo de texto
// 3: modo del gestor
// 4: tiempo impresión
// 5: nombre del pipe

int **leerMatriz(char *fileName);
void imprimirMatriz(int **matriz);
void leerTweet(struct SMensaje temporal);
void atenderConectarCliente();
void atenderSeguimientoCliente();
void atenderTweetCliente();

/*
    Variables globales
*/

// Creamos la estructura gestor
struct SGestor gestor;
int filas = 0;
int columnas = 0;
int contMensajes = 0;
int contClientes = 0;
struct SMensaje temporal;
int leido;

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    mode_t fifo_mode = S_IRUSR | S_IWUSR;

    // 1. Inicializamos num máximo de usuarios
    gestor.numUsuarios = atoi(argv[1]);
    strcpy(gestor.pipeNom, argv[5]);
    // 2. Inicializamos relaciones del sistema
    gestor.relaciones = leerMatriz(argv[2]);

    // 3. Inicializamos modo del gestor
    gestor.modo = *argv[3];

    // 4. Inicializamos tiempo de impresión
    gestor.tiempo = atof(argv[4]);

    // 5. Inicializamos el pipe general

    unlink(gestor.pipeNom);
    if (mkfifo(gestor.pipeNom, fifo_mode) == -1)
    {
        perror("Gestor mkfifo");
        exit(1);
    }

    // para empezar a atender solicitudes se necesita abrir el pipe
    int fdGeneral = open(gestor.pipeNom, O_RDONLY);
    if (fdGeneral == -1)
    {
        perror("pipe");
        exit(0);
    }

    printf(AMARILLO_T "GESTOR INICIALIZADO\n" RESET_COLOR);
    printf("===========================================================\n");
    printf(CYAN_T "Cantidad máxima de usuarios: " RESET_COLOR AMARILLO_T "%d" RESET_COLOR "\n", gestor.numUsuarios);
    printf(CYAN_T "Relaciones cargadas?: " RESET_COLOR AMARILLO_T "%s" RESET_COLOR "\n", (gestor.numUsuarios != NULL ? "Si" : "No"));
    printf(CYAN_T "Modo del gestor: " RESET_COLOR AMARILLO_T "%c" RESET_COLOR "\n", gestor.modo);
    printf(CYAN_T "Tiempo de impresión: " RESET_COLOR AMARILLO_T "%f" RESET_COLOR "\n", gestor.tiempo);
    printf(CYAN_T "Nombre del pipe del gestor: " RESET_COLOR AMARILLO_T "%s" RESET_COLOR "\n", gestor.pipeNom);
    printf("===========================================================\n");
    imprimirMatriz(gestor.relaciones);

    // Lectura de pipe
    while (true)
    {
        if ((leido = read(fdGeneral, &temporal, sizeof(struct SMensaje))) > 0)
        {
            switch (temporal.tipo)
            {
            case CONEXION:
                atenderConectarCliente();
                break;
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
            case TWEET:
                leerTweet(temporal);
                break;
            default:

                break;
            }
        }
    }
}

void leerTweet(struct SMensaje temporal)
{
    printf("========================================================= \n");
    printf(AZUL_T "Tweet entrante del usuario ID: " RESET_COLOR AMARILLO_T "%d\n" RESET_COLOR, temporal.idEmisor);
    printf(AZUL_T "Nombre del pipe : " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);
    printf(VERDE_T "%s" RESET_COLOR, temporal.tweet.mensaje);
}

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

void atenderConectarCliente()
{
    printf("===========================================================\n");
    fprintf(stderr, MAGENTA_T "Solicitud de conexión entrante del usuario ID:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, temporal.idEmisor);
    // fprintf(stderr, "%d\n", temporal.tipo);
    fprintf(stderr, MAGENTA_T "Nombre del pipe: " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);
    bool encontrado = false;
    for (int i = 0; i < contClientes; i++)
    {
        if (gestor.clientes[i].idCliente == temporal.idEmisor)
        {
            encontrado = true;
        }
    }
    if (encontrado)
    {
        fprintf(stderr, MAGENTA_T "Fue encontrado?: " RESET_COLOR AMARILLO_T "Si"
                                  " \n" RESET_COLOR);
    }
    else
    {
        fprintf(stderr, MAGENTA_T "Fue encontrado?: " RESET_COLOR AMARILLO_T "No"
                                  "\n" RESET_COLOR);
    }
    gestor.clientes[contClientes].fd = open(temporal.conexion.pipeNom, O_WRONLY);
    if (gestor.clientes[contClientes].fd < 0)
    {
        perror("Error");
    }
    if (!encontrado)
    {
        gestor.clientes[contClientes].idCliente = temporal.idEmisor;
        strcpy(gestor.clientes[contClientes].mensaje.conexion.pipeNom, temporal.conexion.pipeNom);
        temporal.conexion.exito = 1;
        write(gestor.clientes[contClientes].fd, &temporal, sizeof(temporal));
        contClientes++;
    }
    else
    {
        struct SMensaje aux;
        aux.conexion.exito = 0;
        write(gestor.clientes[contClientes].fd, &aux, sizeof(aux));
    }
}

void atenderSeguimientoCliente()
{
    printf("===========================================================\n");
    fprintf(stderr, MAGENTA_T "Solicitud de Seguimiento entrante del usuario ID:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, temporal.idEmisor);
    fprintf(stderr, MAGENTA_T "Nombre del pipe: " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);
    bool seguido = false;
    gestor.clientes[contClientes].idCliente = temporal.idEmisor;
    bool emisorExiste = false;

    for (int i = 0; i < contClientes; i++)
    {
        if (gestor.clientes[i].idCliente == temporal.idEmisor)
        {
            emisorExiste = true;
        }
    }
}

void atenderTweetCliente()
{
}
