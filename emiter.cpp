#include "global.h"
#include "parser.h"

using namespace std;

extern ofstream stream;
using namespace std;

stringstream stringStream;

// zwraca real jesli którykolwiek z argumentow jest typu real
int generateResultType (int a, int b) {
	if (symbolTable[a].type == REAL_TKN || symbolTable[b].type == REAL_TKN) {
		return REAL_TKN;
	} else {
		return INTEGER_TKN;
	}
}

//zwraca typ symbolu, jeżeli jest referencją, to zwraca typ Integer
int getSymbolType (int i, bool isRef) {
	int type;
	if (isRef) {
		type = INTEGER_TKN;
	} else {
		type = symbolTable[i].type;
	}
	return type;
}

//konwersja dwóch zmiennych na ten sam typ
int setTypes (int &leftVar, bool leftValue, int &rightVar, bool rightValue) {
	int rightType = getSymbolType(rightVar, rightValue);
	int leftType = getSymbolType(leftVar, leftValue);
	if (rightType != leftType) {
		if (rightType == INTEGER_TKN && leftType == REAL_TKN) {
			int newRightVar = generateTmpVar(REAL_TKN);
			emitTwo(INTTOREAL_TKN, rightVar, rightValue, newRightVar, rightValue);
			rightVar = newRightVar;
		} else if (leftType == INTEGER_TKN && rightType == REAL_TKN) {
			int newLeftVar = generateTmpVar(REAL_TKN);
			emitTwo(INTTOREAL_TKN, leftVar, leftValue, newLeftVar, leftValue);
			leftVar = newLeftVar;
		} else {
			cout << "Nie rozpoznano typów zmiennych: " << symbolTable[leftVar].name.c_str() << " " << symbolTable[rightVar].name.c_str();
			yyerror("Nierozpoznano typów");
		}
	}
}

// sprawdza jakiego typu jest zmienna przechowujaca wynik, rzutuje na ten typ
bool setAssignTypes (int &resultVar, bool resultValue, int &rightVar, bool rightValue) {
	int rightType = getSymbolType(rightVar, rightValue);
	int resultType = getSymbolType(resultVar, resultValue);

	if (rightType != resultType) {
		if (resultType == REAL_TKN && rightType == INTEGER_TKN) {
			emitTwo(INTTOREAL_TKN, rightVar, rightValue, resultVar, resultValue);
			return true;
		} else if (resultType == INTEGER_TKN && rightType == REAL_TKN) {
			emitTwo(REALTOINT_TKN, rightVar, rightValue, resultVar, resultValue);
			return true;
		} else {
			cout << "Nie rozpoznano typów zmiennych: " << symbolTable[resultVar].name.c_str() << " " << symbolTable[rightVar].name.c_str();
			yyerror("Nierozpoznano typów");
			return false;
		}
	} else {
		return false;
	}
}

// zapisuje do stream'u wynikowy kod asemlerowy
void writeVar (int i, bool value) {
	if (symbolTable[i].token == NUM_TKN) {
		stringStream << "#" << symbolTable[i].name; //wypisuje liczbę, liczby poprzedzone są #
	} else if (symbolTable[i].reference == true) {
		if (value) {
			stringStream << "*"; //jeżeli referencja, to trzeba wyłuskać adres
		}
		if (symbolTable[i].global == false) {
			stringStream << "BP";

			if (symbolTable[i].address >= 0) {
				stringStream << "+" << symbolTable[i].address;
			}
			if (symbolTable[i].address < 0) {
				stringStream << symbolTable[i].address;
			}
		} else {
			stringStream << symbolTable[i].address;
		}
	} else if (symbolTable[i].token == ARRAY_TKN || symbolTable[i].token == VAR_TKN) {
		if (!value) {
			stringStream << "#";
		}
		if (symbolTable[i].global == false) {
			stringStream << "BP";
			if (symbolTable[i].address <= 0) {
				stringStream << symbolTable[i].address;
			}
		} else {
			stringStream << symbolTable[i].address;
		}
	} else {
		yyerror("Nieprawidłowy typ danych\n");
	}
}

//generuje kod operacji jednoargumentowych
void emitOne (int op, int var, bool value) {
	string operation = "i ";
	if (symbolTable[var].type == REAL_TKN) {
		operation = "r ";
	}
	if (op == FUNCTION_TKN || op == PROCEDURE_TKN) {
		stringStream << "\n" << symbolTable[var].name << ":";
		stringStream << "\n\tenter.i #?"; // ? bo nie wiemy ile beda zajmowaly zmienne lokalne, podmienia sie po wyjsciu z fun/proc
	} else if (op == JUMP_TKN) {
		stringStream << "\n\tjump.i  #" << symbolTable[var].name;
	} else if (op == LABEL_TKN) {
		stringStream << "\n" << symbolTable[var].name << ":";
	} else if (op == CALL_TKN) {
		stringStream << "\n\tcall.i  #" << symbolTable[var].name;
	} else if (op == INCSP_TKN) {
		stringStream << "\n\tincsp.i "; //increase stack pointer
		writeVar(var,value);
	} else if (op == PUSH_TKN) {
		stringStream << "\n\tpush.i \t";
		writeVar(var,value);
	} else if (op == WRITE_TKN) {
		stringStream << "\n\twrite." << operation;
		writeVar (var,value);
	} else if (op == READ_TKN) {
		stringStream << "\n\tread." << operation << " ";
		writeVar(var,value);
	} else if (op == RETURN_TKN) {
		stringStream << "\n\treturn";
		string resultString;
		resultString = stringStream.str();
		stringStream.str(string());
		size_t position = resultString.find("#?");
		stringStream << "#" << -1*generateVarPosition(string(""));
		resultString.replace(position, 2, stringStream.str());
		stream.write(resultString.c_str(), resultString.size());
		stringStream.str(string());
	}
}

//dla operacji dwuargumentowych
void emitTwo (int op, int leftVar, bool leftValue, int resultVar, bool resultValue) {
	string operation = "i ";
	if (symbolTable[resultVar].type == REAL_TKN) {
		operation = "r ";
	}
	if (op == REALTOINT_TKN) {
		stringStream << "\n\trealtoint.r ";
		writeVar(leftVar,leftValue);
		stringStream << ", ";
		writeVar(resultVar,resultValue);
	} else if (op == INTTOREAL_TKN) {
		stringStream << "\n\tinttoreal.i ";
		writeVar(leftVar,leftValue);
		stringStream << ", ";
		writeVar(resultVar,resultValue);
	} else if (op == ASSIGNOP_TKN) {
		bool setTypes = setAssignTypes(resultVar, resultValue, leftVar, leftValue);
		if (setTypes == true) {
			return;
		} else {
			stringStream << "\n\tmov." << operation << "\t";
			writeVar(leftVar, leftValue);
			stringStream << ", ";
			writeVar(resultVar, resultValue);
		}
	}
}

//dla operacji trójargumentowych
void emitThree (int op, int leftVar, bool leftValue, int rightVar, bool rightValue, int resultVar, bool resultValue) {
	string operation = "i ";
	if (symbolTable[resultVar].type == REAL_TKN) {
		operation = "r ";
	}
	if (op == OR_TKN || op == AND_TKN || op == DIV_TKN || op == MOD_TKN || op == MUL_TKN || op == PLUS_TKN || op == MINUS_TKN) {
		setTypes(leftVar,leftValue,rightVar,rightValue);
		stringStream << "\n";
		if (op == OR_TKN) {
			stringStream << "\tor.";
		} else if (op == AND_TKN) {
			stringStream << "\tand.";
		} else if (op == DIV_TKN) {
			stringStream << "\tdiv.";
		} else if (op == MOD_TKN) {
			stringStream << "\tmod.";
		} else if (op == MUL_TKN) {
			stringStream << "\tmul.";
		} else if (op == PLUS_TKN) {
			stringStream << "\tadd.";
		} else if (op == MINUS_TKN) {
			stringStream << "\tsub.";
		}
		stringStream << operation << "\t";
		writeVar(leftVar, leftValue);
		stringStream << ", ";
		writeVar(rightVar, rightValue);
		stringStream << ", ";
		writeVar(resultVar,resultValue);
	} else if (op == EQ_TKN || op == NE_TKN || op == LT_TKN || op == LE_TKN || op == GE_TKN || op == GT_TKN) {
		setTypes(leftVar, leftValue, rightVar, rightValue);
		operation == "i ";
		stringStream << "\n";
		if (symbolTable[leftVar].type == REAL_TKN) {
			operation = "r ";
		}
		if (op == EQ_TKN) {
			stringStream << "\tje.";
		} else if (op == NE_TKN) {
			stringStream << "\tjne.";
		} else if (op == LT_TKN) {
			stringStream << "\tjl.";
		} else if (op == LE_TKN) {
			stringStream << "\tjle.";
		} else if (op == GE_TKN) {
			stringStream << "\tjge.";
		} else if (op == GT_TKN) {
			stringStream << "\tjg.";
		}
		stringStream << operation << "\t";
		writeVar(leftVar,leftValue);
		stringStream << ", ";
		writeVar(rightVar,rightValue);
		stringStream << ", ";
		stringStream << "#" << symbolTable[resultVar].name;
	}
}

void writeToOut (const char* s) {
	stringStream << s;
}

void saveToFile () {
	stream.write(stringStream.str().c_str(), stringStream.str().size());
	stringStream.str(string());
}
