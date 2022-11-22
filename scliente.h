#if !defined(SCLIENTE__H)
#define SCLIENTE__H

#include "stweet.h"
#include "smensaje.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stweet.h"
#include "smensaje.h"

struct SCliente
{
    int idCliente;
    int processId;
    bool conectado;
    int fd;
    char nombreUsuario[40];
    char pipeNom[20];
    int cantTweetsPorLeer;
    struct STweet tweetsPorLeer[20];
    struct SMensaje mensaje;
};

#endif // SCLIENTE__H
