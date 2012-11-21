/*
	< DanaSah is a chess engine winboard. >
    Copyright (C) <2012>  <Pedro Castro Elgarresta>
	E-mail <pecastro@msn.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
Funciones para manejar las tablas hash y evaluaci�n cache. Quiero dar las gracias a
Miguel Izquierdo (Popochin) sin cuya ayuda hoy en d�a no se si DanaSah tendr�a tablas
hash.

Las tablas hash actuan durante la b�squeda, en el momento de la b�squeda cuando una posici�n
se visita, guardamos informaci�n de esa posici�n, no sabemos el valor exacto en ese momento
de la evaluaci�n, pero si que tenemos algo de informaci�n, vamos a poder cortar con las tablas
hash muchas ramas de los �rboles ahorrando muchos nodos, vamos a conseguir con ellas de 1 a 2
plys m�s en el medio juego y m�s de 4 en el final.

En el caso de la evaluaci�n cach�, actuan en el momento que estamos evaluando una posici�n, si
hemos hecho una evaluaci�n completa guardamos la llave de la posici�n y la evaluaci�n, si volvemos
a visitar esa posici�n no tendremos que evaluarla sino retorna ya su valor, la evaluaci�n cache
no nos ahorra nodos pero si un poco de trabajo, en mi programa da aproximadamente un 5% de
velocidad.

*/

/*Includes*/
#include <stdio.h>		/*prinft, scanf, sprinft, sscanf, fgets...*/
#include <string.h>
#include <stdlib.h>		/*malloc, free ...*/
#include "definiciones.h"
#include "variables.h"
#include "funciones.h"

/*Aqu� defino unas variables para saber el n�mero de veces que entramos en las tablas hash
para almacenar informaci�n, el n�mero de veces que luego probamos y el n�mero de veces que
tenemos �xito, el programa luego de momento no imprime estos resultados*/
long TTProbes;
long TTHits;
long TTStores;

/*lo mismo para la evaluaci�n cach�*/
long ETProbes;
long ETHits;
long ETStores;

TType *TTable;
EType *ETable;

/*iniciamos las tablas con n�mero aleatorios*/
void inicia_hash(void) 
{
	int i, j, k;

	for (i = 0; i < 2; ++i)
		for (j = 0; j < 6; ++j)
			for (k = 0; k < 64; ++k)
				hash_piezas[i][j][k] = Rand64();
	hash_turno[BLANCO] = Rand64();
	hash_turno[NEGRO] = Rand64();
	for (i = 0; i < 64; ++i)
		hash_alpaso[i] = Rand64();
	for (i = 0; i < 16; ++i)
		hash_enroque[i] = Rand64();
}

/*poner_hash() usa llaves de Zobrist para generar n�meros �nicos*/
void poner_hash(void)
{
	int i;

	hash = 0;
	for (i = 0; i < 64; ++i)
		if (color[i] != VACIO)
			hash ^= hash_piezas[color[i]][pieza[i]][i];
	hash ^= hash_turno[turno];
	if (alpaso != -1)
		hash ^= hash_alpaso[alpaso];
	hash ^= hash_enroque[enroque];
}

/*borramos las tablas hash y evaluaci�n cach�*/
void clear_hash(void) {
	memset(TTable, 0, sizeof(TType) * TTSize);
	memset(ETable, 0, sizeof(EType) * ETSize);
}

/*almacenamos informaci�n en las tablas hash*/
void store_hash(int type, int depth, int score, movimiento hmov) {
	U64 index;

	index = hash % TTSize;

	if (TTable[index].Depth > depth)
		return;

	if (abs (score) > MATE - 64) {
		if (score > 0)
			score += ply;
		else
			score -= ply;
	}

	/* deep prefered */
	TTable[index].Hash = hash;
	TTable[index].Type = type;
	TTable[index].Depth = depth;
	TTable[index].Value = score;
	TTable[index].Mov = hmov;

	return;
}

/*probamos si la posici�n la tenemos almacenada en las tablas hash*/
int probe_hash(int *type, int depth, int beta, movimiento *hmov, int *hacer_nulo) {
	U64 index;
	int score;

	index = hash % TTSize;

	*type = hasfNULL;
	*hmov = no_move;
	*hacer_nulo = VERDADERO;

	if (TTable[index].Hash == hash) {
		*hmov = TTable[index].Mov;
		score = TTable[index].Value;
		if (TTable[index].Depth >= depthnull && score < beta && TTable[index].Type == hasfALPHA)
			*hacer_nulo = FALSO;
		if (TTable[index].Depth >= depth) {
			if (abs (score) > MATE - 64) {
				if (score > 0)
					score -= ply;
				else
					score += ply;
			}
			*type = TTable[index].Type;
			return score;
		}
	}
	return valUNKNOWN;
}

/*almacenamos informaci�n sobre la evaluaci�n de una posici�n*/
void store_eval(int score)
{
	U64 index;

	ETStores++;

	index = hash % ETSize;

	ETable[index].Hash = hash;
	ETable[index].Bound = score;

	return;
}

/*comprobamos si una posici�n ya la hemos evaluado antes*/
int probe_eval(void)
{
	U64 index;

	ETProbes++;

	index = hash % ETSize;

	if (ETable[index].Hash == hash) {
		ETHits++;
		return (ETable[index].Bound);
	}
	return -MATE;
}

/*asignamos memoria para las tablas hash y evaluaci�n cach�*/
void alloc_hash(void) {
	TTable = (TType *) malloc(sizeof(TType) * TTSize);
	ETable = (EType *) malloc(sizeof(EType) * ETSize);

	if (TTable == NULL || ETable == NULL) {
		printf("# Out of memory allocating hash or evalcache.\n");
		exit(1);
	}

	return;
}

/*liberamos memoria de las tablas hash y de la evaluaci�n cach� (cuando salimos del programa)*/
void free_hash(void) {
	free(TTable);
	free(ETable);
	return;
}
