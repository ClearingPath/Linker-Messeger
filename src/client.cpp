#include "client.hpp"

//global variable
int sock, len;
char buffer[buffer_size];
bool active;
mutex kunci;
string username;
vector<int> replyFromserver;
vector<string> newMessageSender;

int main(int argc, char** argv){
	if (argc != 2){
		printf("Pemakaian: ./client <server ip>\n");
	}
	else{
		connectToServer(argv[1]); // membuka koneksi ke server, jika gagal keluar dengan kode 1

		active = true;

		header();
		
		thread menu(start);

		thread msgrec(recieveMessage);

		menu.join();

		msgrec.join();

	}
	return 0;
}

//implementasi fungsi dan prosedur
void connectToServer(char * host){
	struct sockaddr_in serv_addr;
	struct hostent *server;

	// buka socket TCP (SOCK_STREAM) dengan alamat IPv4 dan protocol IP
	if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0){
		close(sock);
		printf("Cannot open socket\n");
		exit(1);
	}
	
	// buat address server
	server = gethostbyname(host);
	if (server == NULL) {
		fprintf(stderr,"Host not found\n");
		exit(1);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(port);
	
	// connect ke server, jika error keluar
	if (connect(sock,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) exit(1);
}

void closeConnection(){
	close(sock);
}

void recieveMessage(){
	while(active){
		len = read(sock,buffer,buffer_size);
		if (len>=0){
			string stringBuffer = buffer;
			if (stringBuffer.compare("") != 0){
				vector<string> choppedString;
				protocolDissambler(stringBuffer,choppedString);
				if (choppedString[0].compare("SVR") == 0){
					if (choppedString[1].compare("200") == 0){
						replyFromserver.push_back(0);
					}
					else{
						replyFromserver.push_back(1);
					}
				}
				else if (choppedString[0].compare("GRP") == 0){
					string path = "bin/client/message_history/";
					path += choppedString[2];
					path += "-";
					path += choppedString[1];
					kunci.lock();
					addNotifSender(choppedString[1]);
					kunci.unlock();
					path += ".txt";
					choppedString.erase(choppedString.begin()+1);
					string a = choppedString[1];
					choppedString[1] = choppedString[2];
					choppedString[2] = a;
					vector<string> data = readExternalFileAutoCreate(path);
					data.push_back(protocolMaker(choppedString));
					writeExternalFile(path,data);
				}
				else{
					string path = "bin/client/message_history/";
					path += choppedString[2];
					path += "-";
					path += choppedString[1];
					kunci.lock();
					addNotifSender(choppedString[1]);
					kunci.unlock();
					path += ".txt";
					vector<string> data = readExternalFileAutoCreate(path);
					data.push_back(stringBuffer);
					//moveToBottom(path);
					writeExternalFile(path,data);
					//writeExternalFile2(path,ret); // append data baru aja
				}
			}
			else {
				cout << "server is shutdown, closing Client" << endl;
				close(sock);
				exit(0);
			}
		}
		usleep(10 * 1000);
	}
}

void addNotifSender(string user){
	string path = "bin/client/message_history/";
	path += username;
	vector<string> data = readExternalFileAutoCreate(path);
	int i=2; 
	bool found=false;
	while(i<data.size() && !found)	
	{
		if(data[i]==user) 
			found=true;
		else i++;
	}
	if(!found) 
		writeExternalFile2(path,user);
}

void deleteNotifSender(string user){
	string path = "bin/client/message_history/";
	path += username;
	vector<string> data = readExternalFileAutoCreate(path);
	int i=2; 
	bool found=false;
	while(i<data.size() && !found)	
	{
		if(data[i]==user){
			found=true;
			data.erase(data.begin()+i);
		} 
		else i++;
	}
	if(found) 
		writeExternalFile(path,data);
}

string reFormatMessage(vector<string> message,int read=-1){
	string ret = "";
	if (read == 0){
		ret += "v ";
	}
	else if (read == 1){
		ret += "vv ";
	}
	else if (read == 2){
		ret += "R ";
	}
	
	ret += "[";
	ret += message[3];
	ret += "] ";
	ret += message[1];
	ret += " : ";
	for (int i = 4;i < message.size();i++){
		if (i == message.size()-1){
			ret += message[i];
		}
		else{
			ret += message[i];
			ret += "||";
		}
	}
	return ret;
}

int sendToServer(string message){
	int ret = -1;
	bzero(buffer,buffer_size);
	strcpy(buffer,message.c_str());
	len = write(sock,buffer,buffer_size);  
	if (len >= 0){
		//message sent
		//await buffer rep ly
		bzero(buffer,buffer_size);
		while (replyFromserver.size() == 0){
			//waiting reply
		}
		ret = replyFromserver[0];
		replyFromserver.erase(replyFromserver.begin());
	}
	return ret;
}

void header(){
	cout << "*************************************" <<endl;
	cout << "     Welcome to LINKER MESSENGER  " << endl;
	cout << "*************************************" << endl;	
	cout << "1. Type signup to register" << endl;
	cout << "2. Type login if you already registered" << endl;
	cout << "3. Type exit to exit the application" << endl;
	cout << "------------------------------------" << endl;
}
void messageNotifier(){
	string path = "bin/client/message_history/";
	path += username;
	vector<string> data = readExternalFileAutoCreate(path);
	int i = 2;
	if (data.size() > 2){
		cout << "You have new message from ";
		if (data.size() == 3){
			cout << data[2] << "." << endl;
		}
		else if (data.size() == 4){
			cout << data[2] << " and " << data[3] << "." << endl;
		}
		else{
			while (i < data.size()){
				if (i == data.size()-1){
					cout << "and " << data[i] << "." << endl;
				}
				else if (i == data.size()-2){
					cout << data[i];
				}
				else{
					cout << data[i] << " , ";
				}
				i++;
			}
		}
	}
}
void start(){
	string str; 
	bool correct;
	correct = false;
	while (!correct){
		cout << endl << ">  ";
		cin >> str;
		if(str.compare("signup")==0){
			signup();
		}
		else if(str.compare("login")==0){
			login();
		}
		else if(str.compare("exit")==0){
			correct=true;
			active = false;
			closeConnection();
			exit(0);
		}
		else {
			cout << "Wrong input!" <<endl<<endl;
		}
	}
}

void signup(){
	string user;
	string pass;
	string ret;
	cout << "enter new username : ";
	getline(cin >> ws,user);
	cout << "enter password     : "; 
	getline(cin >> ws,pass);
	ret.append("REG||");
	ret.append(user);
	ret.append("||");
	ret.append(pass);
	ret.append("||");
	if (sendToServer(ret) == 0){
		cout << endl << "You've already registered!" << endl <<endl;
	}
}

void login(){
	string user,pass;
	string ret;
	cout << "Username : ";
	getline(cin >> ws,user);
	cout << "Password : ";
	getline(cin >> ws,pass);
	ret = "LIN||";
	ret += user;
	ret += "||";
	ret += pass;
	ret += "||";
	if (sendToServer(ret) == 0){
		username = user;
		cout << endl << "---------------WELCOME " <<  username << "!----------------" << endl;
		mainChat();
	}
	else {
		cout << "Invalid username or password." << endl;
	}
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

void header2(){
	cout << "--------------------------------------------------" << endl;
	cout << "MENU : " << endl;
	cout << "1. type message <destination> to send message" << endl;
	cout << "2. type create <group_name> to create a group" << endl;
	cout << "3. type join <group_name> to join a group" << endl;
	cout << "4. type leave <group_name> to leave a group" << endl; 
	cout << "5. type show <friend's_name or group_name> to show chat history" <<endl;
	cout << "6. type logout to sign out from the application" << endl;
	cout << "7. type help to know commands of this application" << endl;
	cout << "8. type exit to exit the application" << endl;
	cout << "--------------------------------------------------" << endl;
	cout << endl;
}
void mainChat(){
	string str,temp;
	int status;
	status=1; //0 if logout

	header2();
	while(status==1){
		cout << endl;
		kunci.lock();
		messageNotifier();
		cout << username << "> ";
		getline(cin >> ws,str);
		kunci.unlock();
		//logout
		if(str.compare("logout")==0){
			status=0;
			logout();
		}
		//message
		else if(str.substr(0,8).compare("message ")==0){
			temp = str.substr(8);
			sendMessage(temp);
		}
		//create group
		else if(str.substr(0,7).compare("create ")==0){
			temp = str.substr(7);
			groupCreate(temp);
		}
		//join group
		else if(str.substr(0,5).compare("join ")==0){
			temp = str.substr(5);
			groupJoin(temp);
		}
		//leave group
		else if(str.substr(0,6).compare("leave ")==0){
			temp = str.substr(6);
			groupLeave(temp);
		}
		//show chat history
		else if(str.substr(0,5).compare("show ")==0){
			temp = str.substr(5);
			readChat2(temp);
			deleteNotifSender(temp);
			//moveToBottom(..pathnya dimana..);
		}
		else if(str.compare("help")==0){
			header2();
		}
		//exit
		else if(str.compare("exit")==0){
			status=0;
			active = false;
			closeConnection();
			exit(0);
		}
		//empty input
		else if(str.compare("")==0){
			//do nothing
		}
		//wrong input
		else{
			cout <<"Unknown input, please type \'help\' for help." <<endl;
		}
	}
}

void logout(){
	vector<string> temp;
	string ret;
	temp.push_back("LOU");
	temp.push_back(username);
	ret = protocolMaker(temp);
	if (sendToServer(ret) == 0){
		username = "";
		start();
	}
	else{
		cout << "error logging out!" << endl;
	}
}

void groupJoin(string str){
	vector<string> temp;
	string ret;
	temp.push_back("JGR");
	temp.push_back(str);
	temp.push_back(username);
	ret = protocolMaker(temp);
	if (sendToServer(ret) == 0){
		cout << "You've joined " << str << " !" << endl << endl;
		sendMessageHidden(str,"Has join the group");
	}
	else{
		cout << "Failed to join " << str << " !" << endl;
		cout << "Reason : Group is not exist or you've joined !" << endl << endl;
	}
}

void groupCreate(string str){
	vector<string> temp;
	string ret;
	temp.push_back("CGR");
	temp.push_back(str);
	ret = protocolMaker(temp);
	if (sendToServer(ret) == 0){
		cout << str << " have been created !" << endl;
		groupJoin(str);
	}
	else{
		cout << "Failed to create " << str << " !" << endl;
		cout << "Reason : Group is existed !" << endl << endl;
	}
}

void groupLeave(string str){
	vector<string> temp;
	string ret;
	temp.push_back("LGR");
	temp.push_back(str);
	temp.push_back(username);
	ret = protocolMaker(temp);
	if (sendToServer(ret) == 0){
		cout << "You have successfully leave " << str << " !" << endl << endl;
	}
	else{
		cout << "Failed to leave " << str << " !" << endl;
		cout << "Reason : Group is not existed or you are not member of this group !" << endl << endl;
	}
}

void sendMessage(string str){
	vector<string> temp;
	string ret;
	string message;
	cout << "Please enter your message to " << str << " : ";
	getline(cin >> ws,message);
	temp.push_back("MSG");
	temp.push_back(username);
	temp.push_back(str);
	temp.push_back(currentDateTime());
	temp.push_back(message);
	ret = protocolMaker(temp);	
	if (sendToServer(ret) == 0){
		string path = "bin/client/message_history/";
		path += username;
		path += "-";
		path += str;
		path += ".txt";
		vector<string> data = readExternalFileAutoCreate(path);
		data.push_back(ret);
		//moveToBottom(path);
		//writeExternalFile(path,data);
		writeExternalFile2(path,ret); //append data baru aja
		cout << endl << "Message sent" << endl;
	}
	else{
		cout << "Failed to send messages or the user does not exist!" << endl << endl;
	}
}

void sendMessageHidden(string str,string message){
	vector<string> temp;
	string ret;
	//string message;
	//cout << "Please enter your message to " << str << " : ";
	//getline(cin >> ws,message);
	temp.push_back("MSG");
	temp.push_back(username);
	temp.push_back(str);
	temp.push_back(currentDateTime());
	temp.push_back(message);
	ret = protocolMaker(temp);	
	if (sendToServer(ret) == 0){
		string path = "bin/client/message_history/";
		path += username;
		path += "-";
		path += str;
		path += ".txt";
		vector<string> data = readExternalFileAutoCreate(path);
		data.push_back(ret);
		//moveToBottom(path);
		//writeExternalFile(path,data);
		writeExternalFile2(path,ret); //append data baru aja
	}
	else{
		cout << "Failed to send messages or the user does not exist!" << endl << endl;
	}	
}

void readChat(string str){
	string path = "bin/client/message_history/";
	path += username;
	path += "-";
	path += str;
	path += ".txt";
	vector<string> data = readExternalFileAutoCreate(path);
	vector<string> temp;
	int i = 1;
	cout << "Messages from " << str << endl << endl;
	
	
	while (i < data.size()){
		protocolDissambler(data[i],temp);
		if (temp[0].compare("marker") == 0){
			cout <<"================= new messages =================" << endl;	
		}
		else{
			cout << reFormatMessage(temp) << endl;
		}
		i++;
		temp.clear();
	}
}
void readChat2(string str){
	string line;
	string path = "bin/client/message_history/"; 
	path += username;
	path += "-";
	path += str;
	path += ".txt";
	vector<string> data = readExternalFile(path);
	vector<string> temp;
	int i = 1;
	cout << "Messages from " << str << endl << endl;
	
	while (i < data.size()){
		protocolDissambler(data[i],temp);
		if (temp[0].compare("newmsg") == 0){
			if (i != data.size()-1)
				cout <<"================= new messages =================" << endl;	
		}
		else{
			cout << reFormatMessage(temp) << endl;
		}
		i++;
		temp.clear();
	}
	moveToBottom(path);
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
		fileHeader += path.substr(find+1);
		/*string user = "marker||";
		user += username;
		user += "||";*/
		string user ="newmsg";
		vector<string> temp;
		temp.push_back(fileHeader);
		temp.push_back(user);
		writeExternalFile(path,temp);
		ret = readExternalFileAutoCreate(path);
	}
	return ret;
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
		cout << "File does not exist!" << endl;
	}
	return ret;
}

void writeExternalFile(string filename,vector<string> message){
	//ganti jadi append aja
	
	ofstream myfile;
	myfile.open(filename.c_str());
	for (int i = 0;i < message.size();i++){
		myfile << message[i];
		myfile << endl;
	}
	myfile.close();
}
void writeExternalFile2(string filename,string message){
	//ganti jadi append aja
	ofstream file;
	file.open(filename.c_str(), ios_base::app);
	file << message;
	file << endl;
	file.close();
}


//move new message marker to the bottom
void moveToBottom(string str){
//kalo ini dipake, marker nya newmsg
	string line;
	std::string path;
	//set path --> tinggal ganti path nya
	path = str;
	
	//open file
	ifstream in(path.c_str());
	if(!in.is_open()){
		cout << "input file failed to open\n";
	}

	//create new temp file
	ofstream out("bin/client/message_history/output.txt");
	while(getline(in,line)){		
		if(line.substr(0,6) != "newmsg"){
			out << line << endl;
		}else {
			//do nothing
		}
	}
	in.close();
	out.close();

	//delete original file
	remove(path.c_str());
	//cout << "path nya : " << path << endl;
	//rename file to original
	rename("bin/client/message_history/output.txt",path.c_str());

	// add mark new message
	addMarkNewMsg(path);
}

// add new message marker
void addMarkNewMsg(string path){
	ofstream myfile;
	myfile.open(path.c_str(), ios_base::app);
	myfile << "newmsg";
	myfile << endl;
	myfile.close();
	
}
