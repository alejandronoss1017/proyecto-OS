#if !defined(SEGUIMIENTO__H)
#define SEGUIMIENTO__H

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct Seguimiento
{
    bool status;
    int idReceptor;
    int exito;
    int fdSeguir;
};

#endif // SEGUIMIENTO__H
