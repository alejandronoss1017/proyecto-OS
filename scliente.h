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
    int fd;
    char pipeNom[20];
    struct STweet tweetsPorLeer[20];
    struct SMensaje mensaje;
};

#endif // SCLIENTE__H
