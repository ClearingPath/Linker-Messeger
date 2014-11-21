#ifndef client_hpp
#define client_hpp

#include <sys/types.h>   // tipe data penting untuk sys/socket.h dan netinet/in.h
#include <netinet/in.h>  // fungsi dan struct internet address
#include <sys/socket.h>  // fungsi dan struct socket API
#include <netdb.h>       // lookup domain/DNS hostname
#include <errno.h>
#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
// thread & mutex
#include <thread>
#include <mutex>

//vector
#include <vector>

// untuk printf
#include <cstdio>

// untuk random, usleep, dan time
#include <cstdlib>
#include <unistd.h>
#include <time.h>

#define port 9000
#define buffer_size 1024

using namespace std;
//deklarasi fungsi dan prosedur
void connectToServer(char * host);
void closeConnection();
void header();
void start();
void signup();
void login();
void recieveMessage();
string reFormatMessage(vector<string> message,int read);
int sendToServer(string message);
void protocolDissambler(string message,vector<string> & result);
string protocolMaker(vector<string> message);
const string currentDateTime();
void mainChat();
void logout();
void groupJoin(string str);
void groupCreate(string str);
void groupLeave(string str);
void sendMessage(string str);
void readChat(string str);
const std::string currentDateTime();
void writeExternalFile(string filename,vector<string> message);
vector<string> readExternalFileAutoCreate(string path);
#endif