%{

#include "global.h"

using namespace std;

vector<int> argumentsTmp; //lista dla zmiennych, którym przypisywany jest typ danych
list<int> funParams; //lista do obliczenia wartości incsp
arrayStruct arrayTmp;
list<pair<int, arrayStruct>> parameters;
int arrayType; //zmienna pomocnicza przechowująca typ danych tablicy
int paramsOffset = 8; //8 dla procedur, 12 dla funkcji
void yyerror (const char* text);

%}

%token PROGRAM_TKN
%token ID_TKN
%token VAR_TKN
%token ARRAY_TKN
%token NUM_TKN
%token OF_TKN
%token INTEGER_TKN
%token REAL_TKN
%token FUNCTION_TKN
%token PROCEDURE_TKN
%token BEGIN_TKN
%token END_TKN
%token ASSIGNOP_TKN
%token IF_TKN
%token THEN_TKN
%token ELSE_TKN
%token WHILE_TKN
%token DO_TKN
%token OR_TKN
%token NOT_TKN
%token SIGN_TKN
%token RELOP_TKN
%token MULOP_TKN
%token DONE_TKN 0
%token NONE_TKN
%token EQ_TKN
%token NE_TKN
%token GE_TKN
%token GT_TKN
%token LE_TKN
%token LT_TKN
%token MUL_TKN
%token DIV_TKN
%token AND_TKN
%token MOD_TKN
%token PLUS_TKN
%token MINUS_TKN
%token WRITE_TKN
%token READ_TKN
%token INTTOREAL_TKN
%token REALTOINT_TKN
%token LABEL_TKN
%token PUSH_TKN
%token CALL_TKN
%token RETURN_TKN
%token INCSP_TKN
%token JUMP_TKN

%%
program:
	PROGRAM_TKN ID_TKN
	/*{
		int id=$2;
		symbolTable.erase(symbolTable.begin()+id); //w celu usunięcia nazwy programu
	}*/
	'(' start_params ')' ';' //male litery to nieterminale(produkcje), duze to tokeny, w cudzyslowiach są terminale
	declarations
	subprogram_declarations	{
		writeToOut("\nlab0:");

	}
	compound_statement '.'	{
		writeToOut("\n\texit\n");
		saveToFile();
	}
	eof
	;

start_params:
	ID_TKN
	/*{
		//w celu usunięcia input,output z tablicy symboli
			int id=$1;
			symbolTable.erase(symbolTable.begin()+id);
	}*/
	| start_params ',' ID_TKN
	/*{
			int id=$3;
			symbolTable.erase(symbolTable.begin()+id);
	}*/
	;

identifier_list:
	ID_TKN {
		int id = $1; // zczytujemy wartosc spod ID_TKN
		if (id == -1) { 
			YYERROR;
		}
		argumentsTmp.push_back(id);
	}
	| identifier_list ',' ID_TKN	{
		int id = $3;
		if (id == -1) { 
			YYERROR;
		}
		argumentsTmp.push_back(id);
	}
	;

declarations :
	declarations VAR_TKN identifier_list ':' type ';'	{
		int type = $5;
		for (int i = 0; i < argumentsTmp.size(); i++) { 
			int index = argumentsTmp[i];
			if (type == INTEGER_TKN || type == REAL_TKN) { 
				symbolTable[index].type = type;
				symbolTable[index].token = VAR_TKN;
				symbolTable[index].address = generateVarPosition(symbolTable[index].name);//obliczenie adresu
			}
			else if (type == ARRAY_TKN) { 
				symbolTable[index].token = type;
				symbolTable[index].type = arrayType;
				symbolTable[index].array = arrayTmp;
				symbolTable[index].address = generateVarPosition(symbolTable[index].name);
			}
			else { 
				yyerror("Błędny typ");
				YYERROR;
			}
		}
		argumentsTmp.clear();
	}
	|
	;

type :
	standard_type
	| ARRAY_TKN '[' NUM_TKN '.''.' NUM_TKN ']' OF_TKN standard_type	{
		$$ = ARRAY_TKN; // $$ wartosc semantyczna produkcji
		int start = $3;
		int stop = $6;
		arrayType = $9; //przekazanie typu elementów tablicy
		arrayTmp.array_start = start;
		arrayTmp.array_startValue = atoi(symbolTable[start].name.c_str());
		arrayTmp.array_stop = stop;
		arrayTmp.array_stopValue = atoi(symbolTable[stop].name.c_str());
		arrayTmp.array_argType = arrayType;
	}
	;

standard_type :
	INTEGER_TKN
	| REAL_TKN
	;

subprogram_declarations :
	subprogram_declarations subprogram_declaration ';'
	|
	;

subprogram_declaration :
	subprogram_head declarations compound_statement	{
		writeToOut("\n\tleave");
		generateOneArgOperation(RETURN_TKN, -1, true);
		printSymtable();
		clearLocalVars(); //czyszczenie zmiennych lokalnych funkcji
		isGlobal = true; //po wyjściu z funkcji/procedury zmienia zakres na globalny
		paramsOffset = 8;
	}
	;

subprogram_head :
	FUNCTION_TKN ID_TKN 	{
		int id = $2;
		if(id == -1) { 
			YYERROR;
		}
		symbolTable[id].token = FUNCTION_TKN;
		isGlobal = false; //zmiana zakresu z globalnego na lokalny
		generateOneArgOperation(FUNCTION_TKN, id, true); //wypisuje etykietę funkcji
		paramsOffset = 12; //offset dla funkcji 12
	}
	arguments	{
		int id = $2;
		symbolTable[id].parameters = parameters; //przepisuje listę parametrów
		parameters.clear();
	}
	':' standard_type	{
		int type = $7;
		int id = $2;
		symbolTable[id].type = type;
		int returnVar = addToSymbolTable(symbolTable[id].name.c_str(), type, VAR_TKN); //zmienna przechowująca wartość zwracana
		symbolTable[returnVar].reference = true;
		symbolTable[returnVar].address = 8;
	}
	';'
	| PROCEDURE_TKN ID_TKN	{
		int id = $2;
		if (id == -1) { 
			YYERROR;
		}
		symbolTable[id].token = PROCEDURE_TKN;
		isGlobal = false;
		generateOneArgOperation(PROCEDURE_TKN, id, true); //wypisuje etykietę procedury
		paramsOffset = 8; //offset dla procedury 8
	}
	arguments	{
		int id = $2;
		symbolTable[id].parameters = parameters;
		parameters.clear();
	}
	';'
	;

arguments :
	'(' parameter_list ')'	{
		//lista dla parametrów funkcji, nadaje kolejne adresy
		list<int>::iterator it = funParams.begin();
		for(it; it != funParams.end(); it++) { 
			symbolTable[*it].address = paramsOffset;
			paramsOffset+=4;
		}
		funParams.clear();
	}
	|
	;

parameter_list :
	identifier_list ':' type	{
		int type = $3;
		for (int i = 0; i < argumentsTmp.size(); i++) { 
			int index = argumentsTmp[i];
			symbolTable[index].reference = true; //ustawia, że jest referencją
			if (type == ARRAY_TKN) { 
				symbolTable[index].type = arrayType;
				symbolTable[index].token = ARRAY_TKN;
				symbolTable[index].array = arrayTmp;
			}
			else { 
				symbolTable[index].type = type;
			}
			parameters.push_back(make_pair(type, arrayTmp)); //dodawanie do listy argumentów
			funParams.push_front(argumentsTmp[i]);
		}
		argumentsTmp.clear();
	}
	| parameter_list ';' identifier_list ':' type	{
		int type = $5;
		for (int i = 0; i < argumentsTmp.size(); i++) { 
			int index = argumentsTmp[i];
			symbolTable[index].reference = true;
			if (type == ARRAY_TKN) { 
				symbolTable[index].type = arrayType;
				symbolTable[index].token = ARRAY_TKN;
				symbolTable[index].array = arrayTmp;
			}
			else { 
				symbolTable[index].type = type;
			}
			parameters.push_back(make_pair(type, arrayTmp));
			funParams.push_front(argumentsTmp[i]);
		}
		argumentsTmp.clear();
	}
	;

compound_statement :
	BEGIN_TKN
	optional_statements
	END_TKN
	;

optional_statements :
	statement_list
	|
	;

statement_list :
	statement
	| statement_list ';' statement
	;

statement :
	variable ASSIGNOP_TKN expression	{
		int var = $1;
		int expression = $3;
		generateTwoArgsOperation(ASSIGNOP_TKN, expression, true, var, true);
	}
	| procedure_statement
	| compound_statement
	| IF_TKN expression	{
		int lab1 = generateLabel();//tworzy nową etykietę
		int newNum = addNum("0", INTEGER_TKN);
		int expression = $2;

		//skok dla warunku niespełnionego
		generateThreeArgsOperation(EQ_TKN, expression, true, newNum, true, lab1, true);
		$2 = lab1;
	}
		THEN_TKN statement	{
			//etykieta dla statement, jump do statement po else, wypisanie etykiety statement
			int lab2 = generateLabel();
			$5 = lab2;
			generateOneArgOperation(JUMP_TKN, lab2, true); //skacze do statement label($5)
			generateOneArgOperation(LABEL_TKN, $2, true); //etykieta dla $2
		}
		ELSE_TKN statement	{
			generateOneArgOperation(LABEL_TKN, $5, true); //etykieta dla $5
		}
	| WHILE_TKN	{
			int start = generateLabel();
			int stop = generateLabel();
			$1 = start;
			$$ = stop;
			generateOneArgOperation(LABEL_TKN, start, true);
		}
		expression DO_TKN	{
			//jeżeli warunek jest niespełniony,  to skacz do stop
			int num1 = addNum("0", INTEGER_TKN);
			generateThreeArgsOperation(EQ_TKN, $3, true, num1, true, $2, true);
		}
		statement { 
			generateOneArgOperation(JUMP_TKN, $1, true);
			generateOneArgOperation(LABEL_TKN, $2, true);
		}
	;

variable :
	ID_TKN	{
		int tmp = $1;
		if (tmp == -1) { 
			yyerror("Zmienna niezadeklarowana");
			YYERROR;
		}
		$$ = tmp; //zwraca id
	}
	| ID_TKN '[' expression ']'	{
		int index = $3;
		if (symbolTable[index].type == REAL_TKN) { 
			int value = generateTmpVar(INTEGER_TKN);
			generateTwoArgsOperation(REALTOINT_TKN, index, true, value, true);
			index = value;
		}
		int id = $1;
		int start = symbolTable[id].array.array_start;
		int rIndex = generateTmpVar(INTEGER_TKN);
		generateThreeArgsOperation(MINUS_TKN, index, true, start, true, rIndex, true); //odejmuje od indeksu indeks początkowy

		int arrayElemSize = 0;
		if (symbolTable[id].type == REAL_TKN) { 
			arrayElemSize = addNum("8", INTEGER_TKN);
		}
		else if (symbolTable[id].type == INTEGER_TKN) { 
			arrayElemSize = addNum("4", INTEGER_TKN);
		}
		generateThreeArgsOperation(MUL_TKN, rIndex, true, arrayElemSize, true, rIndex, true); //element*pozycja

		int varArrayAddress = generateTmpVar(INTEGER_TKN);
		generateThreeArgsOperation(PLUS_TKN, id, false, rIndex, true, varArrayAddress, true); //adres poczatku tablicy + adres elementu tablicy

		symbolTable[varArrayAddress].type = symbolTable[id].type;
		symbolTable[varArrayAddress].reference = true;
		$$ = varArrayAddress;
	}
	;

procedure_statement :
	ID_TKN	{
		//dla wywołania bez parametrów
		int proc = $1;
		if (proc == -1) { 
			yyerror("Nie znaleniono procedury/funkcji");
			YYERROR;
		}

		if (symbolTable[proc].token == PROCEDURE_TKN) { 
			int paramSize = symbolTable[proc].parameters.size();
			if (paramSize > 0) {
				yyerror("Nieprawidłowa liczba argumentów");
				YYERROR;
			}
			generateOneArgOperation(CALL_TKN, proc, true);
		}
		else { 
			yyerror("Oczekiwano nazwy procedury/funkcji");
			YYERROR;
		}
	}
	| ID_TKN '(' expression_list ')'	{
		//dla wywołania z parametrami
		int ind = $1;
		int write = findSymbolIndexByName("write");
		int read = findSymbolIndexByName("read");
		if (ind == write || ind == read) { 
			for (int i = 0; i < argumentsTmp.size(); i++) { 
				if (ind == read) { 
					generateOneArgOperation(READ_TKN, argumentsTmp[i], true);
				}
				else if (ind == write) { 
					generateOneArgOperation(WRITE_TKN, argumentsTmp[i], true);
				}
			}
		} else {
			string name = symbolTable[ind].name;
			int index = findSymbolIndexIfProcOrFunc(name.c_str());
			if (index == -1) { 
				yyerror("Nazwa niezadeklaowana");
				YYERROR;
			}
			int tmpToken = symbolTable[index].token;
			if (tmpToken == PROCEDURE_TKN) { 
				int paramSize = symbolTable[index].parameters.size();
				if (argumentsTmp.size() < paramSize) { 
					yyerror("Nieprawidłowa liczba argumentów");
					YYERROR;
				}

				int incspCounter = 0; //przechowuje rozmiar referencji wrzuconych na subprogram_declarations

				list<pair<int, arrayStruct>>::iterator it = symbolTable[index].parameters.begin();//iterator po argumentach
				int start = argumentsTmp.size() - symbolTable[index].parameters.size();

				//przechodzi po wszystkich argumentach
				for (int i = start; i < argumentsTmp.size(); i++) { 
					int id = argumentsTmp[i];
					int argType = (*it).first; //typ argumentu procedury/funkcji
					if (argType == ARRAY_TKN) { 
						argType = (*it).second.array_argType;
					}

					//jezli przekazywana jest NUM, to tworzy nowy wpis w tablicy
					int tmpIndex = argumentsTmp[i];
					if (symbolTable[tmpIndex].token == NUM_TKN) { 
						int num = generateTmpVar(argType);
						generateTwoArgsOperation(ASSIGNOP_TKN, tmpIndex, true, num, true);
						id = num;
					}

					//przekazywany typ
					int passType = symbolTable[id].type;

					//gdy typ argumentu i wartości przekzaywanej są różne
					if (argType != passType) { 
						int tmpVar = generateTmpVar(argType);
						generateTwoArgsOperation(ASSIGNOP_TKN, id, true, tmpVar, true);
						id = tmpVar;
					}

					generateOneArgOperation(PUSH_TKN, id, false);
					incspCounter+=4;
					it++;
				}

				//usuwanie argumentów z wektora
				int argsSize = argumentsTmp.size();
				for (int i = start; i < argsSize; i++) { 
					argumentsTmp.pop_back ();
				}

				//call
				generateOneArgOperation(CALL_TKN, index, true);
				stringstream ss;
				ss << incspCounter;

				//inscp
				int incspNum = addNum(ss.str().c_str(), INTEGER_TKN);
				generateOneArgOperation(INCSP_TKN, incspNum, true);
			}
			else { 
				yyerror("Nie znaleziono funkcji/procedury");
				YYERROR;
			}
		}
		argumentsTmp.clear();
	}
	;

expression_list :
	expression	{
		int exp = $1;
		argumentsTmp.push_back(exp);
	}
	| expression_list ',' expression	{
		int exp = $3;
		argumentsTmp.push_back(exp);
	}
	;

expression :
	simple_expression	{
		$$ = $1;
	}
	| simple_expression RELOP_TKN simple_expression	{
		int newLab = generateLabel();
		int relopType = $2;
		int leftSE = $1;
		int rightSE = $3;

		//jeżeli spełniony warunek to skacz
		generateThreeArgsOperation(relopType, leftSE, true, rightSE, true, newLab, true);

		//wynik operacji relop
		int resVar = generateTmpVar(INTEGER_TKN);
		int bVal = addNum("0", INTEGER_TKN);

		//ustawia resVar na 0
		generateTwoArgsOperation(ASSIGNOP_TKN,  bVal, true, resVar, true);

		//ostatni label, potem dalsza część programu
		int finishLabel = generateLabel();
		generateOneArgOperation(JUMP_TKN, finishLabel, true);

		//spełniony warunek
		generateOneArgOperation(LABEL_TKN, newLab, true);
		int gVal = addNum("1", INTEGER_TKN);
		generateTwoArgsOperation(ASSIGNOP_TKN, gVal, true, resVar, true);

		//etykieta za całym wyrażeniem
		generateOneArgOperation(LABEL_TKN, finishLabel, true);
		$$ = resVar;
	}
	;

simple_expression :
	term
	| SIGN_TKN term	{
		int tmpToken = $1;
		int term = $2;
		if (tmpToken == PLUS_TKN) { 
			$$ = term;
		}
		else { 
			//w przypadku liczb ujemnych
			$$ = generateTmpVar(symbolTable[term].type);
			int tmpVar = addNum("0", symbolTable[term].type);
			generateThreeArgsOperation(tmpToken, tmpVar, true, term, true, $$, true); //odejmuje wartość od 0
		}
	}
	| simple_expression SIGN_TKN term	{
		int se = $1;
		int sign = $2;
		int term = $3;
		int resType = getResultType(se, term);
		$$ = generateTmpVar(resType);
		generateThreeArgsOperation(sign, se, true, term, true, $$, true);
	}
	| simple_expression OR_TKN term{
		int se = $1;
		int term = $3;
		int resVar = generateTmpVar(INTEGER_TKN);
		generateThreeArgsOperation(OR_TKN, se, true, term, true, resVar, true);
		$$ = resVar;
	}
	;

term :
	factor
	| term MULOP_TKN factor	{
		int term = $1;
		int mulop = $2;
		int factor = $3;
		int resType = getResultType(term, factor);
		int tmpVar = generateTmpVar(resType);
		generateThreeArgsOperation(mulop, term, true, factor, true, tmpVar, true);
		$$ = tmpVar;
	}
	;

factor :
	variable	{
		int id = $1;
		if (symbolTable[id].token == FUNCTION_TKN) { 
			if (symbolTable[id].parameters.size()>0) { 
				yyerror("Wywołanie funkcji bez odpowiedniej liczby argumentów");
				YYERROR;
			}
			id = generateTmpVar(symbolTable[id].type);//nowa zmienna na wartośc zwracaną przez funkcję
			generateOneArgOperation(PUSH_TKN, id, false);
			writeToOut(string("\n\tcall.i #").c_str());
			writeToOut(symbolTable[$1].name.c_str());

			//inscp po wywołaniu funkcji bez parametrów
			writeToOut(string("\n\tincsp.i #4").c_str());
		}
		else if (symbolTable[id].token == PROCEDURE_TKN) { 
			yyerror("Procedura nie zwraca wyniku");
			YYERROR;
		}
		$$ = id;
	}
	| ID_TKN '(' expression_list ')'	{
		int idT = $1;
		string name = symbolTable[idT].name;
		int index = findSymbolIndexIfProcOrFunc(name.c_str());

		if (index == -1) { 
			yyerror("Nazwa niezadeklarowana");
			YYERROR;
		}

		if (symbolTable[index].token == FUNCTION_TKN) { 
			int symParametersSize = symbolTable[index].parameters.size();
			if (argumentsTmp.size() < symParametersSize) { 
				yyerror("Nieprawidłowa liczba argumentów");
				YYERROR;
			}

			int incspCounter = 0;//zmienna przechowująca rozmiar referencji na stosie

			list<pair<int, arrayStruct>>::iterator it = symbolTable[index].parameters.begin();
			int start = argumentsTmp.size() - symbolTable[index].parameters.size();

			for (int i = start; i < argumentsTmp.size(); i++) { 
				int id = argumentsTmp[i];

				//typ argumentu
				int argType = (*it).first;
				if (argType == ARRAY_TKN) { 
					argType = (*it).second.array_argType;
				}

				int tmpIndex = argumentsTmp[i];
				if (symbolTable[tmpIndex].token == NUM_TKN) { 
					int numVar = generateTmpVar(argType);
					generateTwoArgsOperation(ASSIGNOP_TKN, tmpIndex, true, numVar, true);
					id = numVar;
				}

				int passType = symbolTable[id].type;//typ przekazywany

				//gdy typ argumentu i wartości przekzaywanej są różne
				if (argType != passType) { 
					int tmpVar = generateTmpVar(argType);
					generateTwoArgsOperation(ASSIGNOP_TKN, id, true, tmpVar, true);
					id = tmpVar;
				}
				generateOneArgOperation(PUSH_TKN, id, false);
				incspCounter+=4;
				it++;
			}
			int argsSize = argumentsTmp.size();
			for (int i = start; i < argsSize; i++) { 
				argumentsTmp.pop_back();
			}

			int id = generateTmpVar(symbolTable[index].type);
			generateOneArgOperation(PUSH_TKN, id, false);
			incspCounter+=4;
			$$ = id;

			//call
			generateOneArgOperation(CALL_TKN, index, true);

			stringstream ss;
			ss << incspCounter;

			//incsp
			int inum = addNum(ss.str().c_str(), INTEGER_TKN);
			generateOneArgOperation(INCSP_TKN, inum, true);
		}
		else if (symbolTable[index].token == PROCEDURE_TKN) { 
			yyerror("Procedura nie może zwracać wartości");
			YYERROR;
		} else { 
			yyerror("Nie odnaleziono funkcji/procedury");
			YYERROR;
		}
	}
	| NUM_TKN
	| '(' expression ')'	{
		$$ = $2;
	}
	| NOT_TKN factor	{
		int label = generateLabel();
		int id = addNum("0", INTEGER_TKN);
		int factor = $2;

		generateThreeArgsOperation(EQ_TKN, factor, true, id, true, label, true);//jeq, jeżeli facotr=0 to skacz do 1

		int noResultVar = generateTmpVar(INTEGER_TKN);
		generateTwoArgsOperation(ASSIGNOP_TKN, id, true, noResultVar, true);

		int finishLabel = generateLabel();
		generateOneArgOperation(JUMP_TKN, finishLabel, true); //jump na koniec
		generateOneArgOperation(LABEL_TKN, label, true);

		int numVar = addNum("1", INTEGER_TKN);
		generateTwoArgsOperation(ASSIGNOP_TKN, numVar, true, noResultVar, true);//jezeli factor był 0 to zapisuje 1

		generateOneArgOperation(LABEL_TKN, finishLabel, true);
		$$ = noResultVar;
	}
	;

eof:
	DONE_TKN
%%

void yyerror (const char* text){
	printf("%s w linii: %d\n", text, lineno);
}
