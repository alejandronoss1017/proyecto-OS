#if !defined(SCONEXION__H)
#define SCONEXION__H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct SConexion
{
    bool status;
    int exito;
    char pipeNom[20];
};

#endif // SCONEXION__H
