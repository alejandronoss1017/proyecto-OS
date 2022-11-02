#if !defined(SMENSAJE__H)
#define SMENSAJE__H

#include "stweet.h"
#include "seguimiento.h"
#include "sconexion.h"

#define CONEXION 1
#define SEGUIMIENTO 2
#define TWEET 3

struct SMensaje
{
    int idEmisor;
    int tipo;
    struct STweet tweet;
    struct Seguimiento seguimiento;
    struct SConexion conexion;

}mensajes;

#endif // SMENSAJE__H
