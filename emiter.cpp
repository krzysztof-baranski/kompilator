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
int getSymbolType (int i, bool isRef) {
	if (isRef) {
		return INTEGER_TKN;
	} else {
		return symbolTable[i].symbol_type;
	}
}

//konwersja dwóch zmiennych na ten sam typ
int setTypes (int &leftVar, bool leftValue, int &rightVar, bool rightValue) {
	int rightType = getSymbolType(rightVar, rightValue);
	int leftType = getSymbolType(leftVar, leftValue);

	if (rightType != leftType) {
		if (rightType == INTEGER_TKN && leftType == REAL_TKN) {
			int newRightVar = generateTmpVar(REAL_TKN);
			generateTwoArgsOperation(INTTOREAL_TKN, rightVar, rightValue, newRightVar, rightValue);
			rightVar = newRightVar;
		} else if (leftType == INTEGER_TKN && rightType == REAL_TKN) {
			int newLeftVar = generateTmpVar(REAL_TKN);
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
void writeVar (int i, bool value) {
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
			}

			if (symbolTable[i].symbol_address < 0) {
				stringStream << symbolTable[i].symbol_address;
			}
		}
	} else if (symbolTable[i].symbol_token == NUM_TKN) {
		stringStream << "#" << symbolTable[i].symbol_name; //wypisuje liczbę, liczby poprzedzone są #
	} else if (symbolTable[i].symbol_token == ARRAY_TKN || symbolTable[i].symbol_token == VAR_TKN) {
		if (!value) {
			stringStream << "#";
		}
		
		if (symbolTable[i].is_global) {
			stringStream << symbolTable[i].symbol_address;
		} else {
			stringStream << "BP";
			if (symbolTable[i].address <= 0) {
				stringStream << symbolTable[i].symbol_address;
			}
		}
	} else {
		yyerror("Nieprawidłowy typ danych\n");
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
			writeVar(var, value);
			break;
		case PUSH_TKN:
			stringStream << "\n\tpush.i \t";
			writeVar(var, value);
			break;
		case WRITE_TKN:
			stringStream << "\n\twrite." << operation;
			writeVar (var, value);
			break;
		case READ_TKN:
			stringStream << "\n\tread." << operation << " ";
			writeVar(var, value);
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
//	if (token == FUNCTION_TKN || token == PROCEDURE_TKN) {
//		stringStream << "\n" << symbolTable[var].symbol_name << ":";
//		stringStream << "\n\tenter.i #?"; // ? bo nie wiemy ile beda zajmowaly zmienne lokalne, podmienia sie po wyjsciu z fun/proc
//	} else if (token == JUMP_TKN) {
//		stringStream << "\n\tjump.i  #" << symbolTable[var].symbol_name;
//	} else if (token == LABEL_TKN) {
//		stringStream << "\n" << symbolTable[var].symbol_name << ":";
//	} else if (token == CALL_TKN) {
//		stringStream << "\n\tcall.i  #" << symbolTable[var].symbol_name;
//	} else if (token == INCSP_TKN) {
//		stringStream << "\n\tincsp.i "; //increase stack pointer
//		writeVar(var,value);
//	} else if (token == PUSH_TKN) {
//		stringStream << "\n\tpush.i \t";
//		writeVar(var,value);
//	} else if (token == WRITE_TKN) {
//		stringStream << "\n\twrite." << operation;
//		writeVar (var,value);
//	} else if (token == READ_TKN) {
//		stringStream << "\n\tread." << operation << " ";
//		writeVar(var,value);
//	} else if (token == RETURN_TKN) {
//		stringStream << "\n\treturn";
//		string resultString;
//		resultString = stringStream.str();
//		stringStream.str(string());
//		size_t position = resultString.find("#?");
//		stringStream << "#" << -1*generateVarPosition(string(""));
//		resultString.replace(position, 2, stringStream.str());
//		stream.write(resultString.c_str(), resultString.size());
//		stringStream.str(string());
//	}
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
			writeVar(leftVar, leftValue);
			stringStream << ", ";
			writeVar(resultVar, resultValue);
			break;
		case INTTOREAL_TKN:
			stringStream << "\n\tinttoreal.i ";
			writeVar(leftVar, leftValue);
			stringStream << ", ";
			writeVar(resultVar, resultValue);
			break;
		case ASSIGNOP_TKN:
			if (!setResultType(resultVar, resultValue, leftVar, leftValue)) {
				stringStream << "\n\tmov." << operation << "\t";
				writeVar(leftVar, leftValue);
				stringStream << ", ";
				writeVar(resultVar, resultValue);
			}
     		break;
	}
	
//	if (token == REALTOINT_TKN) {
//		stringStream << "\n\trealtoint.r ";
//		writeVar(leftVar,leftValue);
//		stringStream << ", ";
//		writeVar(resultVar,resultValue);
//	} else if (token == INTTOREAL_TKN) {
//		stringStream << "\n\tinttoreal.i ";
//		writeVar(leftVar,leftValue);
//		stringStream << ", ";
//		writeVar(resultVar,resultValue);
//	} else if (token == ASSIGNOP_TKN) {
//		bool setTypes = setResultType(resultVar, resultValue, leftVar, leftValue);
//		if (setTypes == true) {
//			return;
//		} else {
//			stringStream << "\n\tmov." << operation << "\t";
//			writeVar(leftVar, leftValue);
//			stringStream << ", ";
//			writeVar(resultVar, resultValue);
//		}
//	}
}

void _writeToStream (bool isEquality, string s, int leftVar, bool leftValue, int rightVar, bool rightValue, int resultVar, bool resultValue) {
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
	writeVar(leftVar, leftValue);
	stringStream << ", ";
	writeVar(rightVar, rightValue);
	stringStream << ", ";
	if (isEquality) {
		stringStream << "#" << symbolTable[resultVar].symbol_name;
	} else {
		writeVar(resultVar, resultValue);
	}
}

//dla operacji trójargumentowych
void generateThreeArgsOperation (int token, int leftVar, bool leftValue, int rightVar, bool rightValue, int resultVar, bool resultValue) {
//	string tokenString = "";
//	string operation = "i ";
//	
//	if (symbolTable[resultVar].symbol_type == REAL_TKN) {
//		operation = "r ";
//	}
	
	switch (token) {
		// COMPARISON OR MATHEMATICAL OPERATIONS!
		case OR_TKN:
			_writeToStream(false, "\tor.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case AND_TKN:
			_writeToStream(false, "\tand.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case DIV_TKN:
			_writeToStream(false, "\tdiv.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case MOD_TKN:
			_writeToStream(false, "\tmod.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case MUL_TKN:
			_writeToStream(false, "\tmul.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case PLUS_TKN:
			_writeToStream(false, "\tadd.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case MINUS_TKN:
			_writeToStream(false, "\tsub.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		// EQUALITY OPERATIONS!
		case EQ_TKN:
			_writeToStream(true, "\tje.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case NE_TKN:
			_writeToStream(true, "\tjne.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break
		case LT_TKN:
			_writeToStream(true, "\tjl.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case LE_TKN:
			_writeToStream(true, "\tjle.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case GE_TKN:
			_writeToStream(true, "\tjge.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
		case GT_TKN:
			_writeToStream(true, "\tjg.", leftVar, leftValue, rightVar, rightValue, resultVar, resultValue);
			break;
	}
//		}
	
//	if (token == OR_TKN || token == AND_TKN || token == DIV_TKN || token == MOD_TKN || token == MUL_TKN || token == PLUS_TKN || token == MINUS_TKN) {
//		setTypes(leftVar, leftValue, rightVar, rightValue);
//		stringStream << "\n";

//		switch (token) {
//			case OR_TKN:
//				tokenString = "\tor.";
//				break;
//			case AND_TKN:
//				tokenString = "\tand.";
//				break;
//			case DIV_TKN:
//				tokenString = "\tdiv.";
//				break;
//			case MOD_TKN:
//				tokenString = "\tmod.";
//				break;
//			case MUL_TKN:
//				tokenString = "\tmul.";
//				break;
//			case PLUS_TKN:
//				tokenString = "\tadd.";
//				break;
//			case MINUS_TKN:
//				tokenString = "\tsub.";
//				break;
//		}
//		stringStream << tokenString;
//		
////		stringStream << operation << "\t";
////		writeVar(leftVar, leftValue);
////		stringStream << ", ";
////		writeVar(rightVar, rightValue);
////		stringStream << ", ";
////		writeVar(resultVar, resultValue);
////		if (token == OR_TKN) {
////			stringStream << "\tor.";
////		} else if (token == AND_TKN) {
////			stringStream << "\tand.";
////		} else if (token == DIV_TKN) {
////			stringStream << "\tdiv.";
////		} else if (token == MOD_TKN) {
////			stringStream << "\tmod.";
////		} else if (token == MUL_TKN) {
////			stringStream << "\tmul.";
////		} else if (token == PLUS_TKN) {
////			stringStream << "\tadd.";
////		} else if (token == MINUS_TKN) {
////			stringStream << "\tsub.";
//		}
//		stringStream << operation << "\t";
//		writeVar(leftVar, leftValue);
//		stringStream << ", ";
//		writeVar(rightVar, rightValue);
//		stringStream << ", ";
//		writeVar(resultVar, resultValue);
//	} else if (token == EQ_TKN || token == NE_TKN || token == LT_TKN || token == LE_TKN || token == GE_TKN || token == GT_TKN) {
//		setTypes(leftVar, leftValue, rightVar, rightValue);
//		operation == "i ";
//		stringStream << "\n";
//		if (symbolTable[leftVar].symbol_type == REAL_TKN) {
//			operation = "r ";
//		}
		
			
///
////		if (token == EQ_TKN) {
////			stringStream << "\tje.";
////		} else if (token == NE_TKN) {
////			stringStream << "\tjne.";
////		} else if (token == LT_TKN) {
////			stringStream << "\tjl.";
////		} else if (token == LE_TKN) {
////			stringStream << "\tjle.";
////		} else if (token == GE_TKN) {
////			stringStream << "\tjge.";
////		} else if (token == GT_TKN) {
////			stringStream << "\tjg.";
////		}

//		setTypes(leftVar, leftValue, rightVar, rightValue);
//		operation == "i ";
//		stringStream << "\n";
//		if (symbolTable[leftVar].symbol_type == REAL_TKN) {
//			operation = "r ";
//		}
//		
//		stringStream << tokenString;
//		stringStream << operation << "\t";
//		writeVar(leftVar, leftValue);
//		stringStream << ", ";
//		writeVar(rightVar, rightValue);
//		stringStream << ", ";
//		stringStream << "#" << symbolTable[resultVar].symbol_name;
	}
}

void writeToOut (const char* s) {
	stringStream << s;
}

void saveToFile () {
	stream.write(stringStream.str().c_str(), stringStream.str().size());
	stringStream.str(string());
}
