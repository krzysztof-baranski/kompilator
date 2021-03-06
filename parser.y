%{

#include "global.h"

using namespace std;

vector<int> argumentsTmp; //lista dla zmiennych, którym przypisywany jest typ danych
list<int> functionParameters; //lista do obliczenia wartości incsp
arrayStruct array;
list<pair<int, arrayStruct>> parameters;
int arrayType; //zmienna pomocnicza przechowująca typ danych tablicy
int parametersOffset = 8; //8 dla procedur (oldBP + returnAddr), 12 dla funkcji  (+ returnVal) 
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
	PROGRAM_TKN ID_TKN '(' start_params ')' ';' //male litery to nieterminale(produkcje), duze to tokeny, w cudzyslowiach są terminale
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
	| start_params ',' ID_TKN
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
				symbolTable[index].symbol_type = type;
				symbolTable[index].symbol_token = VAR_TKN;
				symbolTable[index].symbol_address = generateVarPosition(symbolTable[index].symbol_name);//obliczenie adresu
			} else if (type == ARRAY_TKN) { 
				symbolTable[index].symbol_token = type;
				symbolTable[index].symbol_type = arrayType;
				symbolTable[index].array = array;
				symbolTable[index].symbol_address = generateVarPosition(symbolTable[index].symbol_name);
			} else { 
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
		array.array_start = start;
		array.array_startValue = atoi(symbolTable[start].symbol_name.c_str());
		array.array_stop = stop;
		array.array_stopValue = atoi(symbolTable[stop].symbol_name.c_str());
		array.array_argType = arrayType;
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
	subprogram_head 
	declarations 
	compound_statement	{
		writeToOut("\n\tleave");
		handleOneArgOperation(RETURN_TKN, -1, true);
		printSymbolTable();
		clearLocalVariables(); //czyszczenie zmiennych lokalnych funkcji
		isGlobal = true; //po wyjściu z funkcji/procedury zmienia zakres na globalny
		parametersOffset = 8;
	}
	;

subprogram_head :
	FUNCTION_TKN ID_TKN 	{
		int id = $2;
		if(id == -1) { 
			YYERROR;
		}
		symbolTable[id].symbol_token = FUNCTION_TKN;
		isGlobal = false; //zmiana zakresu z globalnego na lokalny
		handleOneArgOperation(FUNCTION_TKN, id, true); //wypisuje etykietę funkcji
		parametersOffset = 12; //offset dla funkcji 12
	}
	arguments	{
		int id = $2;
		symbolTable[id].parameters = parameters; //przepisuje listę parametrów
		parameters.clear();
	}
	':' standard_type	{
		int type = $7;
		int id = $2;
		symbolTable[id].symbol_type = type;
		int returnVar = addToSymbolTable(symbolTable[id].symbol_name.c_str(), type, VAR_TKN); //zmienna przechowująca wartość zwracana
		symbolTable[returnVar].is_reference = true;
		symbolTable[returnVar].symbol_address = 8; // returnAddr
	}
	';'
	| PROCEDURE_TKN ID_TKN	{
		int id = $2;
		if (id == -1) { 
			YYERROR;
		}
		symbolTable[id].symbol_token = PROCEDURE_TKN;
		isGlobal = false;
		handleOneArgOperation(PROCEDURE_TKN, id, true); //wypisuje etykietę procedury
		parametersOffset = 8; //offset dla procedury 8
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
		list<int>::iterator it = functionParameters.begin();
		for(it; it != functionParameters.end(); it++) { 
			symbolTable[*it].symbol_address = parametersOffset;
			parametersOffset += 4;
		}
		functionParameters.clear();
	}
	|
	;

parameter_list :
	identifier_list ':' type	{
		int type = $3;
		for (int i = 0; i < argumentsTmp.size(); i++) { 
			int index = argumentsTmp[i];
			symbolTable[index].is_reference = true; //ustawia, że jest referencją
			if (type == ARRAY_TKN) { 
				symbolTable[index].symbol_type = arrayType;
				symbolTable[index].symbol_token = ARRAY_TKN;
				symbolTable[index].array = array;
			}
			else { 
				symbolTable[index].symbol_type = type;
			}
			parameters.push_back(make_pair(type, array)); //dodawanie do listy argumentów
			functionParameters.push_front(argumentsTmp[i]);
		}
		argumentsTmp.clear();
	}
	| parameter_list ';' identifier_list ':' type	{
		int type = $5;
		for (int i = 0; i < argumentsTmp.size(); i++) { 
			int index = argumentsTmp[i];
			symbolTable[index].is_reference = true;
			if (type == ARRAY_TKN) { 
				symbolTable[index].symbol_type = arrayType;
				symbolTable[index].symbol_token = ARRAY_TKN;
				symbolTable[index].array = array;
			}
			else { 
				symbolTable[index].symbol_type = type;
			}
			parameters.push_back(make_pair(type, array));
			functionParameters.push_front(argumentsTmp[i]);
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
		handleTwoArgsOperation(ASSIGNOP_TKN, expression, true, var, true);
	}
	| procedure_statement
	| compound_statement
	| IF_TKN expression	{
		int lab1 = generateLabel();//tworzy nową etykietę
		int newNum = addNum("0", INTEGER_TKN);
		int expression = $2;

		//skok dla warunku niespełnionego
		handleThreeArgsOperation(EQ_TKN, expression, true, newNum, true, lab1, true);
		$2 = lab1;
	}
	THEN_TKN statement	{
		//etykieta dla statement, jump do statement po else, wypisanie etykiety statement
		int lab2 = generateLabel();
		$5 = lab2;
		handleOneArgOperation(JUMP_TKN, lab2, true); //skacze do statement label($5)
		handleOneArgOperation(LABEL_TKN, $2, true); //etykieta dla $2
	}
	ELSE_TKN statement	{
		handleOneArgOperation(LABEL_TKN, $5, true); //etykieta dla $5
	}
	| WHILE_TKN	{
			int start = generateLabel();
			int stop = generateLabel();
			$1 = start;
			$$ = stop;
			handleOneArgOperation(LABEL_TKN, start, true);
		}
		expression DO_TKN	{
			//jeżeli warunek jest niespełniony,  to skacz do stop
			int num1 = addNum("0", INTEGER_TKN);
			handleThreeArgsOperation(EQ_TKN, $3, true, num1, true, $2, true);
		}
		statement { 
			handleOneArgOperation(JUMP_TKN, $1, true);
			handleOneArgOperation(LABEL_TKN, $2, true);
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
		if (symbolTable[index].symbol_type == REAL_TKN) {
			int value = generateTmpVariable(INTEGER_TKN);
			handleTwoArgsOperation(REALTOINT_TKN, index, true, value, true);
			index = value;
		}
		int id = $1;
		int start = symbolTable[id].array.array_start;
		int tmpVarIndex = generateTmpVariable(INTEGER_TKN);
		handleThreeArgsOperation(MINUS_TKN, index, true, start, true, tmpVarIndex, true); //odejmuje od indeksu indeks początkowy

		int elementSize = 0;
		if (symbolTable[id].symbol_type == REAL_TKN) {
			elementSize = addNum("8", INTEGER_TKN);
		} else if (symbolTable[id].symbol_type == INTEGER_TKN) {
			elementSize = addNum("4", INTEGER_TKN);
		}
		handleThreeArgsOperation(MUL_TKN, tmpVarIndex, true, elementSize, true, tmpVarIndex, true); //element*pozycja

		int varArrayAddress = generateTmpVariable(INTEGER_TKN);
		handleThreeArgsOperation(PLUS_TKN, id, false, tmpVarIndex, true, varArrayAddress, true); //adres poczatku tablicy + adres elementu tablicy

		symbolTable[varArrayAddress].symbol_type = symbolTable[id].symbol_type;
		symbolTable[varArrayAddress].is_reference = true;
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

		if (symbolTable[proc].symbol_token == PROCEDURE_TKN) {
			int paramSize = symbolTable[proc].parameters.size();
			if (paramSize > 0) {
				yyerror("Nieprawidłowa liczba argumentów");
				YYERROR;
			}
			handleOneArgOperation(CALL_TKN, proc, true);
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
		if (ind == write) { 
			for (int i = 0; i < argumentsTmp.size(); i++) { 
				handleOneArgOperation (WRITE_TKN, argumentsTmp[i], true);
			}
		} else if (ind == read) {
			for (int i = 0; i < argumentsTmp.size(); i++) { 
				handleOneArgOperation(READ_TKN, argumentsTmp[i], true);
			}
		} else {
			string name = symbolTable[ind].symbol_name;
			int index = findSymbolIndexIfProcOrFunc(name.c_str());
			if (index == -1) { 
				yyerror("Nazwa niezadeklaowana");
				YYERROR;
			}
			int tmpToken = symbolTable[index].symbol_token;
			if (tmpToken != PROCEDURE_TKN) { 
				yyerror("Nie znaleziono funkcji/procedury");
				YYERROR;
			} else {
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
					if (symbolTable[tmpIndex].symbol_token == NUM_TKN) {
						int num = generateTmpVariable(argType);
						handleTwoArgsOperation(ASSIGNOP_TKN, tmpIndex, true, num, true);
						id = num;
					}

					//przekazywany typ
					int passedType = symbolTable[id].symbol_type;

					//gdy typ argumentu i wartości przekzaywanej są różne
					if (argType != passedType) { 
						int tmpVar = generateTmpVariable(argType);
						handleTwoArgsOperation(ASSIGNOP_TKN, id, true, tmpVar, true);
						id = tmpVar;
					}

					handleOneArgOperation(PUSH_TKN, id, false);
					incspCounter += 4;
					it++;
				}

				//usuwanie argumentów z wektora
				int argsSize = argumentsTmp.size();
				for (int i = start; i < argsSize; i++) { 
					argumentsTmp.pop_back();
				}

				//call
				handleOneArgOperation(CALL_TKN, index, true);
				stringstream ss;
				ss << incspCounter;

				//incsp
				int incspNum = addNum(ss.str().c_str(), INTEGER_TKN);
				handleOneArgOperation(INCSP_TKN, incspNum, true);
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
		handleThreeArgsOperation(relopType, leftSE, true, rightSE, true, newLab, true);

		//wynik operacji relop
		int resVar = generateTmpVariable(INTEGER_TKN);
		int bVal = addNum("0", INTEGER_TKN);

		//ustawia resVar na 0
		handleTwoArgsOperation(ASSIGNOP_TKN,  bVal, true, resVar, true);

		//ostatni label, potem dalsza część programu
		int finishLabel = generateLabel();
		handleOneArgOperation(JUMP_TKN, finishLabel, true);

		//spełniony warunek
		handleOneArgOperation(LABEL_TKN, newLab, true);
		bVal = addNum("1", INTEGER_TKN);
		handleTwoArgsOperation(ASSIGNOP_TKN, bVal, true, resVar, true);

		//etykieta za całym wyrażeniem
		handleOneArgOperation(LABEL_TKN, finishLabel, true);
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
			$$ = generateTmpVariable(symbolTable[term].symbol_type);
			int tmpVar = addNum("0", symbolTable[term].symbol_type);
			handleThreeArgsOperation(tmpToken, tmpVar, true, term, true, $$, true); //odejmuje wartość od 0
		}
	}
	| simple_expression SIGN_TKN term	{
		int se = $1;
		int sign = $2;
		int term = $3;
		int resType = getResultType(se, term);
		$$ = generateTmpVariable(resType);
		handleThreeArgsOperation(sign, se, true, term, true, $$, true);
	}
	| simple_expression OR_TKN term {
		int se = $1;
		int term = $3;
		int resVar = generateTmpVariable(INTEGER_TKN);
		handleThreeArgsOperation(OR_TKN, se, true, term, true, resVar, true);
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
		int tmpVar = generateTmpVariable(resType);
		handleThreeArgsOperation(mulop, term, true, factor, true, tmpVar, true);
		$$ = tmpVar;
	}
	;

factor :
	variable {
		int id = $1;
		if (symbolTable[id].symbol_token == FUNCTION_TKN) {
			if (symbolTable[id].parameters.size() > 0) { 
				yyerror("Wywołanie funkcji bez odpowiedniej liczby argumentów");
				YYERROR;
			}
			id = generateTmpVariable(symbolTable[id].symbol_type);//nowa zmienna na wartośc zwracaną przez funkcję
			handleOneArgOperation(PUSH_TKN, id, false);
			writeToOut(string("\n\tcall.i #").c_str());
			writeToOut(symbolTable[$1].symbol_name.c_str());

			//inscp po wywołaniu funkcji bez parametrów
			writeToOut(string("\n\tincsp.i #4").c_str());
		} else if (symbolTable[id].symbol_token == PROCEDURE_TKN) {
			yyerror("Procedura nie zwraca wyniku");
			YYERROR;
		}
		$$ = id;
	}
	| ID_TKN '(' expression_list ')'	{
		int idT = $1;
		string name = symbolTable[idT].symbol_name;
		int index = findSymbolIndexIfProcOrFunc(name.c_str());

		if (index == -1) { 
			yyerror("Nazwa niezadeklarowana");
			YYERROR;
		}

		if (symbolTable[index].symbol_token == FUNCTION_TKN) {
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
				if (symbolTable[tmpIndex].symbol_token == NUM_TKN) {
					int numVar = generateTmpVariable(argType);
					handleTwoArgsOperation(ASSIGNOP_TKN, tmpIndex, true, numVar, true);
					id = numVar;
				}

				int passedType = symbolTable[id].symbol_type;//typ przekazywany

				//gdy typ argumentu i wartości przekzaywanej są różne
				if (argType != passedType) { 
					int tmpVar = generateTmpVariable(argType);
					handleTwoArgsOperation(ASSIGNOP_TKN, id, true, tmpVar, true);
					id = tmpVar;
				}
				handleOneArgOperation(PUSH_TKN, id, false);
				incspCounter += 4;
				it++;
			}
			int argsSize = argumentsTmp.size();
			for (int i = start; i < argsSize; i++) { 
				argumentsTmp.pop_back();
			}

			int id = generateTmpVariable(symbolTable[index].symbol_type);
			handleOneArgOperation(PUSH_TKN, id, false);
			incspCounter += 4;
			$$ = id;

			//call
			handleOneArgOperation(CALL_TKN, index, true);

			stringstream ss;
			ss << incspCounter;

			//incsp
			int inum = addNum(ss.str().c_str(), INTEGER_TKN);
			handleOneArgOperation(INCSP_TKN, inum, true);
		} else if (symbolTable[index].symbol_token == PROCEDURE_TKN) {
			yyerror("Procedura nie może zwracać wartości");
			YYERROR;
		} else { 
			yyerror("Nie odnaleziono funkcji/procedury");
			YYERROR;
		}
	}
	| NUM_TKN
	| '(' expression ')' {
		$$ = $2;
	}
	| NOT_TKN factor {
		int label = generateLabel();
		int id = addNum("0", INTEGER_TKN);
		int factor = $2;

		handleThreeArgsOperation(EQ_TKN, factor, true, id, true, label, true);//jeq, jeżeli factor==0 to skacz do 1

		int notResultVar = generateTmpVariable(INTEGER_TKN);
		handleTwoArgsOperation(ASSIGNOP_TKN, id, true, notResultVar, true);

		int finishLabel = generateLabel();
		handleOneArgOperation(JUMP_TKN, finishLabel, true); //jump na koniec
		handleOneArgOperation(LABEL_TKN, label, true);

		int numVar = addNum("1", INTEGER_TKN);
		handleTwoArgsOperation(ASSIGNOP_TKN, numVar, true, notResultVar, true);//jezeli factor był 0 to zapisuje 1

		handleOneArgOperation(LABEL_TKN, finishLabel, true);
		$$ = notResultVar;
	}
	;

eof:
	DONE_TKN
%%

void yyerror (const char* text){
	printf("%s w linii: %d\n", text, lineno);
}
