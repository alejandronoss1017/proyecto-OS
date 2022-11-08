
# Mini Twitter

Comunicación entre procesos a traves de distintos pipes.

## Configuración para VScode

Para ejecutar los distintos programas debemos configurar nuestros archivos `launch.json` y `tasks.json`
que se encuentran dentro del workspace en el directorio `.vscode`.

La configuración para el archivo `launch.json` es la siguiente:

```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Cliente",
      "type": "cppdbg",
      "request": "launch",
      "program": "${workspaceFolder}/client",
      "args": ["pipeGeneral", "pipeEspecifico","usuarioGenial"],
      "cwd": "${fileDirname}",
      "externalConsole": true
    },
    {
      "name": "Gestor",
      "type": "cppdbg",
      "request": "launch",
      "preLaunchTask": "cmake build",
      "program": "${workspaceFolder}/gestor",
      "args": ["80", "ArchivoEntrada2.txt", "A", "5", "pipeGeneral"],
      "cwd": "${fileDirname}",
      "externalConsole": true
    }
  ]
}
```
Para el archivo `tasks.json` es la siguiente:
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "cmake build",
      "type": "shell",
      "command": "make",
      "group": "build",
      "options": {
        "cwd": "${workspaceFolder}"
      }
    }
  ]
}

```

La configuración puede variar según los distintos compiladores, parámetros 
que se quieran utilizar, esto es solo un ejemplo practico para la ejecución, para 
observar todos los posibles atributos y configuraciones se recomienda leer la documentación
de [VScode](https://code.visualstudio.com/docs/editor/debugging).
## Authors

- [Camilo Alejandro Nossa](https://github.com/alejandronoss1017)
- [Sara Valentina Escobar](https://github.com/ValEscoSierra)
- [Diego Alejandro Pardo](https://github.com/DiegoPardoMontero)

