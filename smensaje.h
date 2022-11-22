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
    int processIdEmisor;
    int idEmisor;
    char nombreUsuario[40];
    int tipo;
    struct STweet tweet;
    struct STweet tweetsPorVer[20];
    int cantTweetsPorVer;
    struct Seguimiento seguimiento;
    struct SConexion conexion;

} mensajes;

#endif // SMENSAJE__H
