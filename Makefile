all: bonjour.exe aurevoir.exe

bonjour.exe: src/Bonjour.cpp
	g++ src/Bonjour.cpp -o bin/Bonjour.exe -static

aurevoir.exe: src/Aurevoir.cpp
	g++ src/Aurevoir.cpp -o bin/Aurevoir.exe -static

clean:
	rm bin/Bonjour.exe bin/Aurevoir.exe