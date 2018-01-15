objects = main.o emiter.o symbol.o lexer.o parser.o

comp : $(objects)
	g++ -o comp $(objects)

$(objects) : global.h parser.h

main.o : main.cpp
	g++ -c -std=c++11 main.cpp

emiter.o : emiter.cpp
	g++ -c -std=c++11 emiter.cpp

symbol.o : symbol.cpp
	g++ -c -std=c++11 symbol.cpp

lexer.o : lexer.cpp
	g++ -c -std=c++11 lexer.cpp

lexer.cpp: lexer.l
	flex -o lexer.cpp lexer.l

parser.o : parser.cpp
	g++ -c -std=c++11 parser.cpp

parser.cpp parser.h : parser.y
	bison --defines=parser.h -o parser.cpp parser.y

clean :
	rm comp $(objects)
	rm lexer.cpp
	rm parser.cpp
	rm parser.h
	rm output.asm
