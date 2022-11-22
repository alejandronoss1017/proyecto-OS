#if !defined(SGESTOR__H)
#define SGESTOR__H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scliente.h"
#include "smensaje.h"

struct SGestor
{
    int numUsuarios;
    int **relaciones;
    char modo;
    int tiempo;
    char pipeNom[20];
    struct SCliente clientes[80];
    struct SMensaje mensajes[20];
};

#endif // SGESTOR__H
