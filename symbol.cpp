#include "global.h"
#include "parser.h"

using namespace std;

vector<symbolStruct> symbolTable;
int labelCounter = 1; //licznik dla etykiet
int tmpVarCounter = 0; //licznik dla zmiennych pomocniczych

int addToSymbolTable (const char* s, int type, int token) {
	symbolStruct sym;
	string name(s);
	sym.symbol_name = name;
	sym.symbol_token = token;
	sym.symbol_type = type;
	sym.is_global = isGlobal;
	sym.is_reference = false;
	sym.symbol_address = 0;
	symbolTable.push_back(sym);

	return symbolTable.size() - 1;
}
int generateLabel () {
	stringstream stringStream;
	stringStream << "lab" << labelCounter++;
	int labelID = addToSymbolTable(stringStream.str().c_str(), NONE_TKN, LABEL_TKN);

	return labelID;
}

// s - nazwa stalej liczbowej np. "30",
// type - int lub real
int addNum (const char* s, int type) {
	int index = findSymbolIndexByName(s);
	if (index == -1) {
		index = addToSymbolTable(s, type, NUM_TKN);
	}

	return index;
}

int getSymbolSize (symbolStruct sym) {
	if (sym.is_reference) {
		return 4;
	} else if (sym.symbol_token == VAR_TKN) {
		if (sym.symbol_type == REAL_TKN) {
			return 8;
		} else if (sym.symbol_type == INTEGER_TKN) {
			return 4;
		}
	} else if (sym.symbol_token == ARRAY_TKN) {
		int tabElemSize = 0;
		if (sym.symbol_type == REAL_TKN) {
			tabElemSize = 8;
		} else if (sym.symbol_type == INTEGER_TKN) {
			tabElemSize = 4;
		}
		int elemCount = sym.array.array_stopValue - sym.array.array_startValue + 1; // zeby sie zgadzala liczba elementow
		return elemCount * tabElemSize;
	}
	return 0;
}

//zwraca pozycję dla alokowanej zmiennej w części lokalnej i globalnej
int generateVarPosition (string name) {
	int position = 0;
	if (isGlobal) {
		for (int i = 0; i < symbolTable.size(); i++) {
			if (!symbolTable[i].is_global) {
				break;
			}
			if (symbolTable[i].symbol_name != name) {
				position += getSymbolSize(symbolTable[i]);
			}
		}
	} else {
		for (int i = 0; i < symbolTable.size(); i++) {
			if (!symbolTable[i].is_global) {
				if (symbolTable[i].symbol_address <= 0) {
					position -= getSymbolSize(symbolTable[i]);
				}
			}
		}
	}
	return position;
}

//tworzy zmienną tymczasową
int generateTmpVar (int type) {
	stringstream stringStream;
	stringStream << "$t" << tmpVarCounter++;
	int tmpId = addToSymbolTable(stringStream.str().c_str(), type, VAR_TKN);
	symbolTable[tmpId].symbol_address = generateVarPosition(stringStream.str().c_str());

	return tmpId;
}

int findSymbolIndexByName (const char* symbolName) {
	int i = symbolTable.size() - 1;
	for (i; i >= 0; i--) {
		if (symbolTable[i].symbol_name == symbolName) {
			return i;
		}
	}
	return -1;
}

int findSymbolIndexByScope (const char* symbolName) {
	int i = symbolTable.size() - 1;
	if (isGlobal) {  //przeszukujemy w zakresie globalnym od końca
		for (i; i >= 0; i--) { //przeszukiwanie części globalnej
			if (symbolTable[i].symbol_name == symbolName) { //znaleziono w części globalnej
				return i;
			}
		}
		return -1;
	} else {
		for (i; i >= 0; i--) {
			if (symbolTable[i].is_global) { //brak w części lokalnej
				return -1;
			}
			if (symbolTable[i].symbol_name == symbolName) { //znalziono w części lokalnej
				return i;
			}
		}
	}
}

int findSymbolIndexIfProcOrFunc (const char* symbolName) {
	int i = symbolTable.size() - 1;
	for (i; i >= 0; i--) {
		if (symbolTable[i].symbol_name == symbolName && (symbolTable[i].symbol_token == PROCEDURE_TKN || symbolTable[i].symbol_token == FUNCTION_TKN)) {
			return i;
		}
	}
	return -1;
}

// przeszukuje tablice symboli
//int lookup (const char* s, int flag) {
//	if (flag == 0) {  // szuka tylko po nazwach
//		for (int i = symbolTable.size() - 1; i >= 0; i--) {
//			if (symbolTable[i].symbol_name == s) {
//				return i;
//			}
//		}
//		return -1;
//	} else if (flag == 1) {  // szuka w globalnych albo w lokalnych
//		int i = symbolTable.size() - 1;
//		if (!isGlobal) {  //przeszukujemy w zakresie lokalnym od końca
//			for (i; i>=0; i--) {
//				if (symbolTable[i].is_global) { //brak w części lokalnej
//					return -1;
//				}
//				if (symbolTable[i].name == s) { //znalziono w części lokalnej
//					return i;
//				}
//			}
//		} else {
//			for (i; i >= 0; i--) { //przeszukiwanie części globalnej
//				if (symbolTable[i].symbol_name == s) { //znaleziono w części globalnej
//					return i;
//				}
//			}
//			return -1;
//		}
//	} else if (flag == 2) {  // dla funkcji lub procedur
//		int i = symbolTable.size() - 1;
//		for (i; i >= 0; i--) {
//			if (symbolTable[i].symbol_name == s && (symbolTable[i].symbol_token == PROCEDURE_TKN || symbolTable[i].symbol_token == FUNCTION_TKN)) {
//				return i;
//			}
//		}
//		return -1;
	}
//	return -1;
}

// usuwa z zakresu lokalnego po wyjsciu z funkcji/procedury
void clearLocalVars () {
	int tmp = 0;
	for (int i = 0; i < symbolTable.size(); i++) {
		if (!symbolTable[i].is_global) {
			break;
		}
		tmp++;
	}
	symbolTable.erase(symbolTable.begin() + tmp, symbolTable.end());
}

// do wypisywanie na konsoli
string tokenToString (int token) {
	switch (token) {
		case REAL_TKN:
			return string("Real");
		case INTEGER_TKN:
			return string("Integer");
		case PROCEDURE_TKN:
			return string("Procedure");
		case FUNCTION_TKN:
			return string("Function");
		case ARRAY_TKN:
			return string ( "Array");
		case ID_TKN:
			return string("Id");
		case NUM_TKN:
			return string("Number");
		case VAR_TKN:
			return string("Variable");
		case LABEL_TKN:
			return string("Label");
		default:
			return string("Inny token");
	}
	
//	if (token == REAL_TKN)  return string("Real");
//	else if (token == INTEGER_TKN)  return string("Integer");
//	else if (token == PROCEDURE_TKN)  return string("Procedure");
//	else if (token == FUNCTION_TKN)  return string("Function");
//	else if (token == ARRAY_TKN)  return string ( "Array");
//	else if (token == ID_TKN)  return string("Id");
//	else if (token == NUM_TKN)  return string("Number");
//	else if (token == VAR_TKN)  return string("Variable");
//	else if (token == LABEL_TKN)  return string("Label");
//	else return string("Inny token");
}

void printSymtable () {
	cout << "\nSymbol Table\n";
	for (int i = 0; i < symbolTable.size(); i++) {
		symbolStruct sym = symbolTable[i];
		cout << "; " << i;

		if (sym.is_global) {
			cout << " Global ";
		} else {
			cout << " Local ";
		}
		
		if (sym.is_reference) {
			cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " ref " << tokenToString(sym.symbol_type) << " addr offset " << sym.symbol_address << endl;
		} else {
			switch (sym.symbol_token) {
				case ID_TKN:
					cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << endl;
					break;
				case NUM_TKN:
					cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " " << tokenToString(sym.symbol_type) << endl;
					break;
				case VAR_TKN:
					cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " " << tokenToString(sym.symbol_type) << " addr offset " << sym.symbol_address << endl;
					break;
				case ARRAY_TKN:
					cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " array[" << sym.array.array_startValue << ".." << sym.array.array_stopValue << "] of" << tokenToString(sym.symbol_type) << " addr offset " << sym.symbol_address << endl;
					break;
				case LABEL_TKN;
				case PROCEDURE_TKN:
				case FUNCTION_TKN:
					cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << endl;
					break;
				default:
					cout << "OTHER" << sym.symbol_name << " " << sym.symbol_token << " " << sym.symbol_type << " " << sym.symbol_address << endl;
					break;
			}
		}
//		if (sym.symbol_token == ID_TKN) {
//			cout << tokenToString(sym.token) << " " << sym.symbol_name << endl;
//		} else if (sym.symbol_token == NUM_TKN) {
//			cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " " << tokenToString(sym.symbol_type) << endl;
//		} else if (sym.symbol_token == VAR_TKN) {
//			cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " " << tokenToString(sym.symbol_type) << " addr offset " << sym.symbol_address << endl;
//		} else if (sym.symbol_token == ARRAY_TKN) {
//			cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << " array[" << sym.array.array_startValue << ".." << sym.array.array_stopValue << "] of" << tokenToString(sym.symbol_type) << " addr offset " << sym.symbol_address << endl;
//		} else if (sym.symbol_token == LABEL_TKN || sym.symbol_token == PROCEDURE_TKN || sym.symbol_token == FUNCTION_TKN) {
//			cout << tokenToString(sym.symbol_token) << " " << sym.symbol_name << endl;
//		} else {
//			cout << "OTHER" << sym.symbol_name << " " << sym.symbol_token << " " << sym.symbol_type << " " << sym.symbol_address << endl;
//		}
	}
}
