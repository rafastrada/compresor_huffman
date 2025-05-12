#include "Huffman.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


/**	Recorre un archivo y cuenta la frecuencia de cada byte de su contenido, y es
 * almacenado en la tabla de frecuencias indicada.
 *
 * @param nombreArchivo Nombre completo del archivo a recorrer.
 * @param tablaFrecuencias Tabla donde se almacenara la frecuencia de los bytes de 'nombreArchivo'.
 *
 * @return Cero si la operación se realizó correctamente. -1 en caso contrario.
 */
int ArchivoContarFrecuencia(char nombreArchivo[], unsigned int tablaFrecuencias[]) {
	FILE *fuente = fopen(nombreArchivo, "rb");

	// control de error al abrir el archivo
	if (fuente == NULL) return -1;


	uint8_t byteLeido;
	int controlLectura;
	// se recorre el archivo entero hasta el final
	while (!feof(fuente)) {
		// se lee un byte del archivo 
		controlLectura = fread(&byteLeido, 1, 1, fuente);

		// control para evitar contar dos veces el ultimo byte del archivo
		if (controlLectura) {
			++(tablaFrecuencias[byteLeido]);
		}
	}

	fclose(fuente);

	// retorna cero si no hubo errores
	return 0;
}

struct buffer_8 {
	uint8_t byte;
	short int bits_restantes;
};
struct buffer_32 {
	uint32_t palabra;
	short int numBits;
};

void TablaDescriptoresMapear(TablaDescriptores *tabla) {
	for (int i=0; i<TAM_TABLA_FRECUENCIAS; i++) {
		tabla->mapeador[i] = NULL;
	}

	DescriptorCod *auxiliar;
	for (int i=0; i<tabla->numDescriptores; i++) {
		auxiliar = &(tabla->arregloDescriptores[i]);
		tabla->mapeador[auxiliar->caracter] = auxiliar;
	}
}

void TablaDescriptoresAlinearCod(TablaDescriptores *tabla) {
	DescriptorCod auxiliar;
	for (int i=0; i<tabla->numDescriptores; i++) {
		auxiliar = tabla->arregloDescriptores[i];

		auxiliar.codigo = auxiliar.codigo << (32-auxiliar.numBitsCodigo);
		tabla->arregloDescriptores[i] = auxiliar;
	}
}


void ArchivoCodificar(char nombreArchivo[]) {
	unsigned int frecuencias[TAM_TABLA_FRECUENCIAS];

	TablaFrecuenciasInit(frecuencias);
	ArchivoContarFrecuencia(nombreArchivo, frecuencias);

	Arbol arbol = ListaOrdenadaCrear(frecuencias);
	GenerarArbol(&arbol);

	TablaDescriptores descriptores = {
		.arregloDescriptores =
			(DescriptorCod *) malloc(sizeof(DescriptorCod) * arbol.numNodos),
		.numDescriptores = arbol.numNodos
	};

	GenerarCodigos(descriptores.arregloDescriptores, &arbol);

	TablaDescriptoresMapear(&descriptores);

	char nombreDestino[128]; strcpy(nombreDestino, nombreArchivo);
	strcat(nombreDestino, ".huf");

	FILE *fuente = fopen(nombreArchivo, "rb"),
		*destino = fopen(nombreDestino, "wb");
	
	// espacio para bits finales que no son parte del codigo
	uint32_t numCaracteres = 0;
	fwrite(&numCaracteres, sizeof(uint32_t), 1, destino);
	// escritura de tabla en archivo destino
	fwrite(&descriptores.numDescriptores, sizeof(uint32_t), 1, destino);
	fwrite(descriptores.arregloDescriptores, sizeof(DescriptorCod),
			descriptores.numDescriptores, destino);

	uint8_t byteLeido;
	int controlLectura;
	struct buffer_8 salida = {
		.byte = 0x0,
		.bits_restantes = 8
	};
	DescriptorCod descriptor;
	while (!feof(fuente)) {
		controlLectura = fread(&byteLeido, 1, 1, fuente);
		if (controlLectura) {
			descriptor = *(descriptores.mapeador[byteLeido]);
			descriptor.codigo = descriptor.codigo << (32-descriptor.numBitsCodigo);

			while (descriptor.numBitsCodigo) {
				salida.byte = salida.byte << 1;
				if (descriptor.codigo & 0x80000000) ++(salida.byte);
				--(salida.bits_restantes);

				if (!salida.bits_restantes) {
					fwrite(&salida.byte, 1, 1, destino);
					salida.byte = 0x0;
					salida.bits_restantes = 8;
				}

				descriptor.codigo = descriptor.codigo << 1;
				--(descriptor.numBitsCodigo);
			}

			++numCaracteres;
		}
	};

	if (salida.bits_restantes > 0 && salida.bits_restantes < 8) {
		salida.byte = salida.byte << salida.bits_restantes;
		fwrite(&salida.byte, 1, 1, destino);
	};

	rewind(destino);
	fwrite(&numCaracteres, sizeof(uint32_t), 1, destino);

	fclose(destino);
	fclose(fuente);
	free((void *) descriptores.arregloDescriptores);
}

void ArchivoDecodificar(char nombreArchivo[]) {
	FILE *fuente = fopen(nombreArchivo, "rb"), *destino;

	if (fuente == NULL) return;

	uint32_t numCaracteres;
	TablaDescriptores descriptores;


	// lectura de cantidad de caracteres del archivo original
	fread(&numCaracteres, sizeof(uint32_t), 1, fuente);
	// lectura de tamaño de tabla de descriptores
	fread(&descriptores.numDescriptores, sizeof(uint32_t), 1, fuente);
	descriptores.arregloDescriptores =
		(DescriptorCod *) malloc(sizeof(DescriptorCod)
				* descriptores.numDescriptores);
	// lectura de tabla de descriptores
	fread(descriptores.arregloDescriptores, sizeof(DescriptorCod), 
			descriptores.numDescriptores, fuente);

	TablaDescriptoresMapear(&descriptores);


	// descompresion en archivo 'destino'
	char nombreDestino[128];
	strcpy(nombreDestino,nombreArchivo);
	strcat(nombreDestino, ".dhu");
	destino = fopen(nombreDestino, "wb");
	if (destino == NULL) return;

	int controlLectura, caracterEncontrado;
	struct buffer_8 lectura;
	struct buffer_32 codigo;
	DescriptorCod descriptor;
	while (!feof(fuente) || lectura.bits_restantes) {
		caracterEncontrado = 0;
		codigo.palabra = 0x0;
		codigo.numBits = 0;

		while (!caracterEncontrado) {
			if (!lectura.bits_restantes) {
				controlLectura = fread(&lectura.byte, 
					1, 1, fuente);
				if (controlLectura) {
					lectura.bits_restantes = 8;		// bits sin extraer
				}
				else break;
			}

			codigo.palabra = codigo.palabra << 1;
			++codigo.numBits;
			if (lectura.byte & 0x80) ++codigo.palabra;
			lectura.byte = lectura.byte << 1;
			--(lectura.bits_restantes);

			for (int i=0; i<descriptores.numDescriptores; i++) {
				descriptor = descriptores.arregloDescriptores[i];

				if (codigo.numBits == descriptor.numBitsCodigo && 
						codigo.palabra == descriptor.codigo) {
					caracterEncontrado = 1;
					break;
				}
			}
		}

		if (caracterEncontrado && numCaracteres) {
			fwrite(&descriptor.caracter, 1, 1, destino);
			--numCaracteres;
		}
	}


	fclose(destino);
	fclose(fuente);

	free((void *) descriptores.arregloDescriptores);
}

Arbol ListaOrdenadaCrear(unsigned int tablaFrecuencias[]) {
	// usada para copiar los nodos antes de ordenarlos
	NodoArbol arregloAuxiliar[TAM_TABLA_FRECUENCIAS];

	// se recorre la tabla para obtener los caracteres con frecuencia distinta de cero
	int cantidadCaracteres = 0;
	for (int i=0; i<TAM_TABLA_FRECUENCIAS; i++) {
		// si es distinto de cero
		if (tablaFrecuencias[i]) {
			arregloAuxiliar[cantidadCaracteres].caracter = i;
			arregloAuxiliar[cantidadCaracteres].frecuencia = tablaFrecuencias[i];

			arregloAuxiliar[cantidadCaracteres].ramaCero = NULL;
			arregloAuxiliar[cantidadCaracteres].ramaUno = NULL;
			arregloAuxiliar[cantidadCaracteres].siguiente = NULL;

			++cantidadCaracteres;
		}
	};

	Arbol lista = {
		.numNodos = cantidadCaracteres
	};

	// se ordena de menor a mayor
	for (int i = 0; i<cantidadCaracteres; i++) {
		for (int j = i+1; j<cantidadCaracteres; j++) {
			if (arregloAuxiliar[i].frecuencia > arregloAuxiliar[j].frecuencia) {
				NodoArbol auxiliar = arregloAuxiliar[i];
				arregloAuxiliar[i] = arregloAuxiliar[j];
				arregloAuxiliar[j] = auxiliar;
			}
		}
	}

	// se copia la lista ordenada en heap
	lista.primero = (NodoArbol *) malloc(sizeof(NodoArbol));
	*(lista.primero) = arregloAuxiliar[0];
	NodoArbol *ptr_lista = lista.primero, *ptr_siguiente;
	for (int i=1; i<lista.numNodos; i++) {
		ptr_siguiente = (NodoArbol *) malloc(sizeof(NodoArbol));
		*ptr_siguiente = arregloAuxiliar[i];
		ptr_lista->siguiente = ptr_siguiente;
		ptr_lista = ptr_siguiente;
	}
	lista.ultimo = ptr_lista;

	// devuelve la estructura de describe la lista ordenada de menor a mayor
	return lista;
}

void GenerarArbol(Arbol *arbol) {
	NodoArbol *ptr_menor = arbol->primero,
			  *ptr_menor2 = ptr_menor->siguiente,
			  *ptr_union;
	while (ptr_menor2->siguiente != NULL) {
		ptr_union = (NodoArbol *) malloc(sizeof(NodoArbol));
		ptr_union->frecuencia = ptr_menor->frecuencia + ptr_menor2->frecuencia;

		arbol->primero = ptr_menor2->siguiente;
		ptr_union->ramaUno = ptr_menor;
		ptr_union->ramaCero = ptr_menor2;
		ptr_menor->siguiente = NULL;
		ptr_menor2->siguiente = NULL;

		InsertarNodoOrdenado(arbol, ptr_union);
		ptr_menor = arbol->primero;
		ptr_menor2 = ptr_menor->siguiente;
	}

	ptr_union = (NodoArbol *) malloc(sizeof(NodoArbol));
	ptr_union->frecuencia = ptr_menor->frecuencia + ptr_menor2->frecuencia;
	if (ptr_menor->frecuencia < ptr_menor2->frecuencia) {
		ptr_union->ramaUno = ptr_menor;
		ptr_union->ramaCero = ptr_menor2;
	} else {
		ptr_union->ramaCero = ptr_menor;
		ptr_union->ramaUno = ptr_menor2;
	}

	arbol->primero = NULL;
	arbol->ultimo = ptr_union;
}


void InsertarNodoOrdenado(Arbol *arbol, NodoArbol *nuevo) {
	NodoArbol **cursor = &(arbol->primero);
	
	while ((*cursor)->siguiente != NULL ) {
		if ((*cursor)->siguiente->frecuencia > nuevo->frecuencia)
			break;
		cursor = &((*cursor)->siguiente);
	}

	nuevo->siguiente = (*cursor)->siguiente;
	(*cursor)->siguiente = nuevo;
	if (nuevo->siguiente == NULL) arbol->ultimo = nuevo;
}

void GenerarCodigos(DescriptorCod descriptores[], Arbol *arbol) {
	DescriptorCod codigo_inicial = {
		.numBitsCodigo = 0,
		.codigo = 0x0
	};

	DescriptorCod *cursor_descriptores = descriptores;

	GenerarCodigoHoja(&cursor_descriptores, arbol->ultimo, codigo_inicial);
}

void GenerarCodigoHoja(DescriptorCod **arreglo, NodoArbol *hoja, DescriptorCod cod) {
	if (hoja->ramaCero == NULL && hoja->ramaUno == NULL) {
		//arreglo[hoja->caracter].caracter = hoja->caracter;
		//arreglo[hoja->caracter].codigo = cod.codigo;
		//arreglo[hoja->caracter].numBitsCodigo = cod.numBitsCodigo;
		(*arreglo)->caracter = hoja->caracter;
		(*arreglo)->codigo = cod.codigo;
		(*arreglo)->numBitsCodigo = cod.numBitsCodigo;

		++(*arreglo);
	}
	else {
		++(cod.numBitsCodigo);
		cod.codigo = cod.codigo << 1;

		GenerarCodigoHoja(arreglo, hoja->ramaCero, cod);
		free((void *) hoja->ramaCero);
		++(cod.codigo);
		GenerarCodigoHoja(arreglo, hoja->ramaUno, cod);
		free((void *) hoja->ramaUno);
	}
}


/** Inicializa una tabla de frecuencias (arreglo de 256 números) con todos sus lugares
 * en cero.
 * Nota: Sobreescribe la tabla, independientemente de su contenido previo.
 *
 * @param tablaFrecuencias Arreglo de corresponde a la tabla de frecuencias.
 */
void TablaFrecuenciasInit(unsigned int tablaFrecuencias[]) {
	for (int i=0; i<TAM_TABLA_FRECUENCIAS; i++) {
		tablaFrecuencias[i] = 0;
	}
}
