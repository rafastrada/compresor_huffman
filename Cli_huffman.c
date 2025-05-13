#include "Huffman.h"
#include <stdio.h>
#include <string.h>


int main(int argc, char *argv[])
{
	if (argc == 3) {
		char *nombre_archivo = argv[2],
			*operacion = argv[1];

		if (strcmp(operacion, "comprimir") == 0)
			ArchivoCodificar(nombre_archivo);
		else {
			if (strcmp(operacion, "descomprimir") == 0)
				ArchivoDecodificar(nombre_archivo);
		}
	} else {
		printf(
				"\nCompresor de archivos por codificación de Huffman\n\n"
				"Uso: huffman.exe (operacion) (nombre de archivo)\n"
				"Operaciones disponibles:\n"
				"\t'comprimir' : Comprime un archivo en uno nuevo con extensión \".huf\".\n"
				"\t'descomprimir' : Descomprime un archivo de extensión \".huf\", obteniendo el archivo original."
				"El resultado posee extensión \".dhu\".\n"
			  );
	}

	return 0;
}
