all: client server

client: client.o socket.o
	g++ -o client client.o socket.o
client.o: client.cpp 
	g++ -c -o client.o client.cpp
socket.o: socket.cpp
	g++ -c -o socket.o socket.cpp
server: class_table.o server.o socket.o
	g++ -o server class_table.o server.o socket.o
socket.o: socket.cpp
	g++ -c -o socket.o socket.cpp
class_table.o: socket.cpp
	g++ -c -o class_table.o class_table.cpp
server.o: server.cpp
	g++ -c -o server.o server.cpp
