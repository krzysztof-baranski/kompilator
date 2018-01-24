#include "global.h"
#include "parser.h"

using namespace std;

extern ofstream stream;
using namespace std;

stringstream stringStream;

// zwraca real jesli którykolwiek z argumentow jest typu real
int getResultType (int a, int b) {
	if (symbolTable[a].symbol_type == REAL_TKN || symbolTable[b].symbol_type == REAL_TKN) {
		return REAL_TKN;
	} else {
		return INTEGER_TKN;
	}
}

//zwraca typ symbolu, jeżeli jest referencją, to zwraca typ Integer
int getSymbolType (int i, bool value) {
	if (value) {
		return symbolTable[i].symbol_type;
	} else {
		return INTEGER_TKN;
	}
}

//konwersja dwóch zmiennych na ten sam typ
int setTypes (int &leftVar, bool leftValue, int &rightVar, bool rightValue) {
	int rightType = getSymbolType(rightVar, rightValue);
	int leftType = getSymbolType(leftVar, leftValue);

	if (rightType != leftType) {
		if (rightType == INTEGER_TKN && leftType == REAL_TKN) {
			int newRightVar = generateTmpVariable(REAL_TKN);
			generateTwoArgsOperation(INTTOREAL_TKN, rightVar, rightValue, newRightVar, rightValue);
			rightVar = newRightVar;
		} else if (leftType == INTEGER_TKN && rightType == REAL_TKN) {
			int newLeftVar = generateTmpVariable(REAL_TKN);
			generateTwoArgsOperation(INTTOREAL_TKN, leftVar, leftValue, newLeftVar, leftValue);
			leftVar = newLeftVar;
		} else {
			cout << "Nie rozpoznano typów zmiennych: " << symbolTable[leftVar].symbol_name.c_str() << " " << symbolTable[rightVar].symbol_name.c_str();
			yyerror("Nierozpoznano typów");
		}
	}
}

// sprawdza jakiego typu jest zmienna przechowujaca wynik, rzutuje na ten typ
bool setResultType (int &resultVar, bool resultValue, int &rightVar, bool rightValue) {
	int rightType = getSymbolType(rightVar, rightValue);
	int resultType = getSymbolType(resultVar, resultValue);

	if (rightType != resultType) {
		if (resultType == REAL_TKN && rightType == INTEGER_TKN) {
			generateTwoArgsOperation(INTTOREAL_TKN, rightVar, rightValue, resultVar, resultValue);
			return true;
		} else if (resultType == INTEGER_TKN && rightType == REAL_TKN) {
			generateTwoArgsOperation(REALTOINT_TKN, rightVar, rightValue, resultVar, resultValue);
			return true;
		} else {
			cout << "Nie rozpoznano typów zmiennych: " << symbolTable[resultVar].symbol_name.c_str() << " " << symbolTable[rightVar].symbol_name.c_str();
			yyerror("Nierozpoznano typów");
			return false;
		}
	} else {
		return false;
	}
}

// zapisuje do stream'u wynikowy kod asemblerowy
void writeVariable (int i, bool value) {
	if (symbolTable[i].is_reference) {
		if (value) {
			stringStream << "*"; //jeżeli referencja, to trzeba wyłuskać adres
		}

		if (symbolTable[i].is_global) {
			stringStream << symbolTable[i].symbol_address;
		} else {
			stringStream << "BP";
			if (symbolTable[i].symbol_address >= 0) {
				stringStream << "+" << symbolTable[i].symbol_address;
			} else {
				stringStream << symbolTable[i].symbol_address;
			}
		}
	} else {
		switch (symbolTable[i].symbol_token) {
			case NUM_TKN:
				stringStream << "#" << symbolTable[i].symbol_name; //wypisuje liczbę, liczby poprzedzone są #
				break;
			case ARRAY_TKN:
			case VAR_TKN:
				if (!value) {
					stringStream << "#";
				}
				
				if (symbolTable[i].is_global) {
					stringStream << symbolTable[i].symbol_address;
				} else {
					stringStream << "BP";
					if (symbolTable[i].symbol_address >= 0) {
						stringStream << "+" << symbolTable[i].symbol_address;
					} else {
						stringStream << symbolTable[i].symbol_address;
					}
				}	
				break;
			default:
				cout << "Nieprawidłowy typ danych " << tokenToString(symbolTable[i].symbol_token) << endl; 
				yyerror("Nieprawidłowy typ danych\n");
		}
	}
}

//generuje kod operacji jednoargumentowych
void generateOneArgOperation (int token, int var, bool value) {
	string operation = "i ";
	if (symbolTable[var].symbol_type == REAL_TKN) {
		operation = "r ";
	}

	switch (token) {
		case FUNCTION_TKN:
		case PROCEDURE_TKN:
			stringStream << "\n" << symbolTable[var].symbol_name << ":";
			stringStream << "\n\tenter.i #?"; // ? bo nie wiemy ile beda zajmowaly zmienne lokalne, podmienia sie po wyjsciu z fun/proc
			break;
		case JUMP_TKN:
			stringStream << "\n\tjump.i  #" << symbolTable[var].symbol_name;
			break;
		case LABEL_TKN:
			stringStream << "\n" << symbolTable[var].symbol_name << ":";
			break;
		case CALL_TKN:
			stringStream << "\n\tcall.i  #" << symbolTable[var].symbol_name;
			break;
		case INCSP_TKN:
			stringStream << "\n\tincsp.i "; //increase stack pointer
			writeVariable(var, value);
			break;
		case PUSH_TKN:
			stringStream << "\n\tpush.i \t";
			writeVariable(var, value);
			break;
		case WRITE_TKN:
			stringStream << "\n\twrite." << operation;
			writeVariable (var, value);
			break;
		case READ_TKN:
			stringStream << "\n\tread." << operation << " ";
			writeVariable(var, value);
			break;
		case RETURN_TKN:
			stringStream << "\n\treturn";
			string resultString;
			resultString = stringStream.str();
			stringStream.str(string());
			size_t position = resultString.find("#?");
			stringStream << "#" << -1 * generateVarPosition(string(""));
			resultString.replace(position, 2, stringStream.str());
			stream.write(resultString.c_str(), resultString.size());
			stringStream.str(string());
			break;
	}
}

//dla operacji dwuargumentowych
void generateTwoArgsOperation (int token, int leftVar, bool leftValue, int resultVar, bool resultValue) {
	string operation = "i ";
	if (symbolTable[resultVar].symbol_type == REAL_TKN) {
		operation = "r ";
	}

	switch (token) {
		case REALTOINT_TKN:
			stringStream << "\n\trealtoint.r ";
			writeVariable(leftVar, leftValue);
			stringStream << ", ";
			writeVariable(resultVar, resultValue);
			break;
		case INTTOREAL_TKN:
			stringStream << "\n\tinttoreal.i ";
			writeVariable(leftVar, leftValue);
			stringStream << ", ";
			writeVariable(resultVar, resultValue);
			break;
		case ASSIGNOP_TKN:
			if (!setResultType(leftVar, leftValue, resultVar, resultValue)) {
				stringStream << "\n\tmov." << operation << "\t";
				writeVariable(leftVar, leftValue);
				stringStream << ", ";
				writeVariable(resultVar, resultValue);
			}
 			break;
	}
}

void writeToStream (bool isEquality, const char* s, int &leftVar, bool leftValue, int &rightVar, bool rightValue, int &resultVar, bool resultValue) {
	string operation = "i ";

	if (symbolTable[resultVar].symbol_type == REAL_TKN) {
		operation = "r ";
	}

	setTypes(leftVar, leftValue, rightVar, rightValue);
	stringStream << "\n";
	stringStream << s;
	if (isEquality) {
		operation == "i ";
		if (symbolTable[leftVar].symbol_type == REAL_TKN) {
			operation = "r ";
		}
	}
	stringStream << operation << "\t";
	writeVariable(leftVar, leftValue);
	stringStream << ", ";
	writeVariable(rightVar, rightValue);
	stringStream << ", ";

	if (isEquality) {
		stringStream << "#" << symbolTable[resultVar].symbol_name;		
	} else {
		writeVariable(resultVar, resultValue);
	}
}

//dla operacji trójargumentowych
void generateThreeArgsOperation (int token, int leftVar, bool leftValue, int rightVar, bool rightValue, int resultVar, bool resultValue) {
	switch (token) {
		// COMPARISON OR MATHEMATICAL OPERATIONS!
		case OR_TKN:
			writeToStream(false, "\tor.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case AND_TKN:
			writeToStream(false, "\tand.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case DIV_TKN:
			writeToStream(false, "\tdiv.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case MOD_TKN:
			writeToStream(false, "\tmod.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case MUL_TKN:
			writeToStream(false, "\tmul.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case PLUS_TKN:
			writeToStream(false, "\tadd.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case MINUS_TKN:
			writeToStream(false, "\tsub.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		// EQUALITY OPERATIONS!
		case EQ_TKN:
			writeToStream(true, "\tje.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case NE_TKN:
			writeToStream(true, "\tjne.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case LT_TKN:
			writeToStream(true, "\tjl.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case LE_TKN:
			writeToStream(true, "\tjle.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case GE_TKN:
			writeToStream(true, "\tjge.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case GT_TKN:
			writeToStream(true, "\tjg.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
	}
}

void writeToOut (const char* s) {
	stringStream << s;
}

void saveToFile () {
	stream.write(stringStream.str().c_str(), stringStream.str().size());
	stringStream.str(string());
}
