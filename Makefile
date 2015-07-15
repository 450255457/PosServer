all:PosClient test_server
PosClient:PosClient.cpp
	g++ -g PosClient.cpp -o PosClient
test_server:test_server.cpp
	g++ -g test_server.cpp -o test_server
.PHONY : clean
clean : 
	-rm -f PosClient test_server