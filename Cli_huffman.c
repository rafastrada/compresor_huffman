#include "Huffman.h"
#include <string.h>


int main(int argc, char *argv[])
{
	if (argc == 3) {
		char *nombre_archivo = argv[2];

		if (strcmp(argv[1], "com") == 0)
			ArchivoCodificar(nombre_archivo);
		else ArchivoDecodificar(nombre_archivo);
	}

	return 0;
}
