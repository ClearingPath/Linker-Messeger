all:
	g++ -std=c++11 -o bin/server/server src/server.cpp -pthread -Wl,--no-as-needed
	g++ -std=c++11 -o bin/client/client src/client.cpp -pthread -Wl,--no-as-needed
	
runserver:
	./bin/server/server
	
runclient:
	./bin/client/client localhost
