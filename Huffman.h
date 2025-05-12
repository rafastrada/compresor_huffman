#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>

#define TAM_TABLA_FRECUENCIAS 256

typedef struct _listaNodo NodoArbol;
typedef struct _lista Arbol;
struct _listaNodo {
	uint8_t caracter;
	unsigned int frecuencia;

	NodoArbol *ramaCero, *ramaUno, *siguiente;
};
struct _lista {
	NodoArbol *primero, *ultimo;
	int numNodos;
};

typedef struct _descriptor_cod DescriptorCod;
typedef struct _descriptor_tabla TablaDescriptores;
struct _descriptor_cod {
	uint8_t caracter, numBitsCodigo;
	uint32_t codigo;
};
struct _descriptor_tabla {
	DescriptorCod *arregloDescriptores;
	uint32_t numDescriptores;
	DescriptorCod *mapeador[TAM_TABLA_FRECUENCIAS];
};


int ArchivoContarFrecuencia(char nombreArchivo[], unsigned int tablaFrecuencias[]);
void ArchivoCodificar(char nombreArchivo[]);
void ArchivoDecodificar(char nombreArchivo[]);

void TablaFrecuenciasInit(unsigned int tablaFrecuencias[]);

void TablaDescriptoresMapear(TablaDescriptores *tabla);
void TablaDescriptoresAlinearCod(TablaDescriptores *tabla);

Arbol ListaOrdenadaCrear(unsigned int tablaFrecuencias[]);

void InsertarNodoOrdenado(Arbol *arbol, NodoArbol *nuevo);

void GenerarArbol(Arbol *arbol);

void GenerarCodigos(DescriptorCod descriptores[], Arbol *arbol);
void GenerarCodigoHoja(DescriptorCod **arreglo, NodoArbol *hoja, DescriptorCod cod);

#endif /* ifndef HUFFMAN_H */
