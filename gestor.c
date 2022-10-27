#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "sgestor.h"
#include <unistd.h>
#include <fcntl.h>

// ./gestor
// Parametros:
// 1: número de usuarios máximo
// 2: relaciones en archivo de texto
// 3: modo del gestor
// 4: tiempo impresion
// 5: nombre del pipe

int **leerMatriz(char *fileName);

int main(int argc, char **argv)
{
    int contMensajes = 0;
    int contClientes = 0;

    mode_t fifo_mode = S_IRUSR | S_IWUSR;
    fprintf(stderr, "%s \n", argv[5]);
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

    printf("%s \n", gestor.pipeNom);
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

    fprintf(stderr, "Pipe abierto");
    // Lectura de pipe
    while (true)
    {
        struct SMensaje temporal;
        int leido;
        if ((leido = read(fdGeneral, &temporal, sizeof(struct SMensaje))) > 0)
        {
            fprintf(stderr, "Solicitud entrante \n");
            fprintf(stderr, "%d\n", temporal.tipo);
            fprintf(stderr, "%s\n", temporal.conexion.pipeNom);

            if (temporal.tipo == CONEXION)
            {
                bool encontrado = false;
                for (int i = 0; i < contClientes; i++)
                {
                    if (gestor.clientes[i].idCliente == temporal.idEmisor)
                    {
                        encontrado = true;
                    }
                }
                fprintf(stderr, "%d", encontrado);
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
                else{
                    struct SMensaje aux;
                    aux.conexion.exito = 0;
                    write(gestor.clientes[contClientes].fd, &aux, sizeof(aux));
                }
            }
    
        }
    }
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
    fgets(linea, 79, fp);
    char *token = strtok(linea, " "); // 10 6
    printf("%s", token);              //
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
