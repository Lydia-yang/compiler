#include"const.h"
#include"words.h"
#include"grammar.h"
#include"code.h"
#include"optimize.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
#include <cstdlib>
using namespace std;

char ch;					//last character read
int inum;					//integer from getsym
int slength;				//string length
enum symbol sym;			//last symbol read
int cc;						//character count
int ll;						//line length
int linecount;				//line of program
int errpos;					//the error posation
char line[llng];			//save one line of code
char id[30];				//identifier form getsym
string keyword[nkw];	//key words
enum symbol kwsym[nkw];		//symbols of key words 
char preid[30];
enum symbol presym;
struct table* tables[tmax];	//table
int tablecount;				//count total in table
char strtable[stmax][strmax];	//string table
int stcount;				//count total in string table
char strss[strmax];
string symname[40];
ifstream inf;
enum symbol stop[41];
ofstream codef;
ofstream errorf;

void setup();

int main(){
	int judged = 0;
	char infile[100];
	setup();
	scanf("%s", infile);
	inf.open(infile);
	if (!inf) {
		cout  << " can't open" << endl;
		return 0;
	}
	codef.open("code.txt", ios::out);
	errorf.open("errors.txt", ios::out);
	cc = 0;
	ll = 0;
	linecount = 0;
	tablecount = 0;
	stcount = 0;
	ch = ' ';
	errpos=0;
	//while(!inf.eof()) getsym();
	getsym();
	program();
	while(!inf.eof()){
		nextchar();
		if(!(ch==' ' || ch=='	' || ch=='\n')) judged = 1;
	}
	if(judged) error("superfluous statements");
	//if(errpos) 
	codef.close();
	codef.open("opt_code.txt", ios::out);
	if(!errpos) optmain();//optimize
	codef.close();
	pmain(0);//mips code
	pmain(1);//opt mips code
	errorf.close();
	return 0;
}

void setup(){
	keyword[0] = "return";
	keyword[1] = "const";
	keyword[2] = "int";
	keyword[3] = "char";
	keyword[4] = "void";
	keyword[5] = "main";
	keyword[6] = "if";
	keyword[7] = "else";
	keyword[8] = "while";
	keyword[9] = "switch";
	keyword[10] = "case";
	keyword[11] = "default";
	keyword[12] = "scanf";
	keyword[13] = "printf";

	kwsym[0] = returnsy;
	kwsym[1] = constsy;
	kwsym[2] = intsy;
	kwsym[3] = charsy;
	kwsym[4] = voidsy;
	kwsym[5] = mainsy;
	kwsym[6] = ifsy;
	kwsym[7] = elsesy;
	kwsym[8] = whilesy;
	kwsym[9] = switchsy;
	kwsym[10] = casesy;
	kwsym[11] = defaultsy;
	kwsym[12] = scanfsy;
	kwsym[13] = printfsy;

	symname[0] = " ident";
	symname[1] = " intcon";
	symname[2] = " charcon";
	symname[3] = " stringcon";
	symname[4] = " funcsy";
	symname[5] = " arraysy";
	symname[6] = " plus";
	symname[7] = " minus";
	symname[8] = " times";
	symname[9] = " divsy";
	symname[10] = " ==";
	symname[11] = " !=";
	symname[12] = " <";
	symname[13] = " <=";
	symname[14] = " >";
	symname[15] = " >=";
	symname[16] = " lparen";
	symname[17] = " rparen";
	symname[18] = " lbrack";
	symname[19] = " rbrack";
	symname[20] = " lbrace";
	symname[21] = " rbrace";
	symname[22] = " comma";
	symname[23] = " semicolon";
	symname[24] = "  becomes";
	symname[25] = " colon";
	symname[26] = " constsy";
	symname[27] = " intsy";
	symname[28] = " charsy";
	symname[29] = " voidsy";
	symname[30] = " mainsy";
	symname[31] = " ifsy";
	symname[32] = " elsesy";
	symname[33] = " whilesy";
	symname[34] = " switchsy";
	symname[35] = " casesy";
	symname[36] = " defaultsy";
	symname[37] = " scanfsy";
	symname[38] = " printfsy";
	symname[39] = " returnsy";
}