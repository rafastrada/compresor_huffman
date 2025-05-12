# Compresor Huffman

## Compilación

Para compilar el programa debe usarse el siguiente comando (si se usa GCC):

```
gcc -Wall -g3 Cli_huffman.c Huffman.c -o huffman.exe

```

`-o <path/nombre_ejecutable>` es la dirección y nombre del ejecutable del programa.
Puede ser en cualquier lugar.

`-Wall` muestra todas las advertencias del compilador.

`-g3` es necesario incluir solo por propósitos de depuración.

## Uso

```
Ver ayuda.
> huffman.exe

Comprimir un archivo.
> huffman.exe comprimir <nombre de archivo>

Descomprimir un archivo de extensión ".huf".
> huffman.exe descomprimir <nombre de archivo HUF>
```
