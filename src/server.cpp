#include "server.hpp"

//global variable
socklen_t clilen;
struct sockaddr_in serv_addr, cli_addr;
int sock, len,client_sock;
char buffer[buffer_size];
bool active;
mutex kunci;
int token; //jumlah maksimal
vector<thread> pool; //thread pool; resv : 0 -> producer 1 -> client reciever
vector<clientSock> onlineClient; //vector of online clients

int main(){
	cout << "WELCOME!" << endl;
	
	createServer();	

	active = true;

	string logStart = "SERVER_START";
	createLog(logStart);

	pool.push_back(thread(producerToken));

	pool.push_back(thread(clientReciever));

	pool[0].join();

	pool[1].join();

	close(sock);

}

void createServer(){
	// buka socket TCP (SOCK_STREAM) dengan alamat IPv4 dan protocol IP
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0){
		close(sock);
		printf("Cannot open socket\n");
		exit(1);
	}
	
	// ganti opsi socket agar dapat di-reuse addressnya
	int yes = 1;
	if (	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
		perror("Cannot set option\n");
		exit(1);
	}
	
	// buat address untuk server
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY; // semua address pada network interface
	serv_addr.sin_port = htons(port); // port 9000
	
	// bind server ke address
	if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		close(sock);
		printf("Cannot bind socket\n");
		exit(1);
	}
	
	
	listen(sock,5); // listen ke maksimal 5 koneksi klien
	}

void producerToken(){
	while (active){
		kunci.lock();
		if (token < 5){
			token++;
		}
		kunci.unlock();
		usleep(100 * 1000); // tidur selama 1000 ms
	}
}

void clientReciever(){
	while (active){
		// terima koneksi dari klien
		clilen = sizeof(cli_addr);
		client_sock = accept(sock, (struct sockaddr *) &cli_addr, &clilen);
		printf ("Client %d Connected!\n",client_sock);
		pool.push_back(thread(clientHandle,client_sock));
		token--;
		usleep(50 * 1000);
	}
}

void clientHandle(int client_sock){
	clientSock client;
	client.client_sock = client_sock;
	onlineClient.push_back(client);
	bool online = true;
	while (online && active){
		bzero(buffer,buffer_size);
		len = read(client_sock,buffer,buffer_size);
		if (len >= 0){
			kunci.lock();
			printf("Client %d = %s\n",client_sock,buffer);
			if (strcmp(buffer,"") == 0){
				cout << "Client offline : Disconnected from server !" << endl;
				removeOnlineClient(client_sock);
				online = false;
			}
			else{
				string temp = buffer;
				clientMessageHandle(client_sock,temp);
				createLog(temp);
			}
			kunci.unlock();
		}
		usleep(50*1000); //sleep for 50 ms
	}
	close(client_sock);
}

void createLog(string str){
	string ret;
	string path = "bin/server/file/log.txt";
	ofstream myfile(path,ios::app);
	//ganti string nya -- log maker
	ret = logMaker(str);
	myfile << ret;
	myfile << endl;
	myfile.close();
}

string logMaker(string message){
	vector<string> choppedString;
	string ret;
	ret="[";
	ret += currentDateTime();
	ret +="] ";
	protocolDissambler(message,choppedString);
	if(choppedString[0].compare("SERVER_START")==0){
		ret += "server start";
	}
	else if (choppedString[0].compare("LIN") == 0){
		ret += choppedString[1];
		ret += " has logged in";
	}
	else if (choppedString[0].compare("REG") == 0){
		ret += choppedString[1];
		ret += " has registered";
	}
	else if (choppedString[0].compare("LOU") == 0){
		ret += choppedString[1];
		ret += " has logged out";
	}
	else if (choppedString[0].compare("JGR") == 0){
		ret += choppedString[2];
		ret += " has join ";
		ret += choppedString[1];
	}
	else if (choppedString[0].compare("CGR") == 0){
		ret += choppedString[1];
		ret += " created";
	}
	else if (choppedString[0].compare("LGR") == 0){
		ret += choppedString[2];
		ret += " left ";
		ret += choppedString[1];
	}
	else if (choppedString[0].compare("MSG") == 0){
		ret += choppedString[1];
		ret += " messaged ";
		ret += choppedString[2];
	}	
	else if(choppedString[0].compare("SVR")==0){
		ret += choppedString[1];
		ret += " : ";		
		ret += choppedString[2];
		
	}

	return ret;
}
bool checkOnline(int client_sock){
	bool found = false;
	int i = 0;
	while ((i < onlineClient.size()) && !found){
		if (client_sock == onlineClient[i].client_sock){
			found = true;
		}
		else
			i++;
	}
	return found;
}

bool checkOnlineUser(string username){
	bool found = false;
	int i = 0;
	while ((i < onlineClient.size()) && !found){
		if (username.compare(onlineClient[i].username) == 0)
			found = true;
		else
			i++;
	}
	return found;
}

void removeOnlineClient (int client_sock){
	bool found = false;
	int i =0;
	while ((i < onlineClient.size()) && !found){
		if (client_sock == onlineClient[i].client_sock){
			found = true;
		}
		else{
			i++;
		}
	}
	if (i != onlineClient.size()){
		onlineClient.erase(onlineClient.begin()+i);
	}
}

void clientMessageHandle(int client_sock,string message){
	vector<string> choppedString;
	protocolDissambler(message,choppedString);
	if (choppedString[0].compare("LIN") == 0){
		userLogin(client_sock,choppedString);
	}
	else if (choppedString[0].compare("REG") == 0){
		userRegister(client_sock,choppedString);
	}
	else if (choppedString[0].compare("LOU") == 0){
		userLogout(client_sock,choppedString);
	}
	else if (choppedString[0].compare("JGR") == 0){
		groupJoin(client_sock,choppedString);
	}
	else if (choppedString[0].compare("CGR") == 0){
		groupCreate(client_sock,choppedString);
	}
	else if (choppedString[0].compare("LGR") == 0){
		groupLeave(client_sock,choppedString);
	}
	else if (choppedString[0].compare("MSG") == 0){
		sendMessage(client_sock,choppedString);
	}
}

void sendMessage(int client_sock,vector<string> message){
	vector<string> temp = message;
	vector<string> userPwd;
	vector<string> groupUser;
	groupUser.push_back(temp[2]);
	groupUser.push_back(temp[1]);
	userPwd.push_back(temp[2]);
	userPwd.push_back("");
	int i = 0;
	bool found;
	if (userSearch(userPwd) == 0){
		while ((i < onlineClient.size()) && !found){
			if (onlineClient[i].username.compare(temp[2]) == 0){
				found = true;
				sendMessageToClient(onlineClient[i].client_sock,protocolMaker(message));
			}
			else
				i++;
		}
		if (!found){
			//user offline
			string path = "bin/server/file/pendingMessage.txt";
			vector<string> data = readExternalFileAutoCreate(path);
			data.push_back(protocolMaker(message));
			writeExternalFile(path,data);
		}
		serverReplySuccess(client_sock);
	}
	else if(groupSearch(groupUser) == 1){
		//masukin ke file yang grup, nama grup nya aja
		//atau dibikin satu2 kaya chat ber2 biasa
		string groupLine = retGroup(groupUser);
		vector<string> groupVector = protocolDissambler2(groupLine);
		vector<string> kirimPesan;
		for (int j = 1;j < groupVector.size();j++){
			if (groupVector[j].compare(groupUser[1]) != 0){
				kirimPesan.push_back("GRP");
				kirimPesan.push_back(groupUser[0]);
				kirimPesan.push_back(groupVector[j]);
				kirimPesan.push_back(groupUser[1]);
				kirimPesan.push_back(temp[3]);
				kirimPesan.push_back(temp[4]);
				i = 0;
				found = false;
				while ((i < onlineClient.size()) && !found){
					if (onlineClient[i].username.compare(groupVector[j]) == 0){
						found = true;
						sendMessageToClient(onlineClient[i].client_sock,protocolMaker(kirimPesan));
					}
					else
						i++;
				}
				if (!found){
					//user offline
					string path = "bin/server/file/pendingMessage.txt";
					vector<string> data = readExternalFileAutoCreate(path);
					data.push_back(protocolMaker(kirimPesan));
					writeExternalFile(path,data);
				}	
				kirimPesan.clear();
			}
		}
		serverReplySuccess(client_sock);
	}	
	else
		serverReplyError(client_sock,"004");
}

void sendMessageToClient(int client_sock,string message){
	bzero(buffer,buffer_size);
	strcpy(buffer,message.c_str());
	len = write(client_sock,buffer,buffer_size);
}

vector<string> readExternalFile(string path){
	string line;
	vector<string> ret;
	ifstream myfile(path.c_str());
	if (myfile.is_open()){
		while (getline(myfile,line)){
			ret.push_back(line);
		}
		myfile.close();
	}
	else{
		cout << "Error ! cannot open file" << endl;
	}
	return ret;
}

vector<string> readExternalFileAutoCreate(string path){
	string line;
	vector<string> ret;
	ifstream myfile(path.c_str());
	if (myfile.is_open()){
		while (getline(myfile,line)){
			ret.push_back(line);
		}
		myfile.close();
	}
	else{
		int find = path.find_last_of("/\\");
		string fileHeader = "@";
		fileHeader += path.substr(find);
		vector<string> temp;
		temp.push_back(fileHeader);
		writeExternalFile(path,temp);
		ret = readExternalFile(path);
		cout << "Error ! cannot open file" << endl;
	}
	return ret;
}
vector<string> readGroupExternalFileAutoCreate(string path){
	string line;
	vector<string> ret;
	ifstream myfile(path.c_str());
	if (myfile.is_open()){
		while (getline(myfile,line)){
			ret.push_back(line);
		}
		myfile.close();
	}
	else{
		cout << "masuk sini" <<endl;
		int find = path.find_last_of("/\\");
		string fileHeader = "@";
		fileHeader += path.substr(find);
		vector<string> temp;
		temp.push_back(fileHeader);
		writeExternalFile(path,temp);
		ret = readExternalFile(path);
		cout << "Error ! cannot open file" << endl;
	}
	return ret;
}

void writeExternalFile(string filename,vector<string> message){
	ofstream myfile;
	myfile.open(filename.c_str());
	for (int i = 0;i < message.size();i++){
		myfile << message[i];
		myfile << endl;
	}
	myfile.close();
}
void writeGroupExternalFile(string filename,vector<string> message){
	ofstream myfile;
	myfile.open(filename.c_str());
	for (int i = 0;i < message.size();i++){
		myfile << message[i];
		myfile << endl;
	}
	myfile.close();
}

string correctFilePath(){
	char cCurrentPath[FILENAME_MAX];
	getcwd(cCurrentPath, sizeof(cCurrentPath));
	string str = cCurrentPath;
	cout << str << endl;
	string ret;
	//unsigned found = str.find_last_of("/\\");
	//ret += str.substr(0,found);
	ret += str;
	ret += "/bin/server";
	return ret;
}

int userSearch(vector<string> userPwd){
	int found = -1;
	//string path = correctFilePath();
	//path += "/file/user.txt";
	//cout << path << endl;
	string path = "bin/server/file/user.txt";
	vector<string> data = readExternalFile(path);
	int i = 1;
	if (data.size() > 0){
		//data is not empty
		while (i < data.size() && (found == -1) ){
			vector<string> temp;
			protocolDissambler(data[i],temp);
			if (userPwd[0].compare(temp[0]) == 0){
				found = 0;
				if (userPwd[1].compare(temp[1]) == 0){
					found = 1;
				}
			}
			else{
				i++;
			}
		}
	}
	return found;	
}

void userRegister(int client_sock,vector<string> message){
	vector<string> temp = message;
	temp.erase(temp.begin());
	if (userSearch(temp) == -1){
		//string path = correctFilePath();
		//path += "/file/user.txt";
		string path = "bin/server/file/user.txt";
		vector<string> file = readExternalFile(path);
		file.push_back(protocolMaker(temp));
		writeExternalFile(path,file);
		serverReplySuccess(client_sock);
	}
	else
		serverReplyError(client_sock,"002");
}

void userLogin(int client_sock,vector<string> message){
	vector<string> temp = message;
	temp.erase(temp.begin());
	if ((userSearch(temp) == 1) && (!checkOnlineUser(temp[0]))){
		int i = 0;
		bool found = false;
		while ((i < onlineClient.size()) && !found){
			if (onlineClient[i].client_sock == client_sock){
				onlineClient[i].username = temp[0];
				found = true;
			}
			else
				i++;
		}
		vector<string> data = readExternalFile("bin/server/file/pendingMessage.txt");
		i = 1;
		vector<string> msg;
		while (i < data.size()){
			msg = protocolDissambler2(data[i]);
			if (temp[0].compare(msg[2])==0){
				sendMessageToClient(client_sock,data[i]);
				data.erase(data.begin()+i);
			}
			else{
				i++;
			}
		}
		writeExternalFile("bin/server/file/pendingMessage.txt",data);
		serverReplySuccess(client_sock);
	}
	else
		serverReplyError(client_sock,"001");
}

void userLogout(int client_sock,vector<string> message){
	bool found = false;
	int i = 0;
	while ((i < onlineClient.size()) && !found){
		if (onlineClient[i].username.compare(message[1]) == 0){
			found = true;
			onlineClient[i].username = "";
			serverReplySuccess(client_sock);
		}
		else
			i++;
	}
	if (i == onlineClient.size()){
		serverReplyError(client_sock,"001");
	}
}

int groupSearch(vector<string> group){
	int found = -1;
	string temp = "bin/server/file/group.txt";
	vector<string> data = readExternalFileAutoCreate(temp);
	int i = 1;
	if (data.size() > 0){
		while ((i < data.size()) && (found == -1)){
			vector<string> temp;
			protocolDissambler(data[i],temp);
			if (temp[0].compare(group[0]) == 0){
				found = 0;
				int j = 1;
				while ((j < temp.size()) && (found == 0)){
					if (group[1].compare(temp[j]) == 0){
						found = 1;
					}
					else
						j++;
				}
			}
			else
				i++;

		}
	}
	return found;
}

string retGroup(vector<string> group){
	int found = -1;
	string ret;
	string temp = "bin/server/file/group.txt";
	vector<string> data = readExternalFileAutoCreate(temp);
	int i = 1;
	if (data.size() > 0){
		while ((i < data.size()) && (found == -1)){
			vector<string> temp;
			protocolDissambler(data[i],temp);
			if (temp[0].compare(group[0]) == 0){
				found = 0;
				ret=data[i];
			}
			else
				i++;
		}
	}
	return ret;
}

void groupCreate(int client_sock,vector<string> message){
	vector<string> temp = message;
	temp.erase(temp.begin());
	if (groupSearch(temp) == -1){
		string path = "bin/server/file/group.txt";
		vector<string> data = readExternalFileAutoCreate(path);
		string ret = protocolMaker(temp);
		data.push_back(ret);
		writeExternalFile(path,data);
		serverReplySuccess(client_sock);
	}
	else
	{
		serverReplyError(client_sock,"003");
	}
}

void groupJoin(int client_sock,vector<string> message){
	vector<string> temp = message;
	temp.erase(temp.begin());
	if (groupSearch(temp) == 0){
		string path = "bin/server/file/group.txt";
		vector<string> data = readExternalFile(path);
		int i = 1;
		bool found = false;
		vector<string> groupData;
		while ((i<data.size()) && !found){
			protocolDissambler(data[i],groupData);
			if (temp[0].compare(groupData[0]) == 0){
				found = true;
				groupData.push_back(temp[1]);
				data.erase(data.begin()+i);
				data.push_back(protocolMaker(groupData));
			}
			else{
				i++;
			}
			groupData.clear();
		}
		writeExternalFile(path,data);
		serverReplySuccess(client_sock);
	}
	else
		serverReplyError(client_sock,"003");
}

void groupLeave(int client_sock,vector<string> message){
	vector<string> temp = message;
	temp.erase(temp.begin());
	if (groupSearch(temp) == 1){
		vector<string> notif;
		notif.push_back("MSG");
		notif.push_back(temp[1]);
		notif.push_back(temp[0]);
		notif.push_back(currentDateTime());
		notif.push_back("Has leave the group");
		sendMessage(client_sock,notif);
		string path = "bin/server/file/group.txt";
		vector<string> data = readExternalFile(path);
		int i = 1;
		int j = 1;
		bool found = false;
		vector<string> groupData;
		while ((i<data.size()) && !found){
			j = 1;
			protocolDissambler(data[i],groupData);
			if (temp[0].compare(groupData[0]) == 0){
				while((j < groupData.size()) && !found){
					if (temp[1].compare(groupData[j])==0){
						found = true;
						groupData.erase(groupData.begin()+j);
						if (groupData.size() == 1){
							data.erase(data.begin()+i);
						}
						else{
							data.erase(data.begin()+i);
							data.push_back(protocolMaker(groupData));
						}
					}
					else
						j++;
				}
			}
			else{
				i++;
			}
			groupData.clear();
		}
		writeExternalFile(path,data);
		serverReplySuccess(client_sock);
	}
	else
		serverReplyError(client_sock,"003");
}

void serverReplySuccess(int client_sock){
	string ret;
	vector<string> message;
	message.push_back("SVR");
	message.push_back("200");
	ret = protocolMaker(message);
	bzero(buffer,buffer_size);
	strcpy(buffer,ret.c_str());
	len = write(client_sock,buffer,buffer_size);
}

void serverReplyError(int client_sock,string errorCode){
	string ret;
	vector<string> message;
	message.push_back("SVR");
	message.push_back(errorCode);
	//--------------
	message.push_back(" : ");
	if(errorCode.compare("000")==0){
		message.push_back("Protocol Error!");
	}
	else if(errorCode.compare("001")==0){
		message.push_back("Login Error!");
	}
	else if(errorCode.compare("002")==0){
		message.push_back("Registering Error!");
	}
	else if(errorCode.compare("003")==0){
		message.push_back("Group Error!");
	}
	else if(errorCode.compare("004")==0){
		message.push_back("Message Error!");
	}
	else if(errorCode.compare("200")==0){
		message.push_back("OK");
	}
	else {
		message.push_back("-");
	}
	//--------------
	//message.push_back("Protocol Error!");
	ret = protocolMaker(message);
	cout << ret << endl;
	bzero(buffer,buffer_size);
	strcpy(buffer,ret.c_str());
	len = write(client_sock,buffer,buffer_size);	
	createLog(ret);
}

void protocolDissambler(string message,vector<string> & result){
	char temp[1000];
	char * cstr;
	cstr = new char [100];
	string stringTemp;
	strcpy(temp,message.c_str());
	cstr = strtok(temp,"||");
	while (cstr != 0){
		stringTemp = cstr;
		result.push_back(stringTemp);
		cstr = strtok(NULL,"||");
	}
}
vector<string> protocolDissambler2(string message){
	vector<string> ret;	
	char temp[1000];
	strcpy(temp, message.c_str());
	char * cstr;
	cstr = strtok(temp, "||");
	while (cstr != 0){
		ret.push_back(cstr);
		cstr = strtok(NULL, "||");
	}
	return ret;
}

string protocolMaker(vector<string> message){
	string ret;
	int i = 0;
	while (i < message.size()){
		ret += message[i];
		ret += "||";
		i++;
	}
	return ret;
}

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);

    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}
