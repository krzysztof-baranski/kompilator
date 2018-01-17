#include "global.h"
#include "parser.h"

using namespace std;
bool isGlobal = true;
ofstream stream;

int main (int argc, char *argv[]) {
	const char* inputFileName;
	const char* outputFileName = "output.asm";
	FILE* inputFile;
	stringstream stringStream;
	symbolStruct lab0, write, read;

	if (argc < 2) {
		cerr << "Nie podano pliku do przetworzenia" << '\n';
		return -1;
	} else if (argc == 2) {
		inputFileName = argv[1];
	}

	inputFile = fopen(inputFileName, "r");

	if (!inputFile) {
		cerr << "Nie znaleniono pliku o takiej nazwie" << '\n';
		return -1;
	}
	yyin = fopen(inputFileName, "r"); //plik wejściowy dla lexera

	isGlobal = true;
	stream.open(outputFileName, ofstream::trunc);

	if (!stream.is_open()) {
		cerr << "Nie można utworzyć pliku wyjściowego" << '\n';
		return -1;
	}

	read.symbol_name 	= string("read");
	read.symbol_token 	= PROCEDURE_TKN;
	read.is_global 		= true;
	read.is_reference 	= false;
	symbolTable.push_back(read);

	write.symbol_name 	= string("write");
	write.symbol_token 	= PROCEDURE_TKN;
	write.is_global 	= true;
	write.is_reference	= false;
	symbolTable.push_back(write);

	lab0.symbol_name 	= string("lab0");
	lab0.symbol_token 	= LABEL_TKN;
	lab0.is_global 		= true;
	lab0.is_reference 	= false;
	symbolTable.push_back(lab0);

	stringStream << "\tjump.i #" << lab0.symbol_name;
	stream.write(stringStream.str().c_str(), stringStream.str().size());

	yyparse();
	printSymtable();
	stream.close();
	fclose(inputFile);
	yylex_destroy();

	return 0;
}
