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
	symbol lab0, write, read;

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

	read.name 		= string("read");
	read.token 		= PROCEDURE_TKN;
	read.global 	= true;
	read.reference 	= false;
	symbolTable.push_back(read);

	write.name 		= string("write");
	write.token 	= PROCEDURE_TKN;
	write.global 	= true;
	write.reference = false;
	symbolTable.push_back(write);

	lab0.name 		= string("lab0");
	lab0.token 		= LABEL_TKN;
	lab0.global 	= true;
	lab0.reference 	= false;
	symbolTable.push_back(lab0);

	stringStream << "\tjump.i #" << lab0.name;
	stream.write(stringStream.str().c_str(), stringStream.str().size());

	yyparse();
	printSymtable();
	stream.close();
	fclose(inputFile);
	yylex_destroy();

	return 0;
}
