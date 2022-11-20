#if !defined(SCONEXION__H)
#define SCONEXION__H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct SConexion
{
    bool status;
    int exito;
    int idRetorno;
    int fdRetorno;
    char pipeNom[20];
    char modoGestor;
};

#endif // SCONEXION__H
