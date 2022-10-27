#if !defined(SMENSAJE__H)
#define SMENSAJE__H

#include "stweet.h"
#include "seguimiento.h"
#include "sconexion.h"

#define CONEXION 1

struct SMensaje
{
    int idEmisor;
    int tipo;
    struct STweet tweet;
    struct Seguimiento seguimiento;
    struct SConexion conexion;

}mensajes;

#endif // SMENSAJE__H
