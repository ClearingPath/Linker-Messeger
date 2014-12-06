#ifndef server_hpp
#define server_hpp

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

//deklarasi type bentukan
typedef struct{
	string username;
	string pass;
}clientID;

typedef struct{
	int client_sock;
	string username;
}clientSock;

//deklarasi fungsi dan prosedur
void createServer();
void producerToken();
void clientReciever();
void clientHandle(int client_sock);
bool checkOnline(int client_sock);
bool checkOnlineUser(string username);
void removeOnlineClient(int client_sock);
void writeExternalFile(string filename,vector<string> message);
vector<string> readExternalFile(string path);
vector<string> readExternalFileAutoCreate(string path);
void clientMessageHandle(int client_sock,string message);
int userSearch(vector<string> userPwd);
void userLogin(int client_sock,vector<string> message);
void userRegister(int client_sock,vector<string> message);
void userLogout(int client_sock,vector<string> message);
int groupSearch(vector<string> group);
void groupCreate(int client_sock,vector<string> message);
void groupLeave(int client_sock,vector<string> message);
void groupJoin(int client_sock,vector<string> message);
void sendMessage(int client_sock,vector<string> message);
void sendMessageToClient(int client_sock,string message);
void serverReplySuccess(int client_sock);
void serverReplyError(int client_sock,string errorCode);
void protocolDissambler(string message,vector<string> & result);
string protocolMaker(vector<string> message);
string corretFilePath();
const string currentDateTime();
void createLog(string str);
vector<string> readGroupExternalFileAutoCreate(string path);
vector<string> readGroupExternal(string path);
vector<string> protocolDissambler2(string message);
string retGroup (vector<string> group);
string logMaker(string message);
#endif
