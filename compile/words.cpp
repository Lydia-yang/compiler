#include"words.h"
#include"const.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <cstdlib>
using namespace std;

extern char ch;					//last character read
extern int inum;					//integer from getsym
extern int slength;				//string length
extern enum symbol sym;			//last symbol read
extern int cc;						//character count
extern int ll;						//line length
extern int linecount;			//line of program
extern int errpos;					//the error posation
extern char line[llng];			//save one line of code
extern char id[30];				//identifier form getsym
extern char preid[30];
extern enum symbol presym;
extern string keyword[nkw];	//key words
extern enum symbol kwsym[nkw];		//symbols of key words 
extern string symname[40];
extern ifstream inf;
extern enum symbol stop[41];
extern char strtable[stmax][strmax];	//string table
extern int stcount;				//count total in string table
extern char strss[strmax];
extern ofstream errorf;

string ss1;
int numbercount=0;
int linejudge=0;
extern ofstream codef;

int searchstr(char name[100]){
	for(int i=0;i<stcount;i++){
		if(strcmp(name, strtable[i])==0) return i; 
	}
	return -1;
}

void outputfile1(string s) {
	
	
	if (!codef) {
		cout << "code.txt can't open" << endl;
		return;
		
	}
	codef << s;
	codef << "\n";
	//cout << s;
	
}

void outputfile(string s) {
	/*numbercount ++;
	ofstream f;
	f.open("result.txt", ios::app);
	if (!f) {
		cout << "reult.txt can't open" << endl;
		return;
		
	}
	f << numbercount<< " " ;
	f << s;
	f << "\n";
	//cout << s;
	f.close();*/
}
// error information
void error(string s){
	errpos=1;
	cout << s <<" in line "<<linecount<< endl;
	errorf << s <<" in line "<<linecount<< endl;
}
int skip(int total){
	int judge = 0;
	int nowline = linecount;
	while(judge==0){
		for(int i=0; i<total; i++){
			if(sym==stop[i]){
				judge = 1;
				return i;
			}
		}
		getsym();
		if (linecount!=nowline) return -1;
	}
}
// read next char
void nextchar(){
	if(cc == ll){ //end of the line
		if(inf.eof()){ // end of file
			cout << "program incomplete";
			error( "program incomplete");
			inf.close();
			exit(0);
		}
	//	if(errpos != 0){// start to deal with errors
		ll = 0;	//reset the length of line
		cc = 0;	//reset the character counter
		
		inf.getline(line, sizeof(line)); //read next line
		while(line[ll] != '\0') ll++;
		line[ll]=' ';
		ll++;
		line[ll]='\0';
	}
	if(ll!=0){
		ch = line[cc++];
		if(linejudge){
			linejudge=0;
			linecount ++;
		}
		if(ll==1) linejudge=1;
		if(cc==2) linecount ++;
	}
	else{
		ch = ' ';
		if(linejudge){
			linejudge=0;
			linecount ++;
		}
		linejudge=1;
	}
}
// words analysize
void getsym(){
	int i, j, k;

label:while(ch == ' ' || ch == '	' || ch == '\n' ) nextchar();	//skip all space
	presym = sym;
	strcpy(preid,id);
	if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'){ //start to identify word
		int counted =0 ;
		k = 0;
		do{
			counted ++;
			if(k<29) id[k++] = ch;
			if(counted==30) error("words too long");
			nextchar();
		}while((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || (ch >= 'A' && ch <= 'Z'));
		id[k] = '\0';
		//search key words
		j = 0;
		for(i=0; i<nkw; i++){
			if(strcmp(strlwr(id), keyword[i].data())==0){
				j = 1;
				break;
			}
		}
		if(j){
			sym = kwsym[i];
			ss1 = id;
			ss1 += symname[sym];
		}
		else{
			sym = ident;
			ss1 = id;
			ss1 += " ident";
		}
		outputfile(ss1);
	}else if(ch >= '0' && ch <= '9'){ // number
		k = 0; // count the length of number
		inum = 0; // intger
		sym = intcon;
		while(ch >= '0' && ch <= '9'){
			inum = inum *10 + (ch - '0');
			if(k<29) id[k++] = ch;
			nextchar();
		}
		id[k] = '\0';
		if((k > kmax) || (inum > nmax)){
			error("integer beyond boundary"); // uper bond
		}
		if(k!=1 && id[0]=='0'){
			error("0 ahead of nubers");//have 0
		}
		ss1 = id;
		ss1 += " intcon";
		outputfile(ss1);
	}else if(ch == '='){
		nextchar();
		if(ch == '='){
			sym = eql;
			nextchar();
			ss1 = "== eql";
		}else{
			sym = becomes;
			ss1 = "= becomes";
		}
		outputfile(ss1);
	}else if(ch == '<'){
		nextchar();
		if(ch == '='){
			sym = leq;
			nextchar();
			ss1 = "<= leq";
		}else {
			sym = lss;
			ss1 = "< lss" ;
		}
		outputfile(ss1);
	}else if(ch == '>'){
		nextchar();
		if(ch == '='){
			sym = geq;
			nextchar();
			ss1 = ">=  geq";
		}else{
			sym = gtr;
			ss1 = "> gtr";
		}
		outputfile(ss1);
	}else if(ch == '\''){
		nextchar();
		if(ch == '\''){
			error("empty char");//empty char
			nextchar();
			return;
		}
		if(!(ch == '_' || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') 
			|| ch == '+' || ch == '-' || ch == '*' || ch == '/')){
				error("illegal character"); //not legal char
		}else{
			id[0] = ch;
		}
			nextchar();
			if(ch == '\''){
				sym = charcon;
				nextchar();
				ss1 = id[0] ;
				ss1 += " charcon";
				outputfile(ss1);
			}else{
				error("not a char");//not a char
				return;
			}
		
	}else if(ch == '\"'){
		nextchar();
		if(ch == '\"'){
			//error("empty string");//empty string
			sym = stringcon;
			strss[0]='\0';
			nextchar();
			return;
		}
		k = 0;
		int counted=0;
		while(ch != '\"'){
			while(!(ch ==32 || ch == 33 || (ch >= 35 && ch <= 126))){
				error("illegal character");//unlegal char
				nextchar();
			}
			//id[k] = ch;
			counted++;
			if(k<99){
			strss[k] = ch;
			strtable[stcount][k] = ch;
			k++;
			}
			if(counted==100) error("string too long");
			nextchar();
		}
		//id[k] = '\0';
		strtable[stcount][k] = '\0';
		strss[k] = '\0';
		int resulted = searchstr(strtable[stcount]);
		if(resulted==-1) stcount++;
		slength = k;
		sym = stringcon;
		nextchar();
		ss1 = strtable[stcount-1] ;
		ss1 +=" stringcon";
		outputfile(ss1);
	}else if(ch == '!'){
		nextchar();
		if(ch == '='){
			sym = neq;
			nextchar();
			ss1 ="!= neq" ;
			outputfile(ss1);
		}else{
			error("lack a \'=\'");// need a = 
			return;
		}
	}else if(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '(' || ch == ')' || ch == '[' 
		|| ch == ']'|| ch == '{' || ch == '}' || ch == ',' || ch == ';' || ch == ':'){
		switch(ch){
			case '+':
				sym = plus;
				break;
			case '-':
				sym = minus;
				break;
			case '*':
				sym = times;
				break;
			case '/':
				sym = divsy;
				break;
			case '(':
				sym = lparen;
				break;
			case ')':
				sym = rparen;
				break;
			case '[':
				sym = lbrack;
				break;
			case ']':
				sym = rbrack;
				break;
			case '{':
				sym = lbrace;
				break;
			case '}':
				sym = rbrace;
				break;
			case ',':
				sym = comma;
				break;
			case ';':
				sym = semicolon;
				break;
			case ':':
				sym = colon;
				break;
		}
		ss1 = ch ;
		ss1 += symname[sym];
		nextchar();
		outputfile(ss1);
	}else{
		error("illegal character");//illegal char
		nextchar();
		goto label;
	}
}

