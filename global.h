#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


using namespace std;

//struktura przechowuj¹ca dane tablicy
struct arrayStruct{
	int array_argType; //typ argumentów tablicy
	int array_start; //indeks pocz¹tkowy
	int array_stop; //indeks koñcowy
	int array_startValue;
	int array_stopValue;
};

//struktura reprezentuj¹ca wpis w tablicy symboli
struct symbol{
	string name; //nazwa
	int type;	//real lub integer lub none
	int address; //adres przydzielony
	int token;	//typ tokenu
	bool global;	//zmianna lokalna czy globalna
	bool reference;	//czy referencja
	arrayStruct array; //dane dla tablicy
	list<pair<int,arrayStruct>> parameters; //lista parametrów funkcji/procedury
		// pair, bo: int to indeks w tablicy symboli, arrayStruct to jesli jest przekazywana tablica  
};

// extern oznacza ze deklaracja jest w innym miejscu
extern vector<symbol> symbolTable;	//tablica symboli0
extern FILE* yyin;	//plik wejściowy dla lexera
extern bool isGlobal; //je¿eli true to zmienna globalna, je¿eli false to zmienna lokalna
extern int lineno; //numer linii
extern ofstream stream; //strumieñ plikowy do zapisu

int yylex();	//uruchamia lexer
int yylex_destroy();
int yyparse();	//uruchamia parser
void yyerror(char const* s);	//funkcja do ob³ugi b³êdu parsera

int addToST(const char* s, int type, int token); //dodaje element do tablicy symboli
int addNum(const char*, int);	//wstawia liczbê od tablicy symboli, jeœli ta jej nie zawiera
int generateLabel(); //tworzy now¹ etykietê do skoku
int generateTmpVar(int type); //tworzy now¹ zmienn¹ tymczasow¹
int generateVarPosition(string symName=""); //zwraca indeks, pod którym bêdzie nowa zmienna
int generateResultType(int a,int b); //zwraca typ zmiennej wynikowej
int lookup(const char* s, int flag);
int getSymbolSize(symbol sym); //zwraca rozmiar elementu
string tokenToStr(int token);	//zwraca string dla tokena
void emitOne(int op, int var, bool value);
void emitTwo(int op, int lVar, bool lValue, int resultVar, bool resultValue);
void emitThree(int op, int lVar, bool lValue, int rVar, bool rValue, int resultVar, bool resultValue);
void writeToOut(const char* s); //bezpoœredni zapis do pliku
void clearLocalVars();
void saveToFile(); //zapisuje wszystko do pliku wyjœciowego
void printSymtable();	//wypisuje elementy z tablicy symboli
