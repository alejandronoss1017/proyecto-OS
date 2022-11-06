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
void leerTweet(struct SMensaje temporal);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    int contMensajes = 0;
    int contClientes = 0;

    /*
        Creación de un arreglo dinámico de clientes para almacenar toda la información
        de los clientes que se han conectado al gestor.

        Podemos modificar el tamaño de este arreglo con realloc.
    */
    struct SCliente *clientes = calloc(atoi(argv[1]), sizeof(struct SCliente));

    mode_t fifo_mode = S_IRUSR | S_IWUSR;
    // Creamos la estructura gestor
    struct SGestor gestor;

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
    printf(CYAN_T "Relaciones cargadas?: " RESET_COLOR AMARILLO_T "%d" RESET_COLOR "\n", (gestor.numUsuarios != NULL ? 1 : 0));
    printf(CYAN_T "Modo del gestor: " RESET_COLOR AMARILLO_T "%c" RESET_COLOR "\n", gestor.modo);
    printf(CYAN_T "Tiempo de impresión: " RESET_COLOR AMARILLO_T "%f" RESET_COLOR "\n", gestor.tiempo);
    printf(CYAN_T "Nombre del pipe del gestor: " RESET_COLOR AMARILLO_T "%s" RESET_COLOR "\n", gestor.pipeNom);

    if (gestor.numUsuarios != NULL)
    {
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                printf("%d", gestor.relaciones[i][j]);
            }
        }
    }

    // Lectura de pipe
    while (true)
    {
        struct SMensaje temporal;
        int leido;
        if ((leido = read(fdGeneral, &temporal, sizeof(struct SMensaje))) > 0)
        {
            // fprintf(stderr, "Solicitud entrante \n");
            // fprintf(stderr, "%d\n", temporal.tipo);
            // fprintf(stderr, "%s\n", temporal.conexion.pipeNom);
            switch (temporal.tipo)
            {

            case CONEXION:
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
                fprintf(stderr, MAGENTA_T "Fue encontrado?:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, encontrado);
                gestor.clientes[contClientes].fd = open(temporal.conexion.pipeNom, O_WRONLY);
                if (gestor.clientes[contClientes].fd < 0)
                {
                    perror("Error");
                    continue;
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
                break;
            case SEGUIMIENTO:
                /*1. se recorre la matriz de seguimiento del cliente
                2. Se busca la fila correspondiente al id del cliente, por ejemplo si el cliente que quiere seguir a otra persona tiene el id 1, se sigue al id 1
                3. Se busca la columna correspondiente al usuario que desea seguir, dado por Idseguidor
                4. Una vez ubicados dentro de la posición, se verifica el número
                5. Si hay un 0, escribir un 1
                6. Si hay un 1, decir que ya lo sigue
                7. Mensaje de exito o fracaso
                */
                printf("===========================================================\n");
                fprintf(stderr, MAGENTA_T "Solicitud de Seguimiento entrante del usuario ID:" RESET_COLOR AMARILLO_T " %d \n" RESET_COLOR, temporal.idEmisor);
                fprintf(stderr, MAGENTA_T "Nombre del pipe: " RESET_COLOR AMARILLO_T "%s\n" RESET_COLOR, temporal.conexion.pipeNom);
                bool seguido = false;

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
    printf(VERDE_T "%s\n" RESET_COLOR, temporal.tweet.mensaje);
}

int **leerMatriz(char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    int filas2 = 0;
    int columnas2 = 0;
    char linea[80];
    char *token;

    /*
        Contamos las filas que hay en la matriz
    */
    while (fgets(linea, 79, fp))
    {
        filas2++;
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
            columnas2++;
        }
        i++;
    }

    /*
        Creación de una matriz con apuntadores
    */
    int **m1 = (int **)calloc(filas2, sizeof(int *));

    for (int i = 0; i < filas2; i++)
    {
        m1[i] = (int *)calloc(columnas2, sizeof(int));
    }

    // Devolvemos al inicio el apuntador del fichero
    rewind(fp);

    token = strtok(linea, " ");
    for (int i = 0; i < filas2; i++)
    {
        fgets(linea, 79, fp);
        for (int j = 0; j < columnas2; j++)
        {
            if (token != NULL)
            {
                m1[i][j] = atoi(token);
                token = strtok(NULL, " ");
            }

            printf("%d ", m1[i][j]);
        }
        printf("\n");
    }

    printf("%s", token);
    int filas = atoi(token);
    token = strtok(NULL, " ");
    int cols = atoi(token);
    int **matriz = malloc(filas * sizeof(int *));
    for (int i = 0; i < filas; i++)
    {
        matriz[i] = malloc(cols * sizeof(int));
        fgets(linea, 79, fp);
        token = strtok(linea, " ");
        matriz[i][0] = atoi(token);
        for (int j = 1; j < cols; j++)
        {
            token = strtok(NULL, " ");
            matriz[i][j] = atoi(token);
        }
    }
    fclose(fp);
    return matriz;
}
