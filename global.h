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
struct arrayStruct {
	int array_argType; //typ argumentów tablicy
	int array_start; //indeks pocz¹tkowy
	int array_stop; //indeks koñcowy
	int array_startValue;
	int array_stopValue;
};

//struktura reprezentuj¹ca wpis w tablicy symboli
struct symbolStruct {
	string symbol_name; //nazwa
	int symbol_type;	//real lub integer lub none
	int symbol_address; //adres przydzielony
	int symbol_token;	//typ tokenu
	bool is_global;	//zmianna lokalna czy globalna
	bool is_reference;	//czy referencja
	arrayStruct array; //dane dla tablicy
	list<pair<int, arrayStruct> > parameters; //lista parametrów funkcji/procedury
		// pair, bo: int to indeks w tablicy symboli, arrayStruct to jesli jest przekazywana tablica  
};

// extern oznacza ze deklaracja jest w innym miejscu
extern vector<symbolStruct> symbolTable;	//tablica symboli0
extern FILE* yyin;	//plik wejściowy dla lexera
extern bool isGlobal; //je¿eli true to zmienna globalna, je¿eli false to zmienna lokalna
extern int lineno; //numer linii
extern ofstream stream; //strumieñ plikowy do zapisu

int yylex();	//uruchamia lexer
int yylex_destroy();
int yyparse();	//uruchamia parser
void yyerror(char const* s);	//funkcja do ob³ugi b³êdu parsera

int addToSymbolTable(const char* s, int type, int token); //dodaje element do tablicy symboli
int addNum(const char*, int);	//wstawia liczbê od tablicy symboli, jeœli ta jej nie zawiera
int generateLabel(); //tworzy now¹ etykietê do skoku
int generateTmpVar(int type); //tworzy now¹ zmienn¹ tymczasow¹
int generateVarPosition(string symName=""); //zwraca indeks, pod którym bêdzie nowa zmienna
int getResultType(int a,int b); //zwraca typ zmiennej wynikowej
int findSymbolIndexByName(const char* symbolName); // przeszukuje tablic� symboli po nazwach
int findSymbolIndexByScope(const char* symbolName);  // przeszukuje tablic� symboli po nazwach, ale w zaleznosci czy global czy local
int findSymbolIndexIfProcOrFunc(const char* symbolName); // przeszukuje tablic� symboli szukajac funkcji lub procedury
int getSymbolSize(symbolStruct sym); //zwraca rozmiar elementu
string tokenToString(int token);	//zwraca string dla tokena
void generateOneArgOperation(int token, int var, bool value);
void generateTwoArgsOperation(int token, int leftVar, bool leftValue, int resultVar, bool resultValue);
void generateThreeArgsOperation(int token, int leftVar, bool leftValue, int rightVar, bool rightValue, int resultVar, bool resultValue);
void writeToOut(const char* s); //bezpoœredni zapis do pliku
void clearLocalVars();
void saveToFile(); //zapisuje wszystko do pliku wyjœciowego
void printSymbolTable();	//wypisuje elementy z tablicy symboli
