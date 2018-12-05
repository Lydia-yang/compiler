#include"const.h"
#include"code.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;

extern char line[llng];			//save one line of code
extern char id[30];				//identifier form getsym
extern int cc;						//character count
extern char ch;					//last character read
extern char strtable[stmax][strmax];	//string table
extern int stcount;				//count total in string table
extern char tostr[20];
extern int inum;
//table
extern struct table* tables[tmax];	//table
extern int tablecount;				//count total in table
extern char name[30];
extern enum symbol obj;//ident(var) array const
extern enum symbol typ;//int char void
extern int level;//0-global
extern int adr; //const(int, char), array adress, var(normal?register:adress)

enum m_symbol m_sym;
enum m_symbol m_kwsym[m_nkw];
string	m_keyword[m_nkw];
char strs[strmax];
int scount;
int pcount;
int acount=0;
int ismain=0;

string ss2;
int tjudge=0,spjudge=0;
ifstream mcodef;
ofstream f;
void outputfile2(string s) {
	if (!f) {
		cout << "mips_code.txt can't open" << endl;
		return;
		
	}
	f << s;
	f << "\n";
	//cout << s;
	//f.close();
}
void m_setup(){
	m_keyword[0] = "VAR";
	m_keyword[1] = "PARA";
	m_keyword[2] = "RET";
	m_keyword[3] = "PUSH";
	m_keyword[4] = "CALL";
	m_keyword[5] = "GOTO";
	m_keyword[6] = "BZ";
	m_keyword[7] = "const";
	m_keyword[8] = "scanf";
	m_keyword[9] = "printf";
	m_keyword[10] = "int";
	m_keyword[11] = "char";
	m_keyword[12] = "void";
	m_keyword[13] = "main";

	m_kwsym[0] = mvarsy;
	m_kwsym[1] = mparasy;
	m_kwsym[2] = mretsy;
	m_kwsym[3] = mpushsy;
	m_kwsym[4] = mcallsy;
	m_kwsym[5] = mgotosy;
	m_kwsym[6] = mbzsy;
	m_kwsym[7] = mconstsy;
	m_kwsym[8] = mscanfsy;
	m_kwsym[9] = mprintfsy;
	m_kwsym[10] = mintsy;
	m_kwsym[11] = mcharsy;
	m_kwsym[12] = mvoidsy;
	m_kwsym[13] = mmainsy;
}

void m_entertable(char name[30], enum symbol obj, enum symbol typ, int level, int adr, bool normal){
	struct table* p;
	p=(struct table*)malloc(sizeof(struct table));
	strcpy(p->name, name);
	p->obj=obj;
	p->typ=typ;
	p->level=level;
	p->adr=adr;
	p->normal=normal;
	tables[tablecount++] = p;
}
int m_search(char name[30]){
	for(int i=tablecount-1; i>=0; i--){
		if(strcmp(name, tables[i]->name)==0 && tables[i]->level<=level){
			return i;
		}
	}
	return -1;
}
void m_del(){
	for(int i=0; i<tablecount;i++){
		if(tables[i]->level>level){
			tablecount = i;
		}
	}
}

int m_nextline(){
	do{
		if(mcodef.eof()){
			mcodef.close();
			outputfile2("li $v0, 10");
			outputfile2("syscall");
			f.close();
			//exit(0);
			return -1;
		}
		mcodef.getline(line, sizeof(line));
		cc=0;
	}while(line[0]=='\0');
	ch=' ';
	return 0;
}
void m_nextchar(){
	ch = line[cc++];
}
void m_getsym(){
	int i, j, k;
	while(ch == ' ') m_nextchar();	//skip all space
	if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_'){ //start to identify word
		k = 0;
		do{
			id[k++] = ch;
			m_nextchar();
		}while((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_' || (ch >= 'A' && ch <= 'Z'));
		id[k] = '\0';
		//search key words
		j = 0;
		for(i=0; i<m_nkw; i++){
			if(strcmp(id, m_keyword[i].data())==0){
				j = 1;
				break;
			}
		}
		if(j){
			m_sym = m_kwsym[i];
		}
		else{
			m_sym = mident;
		}
	}else if(ch >= '0' && ch <= '9'){ // number
		k = 0; // count the length of number
		inum = 0; // intger
		m_sym = mintcon;
		while(ch >= '0' && ch <= '9'){
			inum = inum *10 + (ch - '0');
			id[k++] = ch;
			m_nextchar();
		}
		id[k] = '\0';
	}else if(ch == '='){
		m_nextchar();
		if(ch == '='){
			m_sym = meql;
			m_nextchar();
		}else{
			m_sym = mbecomes;
		}
	}else if(ch == '<'){
		m_nextchar();
		if(ch == '='){
			m_sym = mleq;
			m_nextchar();
		}else {
			m_sym = mlss;
		}
	}else if(ch == '>'){
		m_nextchar();
		if(ch == '='){
			m_sym = mgeq;
			m_nextchar();
		}else{
			m_sym = mgtr;
		}
	}else if(ch == '\''){
		m_nextchar();
		id[0] = ch;
		id[1] = '\0';
		m_nextchar();
		m_sym = mcharcon;
		m_nextchar();
		
	}else if(ch == '\"'){
		m_nextchar();
		k = 0;
		while(ch != '\"'){
			strs[k] = ch;
			k++;
			m_nextchar();
		}
		strs[k] = '\0';
		m_sym = mstringcon;
		m_nextchar();
	}else if(ch == '!'){
		m_nextchar();
		if(ch == '='){
			m_sym = mneq;
			m_nextchar();
		}
	}else if(ch == '+' || ch == '-' || ch == '*' || ch == '/' ||ch == '(' || ch == ')'|| ch == '[' || ch == ']'|| ch == '$' || ch == ':'){
		switch(ch){
			case '+':
				m_sym = mplus;
				break;
			case '-':
				m_sym = mminus;
				break;
			case '*':
				m_sym = mtimes;
				break;
			case '/':
				m_sym = mdivsy;
				break;
			case '[':
				m_sym = mlbrack;
				break;
			case ']':
				m_sym = mrbrack;
				break;
			case '(':
				m_sym = mlparen;
				break;
			case ')':
				m_sym = mrparen;
				break;
			case ':':
				m_sym = mcolon;
				break;
			case '$':
				m_sym = mdollar;
				break;
		}
		m_nextchar();
	}else{
		cout<<"illegal character"<<endl;//illegal char
	}
}

void pmain(int isopt){
	cc=0;
	tablecount=0;
	level=0;
	acount=0;
    ismain=0;
	tjudge=0;
	spjudge=0;
	if(isopt) mcodef.open("opt_code.txt");//opt mcode text
	else mcodef.open("code.txt");//mcode text
	if (!mcodef) {
		cout  << "code can't open" << endl;
		return;
	}
	if(isopt) f.open("opt_mips_code.txt", ios::out);
	else f.open("mips_code.txt", ios::out);
	m_setup();
	outputfile2(".data");
	if(stcount!=0){
		gstr();
	}
	m_nextline();
	m_getsym();
	if(m_sym==mconstsy){
		do{
			dconst();
			m_nextline();
			m_getsym();
		}while(m_sym==mconstsy);
	}
	if(m_sym==mvarsy){
		do{
			dvar();
			m_nextline();
			m_getsym();
		}while(m_sym==mvarsy);
	}
	if(m_sym==mintsy||m_sym==mcharsy||m_sym==mvoidsy){
		do{
			//if(m_sym==mintsy||m_sym==mcharsy) trfunction();
			if(tvfunction()==-1) break;
		}while(m_sym==mintsy||m_sym==mcharsy||m_sym==mvoidsy);
	}
	
}
void gstr(){
	for(int i=0;i<stcount;i++){
		ss2="a$";
		sprintf(tostr,"%d",i);
		ss2+=tostr;
		ss2+=":.asciiz \"";
		ss2+= strtable[i];
		ss2+="\"";
		outputfile2(ss2);
	}
}
void dconst(){
	string s1;
	//int char
	m_getsym();
	s1=m_sym==mintsy?": .word ":": .byte ";
	typ=m_sym==mintsy?intsy:charsy;
	//ident
	m_getsym();
	strcpy(name, id);
	ss2 = id;
	ss2 += s1;
	//=
	m_getsym();
	if(m_sym!=mbecomes) cout<<"not ="<<endl;
	//value
	m_getsym();
	if(m_sym==mplus||m_sym==mminus){
		if(m_sym==mminus) ss2+="-";
		m_getsym();
	}
	if(m_sym==mcharcon){
		ss2+="\'";
		ss2+=id[0];
		ss2+="\'";
	}else{
		ss2+=id;
	}
	outputfile2(ss2);
	obj=constsy;
	m_entertable(name, obj, typ, level, 0, true);
}
void dvar(){
	string s1;
	//int char
	m_getsym();
	s1=m_sym==mintsy?": .word ":": .byte ";
	typ=m_sym==mintsy?intsy:charsy;
	//ident
	m_getsym();
	strcpy(name, id);
	ss2 = id;
	ss2 += s1;
	//array
	if(ch!='\0'){
		//vaule
		m_getsym();
		if(m_sym!=mlbrack) cout<<"not ["<<endl;
		m_getsym();
		ss2+="0:";
		ss2+=id;
		obj=arraysy;
	}else{
		obj=ident;
		ss2+=typ==intsy?"0":"\'0\'";
	}
	outputfile2(ss2);
	m_entertable(name, obj, typ, level, 0, true);
}

int tvfunction(){
	string spara;
	int j=m_sym==mvoidsy?1:0;
	int sp=0, rjudge=0;
	if(tjudge==0){
		outputfile2(".text");
		outputfile2("j main");
		tjudge=1;
	}
	//ident
	m_getsym();
	if(m_sym==mmainsy) ismain=1;
	ss2=id;
	ss2+=":";
	outputfile2(ss2);

	level++;
	scount=scmax;
	pcount=0;
	sp=0;
	if(m_nextline()==-1) return -1;
	m_getsym();
	//para
	ss2="";
	if(m_sym==mparasy){
		do{
			sp=paras(sp);
			if(m_nextline()==-1) return -1;
			m_getsym();
		}while(m_sym==mparasy);
	}
	//const
	if(m_sym==mconstsy){
		do{
			tconst();
			if(m_nextline()==-1) return -1;
			m_getsym();
		}while(m_sym==mconstsy);
	}
	//var
	if(m_sym==mvarsy){
		do{
			sp=tvar(sp);
			if(m_nextline()==-1) return -1;
			m_getsym();
		}while(m_sym==mvarsy);
	}
	//sp
	spara=ss2;
	ss2="addiu $sp, $sp,";
	sp=(sp%4==0)?sp:sp+4-sp%4;
	sprintf(tostr,"%d",-sp-4);
	ss2+=tostr;
	outputfile2(ss2);
	//para
	outputfile2(spara);
	//ra
	ss2="sw $ra, ";
	sprintf(tostr,"%d",sp);
	ss2+=tostr;
	ss2+="($sp)";
	outputfile2(ss2);
	//statements
	if(m_statements(sp)==-1) rjudge=-1;
	level--;
	m_del();
	//sp
	if(j){
		if(ismain){
			ss2="addiu $sp, $sp,";
			sprintf(tostr,"%d",(sp+4));
			ss2+=tostr;
			outputfile2(ss2);
			//outputfile2("li $v0, 10");
			//outputfile2("syscall");
		}else{
			ss2="lw $ra,";
			sprintf(tostr,"%d",sp);
			ss2+=tostr;
			ss2+="($sp)";
			outputfile2(ss2);
			ss2="addiu $sp, $sp,";
			sprintf(tostr,"%d",(sp+4));
			ss2+=tostr;
			outputfile2(ss2);
			outputfile2("jr $ra");
		}
	}
	return rjudge;
}

int paras(int sp){//$a0-$a3(4-7)
	bool normal=true;
	//int char
	m_getsym();
	typ=m_sym==mintsy?intsy:charsy;
	//ident
	m_getsym();
	strcpy(name, id);
	obj=ident;
	normal=false;
	if(typ==intsy){
		adr=(sp%4==0)?sp:sp+4-sp%4;
		sp=adr+4;
		if(pcount<pcmax){
			ss2+="\nsw $a";
			sprintf(tostr,"%d", pcount);
			ss2+=tostr;
			ss2+=",";
			sprintf(tostr,"%d", adr);
			ss2+=tostr;
			ss2+="($sp)";
		}else{
			ss2+="\nsw $t";
			sprintf(tostr,"%d", pcount);
			ss2+=tostr;
			ss2+=",";
			sprintf(tostr,"%d", adr);
			ss2+=tostr;
			ss2+="($sp)";
		}
	}else{
		adr=sp;
		sp++;
		if(pcount<pcmax){
			ss2+="\nsb $a";
			sprintf(tostr,"%d", pcount);
			ss2+=tostr;
			ss2+=",";
			sprintf(tostr,"%d", adr);
			ss2+=tostr;
			ss2+="($sp)";
		}else{
			ss2+="\nsb $t";
			sprintf(tostr,"%d", pcount);
			ss2+=tostr;
			ss2+=",";
			sprintf(tostr,"%d", adr);
			ss2+=tostr;
			ss2+="($sp)";
		}
	}
	pcount++;
	m_entertable(name, obj, typ, level, adr, normal);
	return sp;
}
void tconst(){
	//int char
	m_getsym();
	typ=m_sym==mintsy?intsy:charsy;
	//ident
	m_getsym();
	strcpy(name, id);
	//=
	m_getsym();
	if(m_sym!=mbecomes) cout<<"not ="<<endl;
	//value
	m_getsym();
	if(m_sym==mplus||m_sym==mminus){
		if(m_sym==mminus){
			m_getsym();
			adr=0-inum;
		}
		else{
			m_getsym();
			adr=inum;
		}
	}else if(m_sym==mintcon){
		adr=inum;
	}else{
		adr=id[0];
	}
	obj=constsy;
	m_entertable(name, obj, typ, level, adr, true);
}
int tvar(int sp){//$s0-$s7(16-23)
	bool normal=true;
	//int char
	m_getsym();
	typ=m_sym==mintsy?intsy:charsy;
	//ident
	m_getsym();
	strcpy(name, id);
	//array
	if(ch!='\0'){
		//vaule
		m_getsym();
		if(m_sym!=mlbrack) cout<<"not ["<<endl;
		m_getsym();
		//adr sp
		obj=arraysy;
		normal=false;
		if(typ==intsy){
			adr=(sp%4==0)?sp:sp+4-sp%4;
			sp=adr+4*inum;
		}else{
			adr=sp;
			sp+=inum;
		}
	}else{
		obj=ident;
		normal=false;
		if(typ==intsy){
		adr=(sp%4==0)?sp:sp+4-sp%4;
		sp=adr+4;
		}else{
			adr=sp;
			sp++;
		}
	}
	m_entertable(name, obj, typ, level, adr, normal);
	return sp;
}


int m_statements(int sp){
	if(m_sym==mgotosy||m_sym==mbzsy||m_sym==mident||m_sym==mdollar||m_sym==mpushsy||m_sym==mcallsy||m_sym==mretsy||m_sym==mscanfsy||m_sym==mprintfsy){
		do{
			switch(m_sym){
				case mgotosy:
					gotostatement();
					break;
				case mbzsy:
					bzstatement();
					break;
				case mident:case mdollar:
					if(m_sym==mdollar||(id[0]>='a' && id[0]<='z')||id[0]=='_') opeartors(); 
					else labels();
					break;
				case mpushsy:
					pushstatement();
					break;
				case mcallsy:
					callstatement();
					break;
				case mretsy:
					ret(sp);
					break;
				case mscanfsy:
					scan();
					break;
				case mprintfsy:
					print();
					break;
				default:
					cout<<"error"<<endl;
			}
			if(m_nextline()==-1) return -1;
			m_getsym();
		}while(m_sym==mgotosy||m_sym==mbzsy||m_sym==mident||m_sym==mdollar||m_sym==mpushsy||m_sym==mcallsy||m_sym==mretsy||m_sym==mscanfsy||m_sym==mprintfsy);
	}	
	return 0;
}

int addexps(string a1){
	int intjudge=1;
	int neg=0;
	//expression +-intcon charcon ident  $, ret
		if(m_sym==mplus||m_sym==mminus){
			if(m_sym==mminus) neg=1;
			m_getsym();
		}
		if(m_sym==mintcon){
			ss2="addiu ";
			ss2+=a1;
			ss2+=",$0,";
			if(neg) inum=-inum;
			sprintf(tostr,"%d",inum);
			ss2+=tostr;
		}else if(m_sym==mcharcon){//-????????????????
			intjudge=0;
			ss2="addiu ";
			ss2+=a1;
			ss2+=", $0,\'";
			ss2+=id[0];
			ss2+="\'";
		}else if(m_sym==mident){//ident const array
			int result=m_search(id);
			intjudge=tables[result]->typ==intsy?1:0;
			if(tables[result]->level==0){//global
				if(tables[result]->obj==arraysy){//array
					m_getsym();//[
					m_getsym();//
					addexps("$t0");
					outputfile2(ss2);
					if(tables[result]->typ==intsy) outputfile2("sll $t0, $t0,2");
					ss2=tables[result]->typ==intsy?"lw ":"lb ";
					ss2+=a1;
					ss2+=",";
					ss2+=tables[result]->name;
					ss2+="($t0)";
					m_getsym();//]
					if(neg){
						outputfile2(ss2);
						ss2="sub ";
						ss2+=a1;
						ss2+=", $0,";
						ss2+=a1;
					}
				}else{//const ident
					ss2=tables[result]->typ==intsy?"lw ":"lb ";
					ss2+=a1;
					ss2+=",";
					ss2+=id;
					if(neg){
						outputfile2(ss2);
						ss2="sub ";
						ss2+=a1;
						ss2+=", $0,";
						ss2+=a1;
					}
				}
			}else{
				if(tables[result]->obj==constsy){//const
					ss2="addiu ";
					ss2+=a1;
					ss2+=", $0,";
					if(neg) sprintf(tostr,"%d",-(tables[result]->adr));
					else sprintf(tostr,"%d",tables[result]->adr);
					ss2+=tostr;
				}else if(tables[result]->obj==arraysy){//array
					m_getsym();//[
					m_getsym();//
					addexps("$t0");
					outputfile2(ss2);
					if(tables[result]->typ==intsy) outputfile2("sll $t0, $t0,2");
					ss2="addiu $t0, $t0,";
					sprintf(tostr,"%d",tables[result]->adr);
					ss2+=tostr;
					outputfile2(ss2);
					m_getsym();//]
					outputfile2("add $t0, $t0, $sp");
					ss2=tables[result]->typ==intsy?"lw ":"lb ";
					ss2+=a1;
					ss2+=",0($t0)";
					if(neg){
						outputfile2(ss2);
						ss2="sub ";
						ss2+=a1;
						ss2+=", $0,";
						ss2+=a1;
					}
				}else{//ident
					if(tables[result]->normal){
						ss2=neg?"sub ":"add ";
						ss2+=a1;
						ss2+=", $0,$";
						sprintf(tostr,"%d",tables[result]->adr);
						ss2+=tostr;
					}else{
						ss2=tables[result]->typ==intsy?"lw ":"lb ";
						ss2+=a1;
						ss2+=",";
						sprintf(tostr,"%d",tables[result]->adr);
						ss2+=tostr;
						ss2+="($sp)";
						if(neg){
							outputfile2(ss2);
							ss2="sub ";
							ss2+=a1;
							ss2+=", $0,";
							ss2+=a1;
						}
					}
				}
			}
		}else if(m_sym==mretsy){
			/*m_getsym();//$
			m_getsym();
			intjudge=inum==1?1:0;*/
			intjudge=1;
			ss2=neg?"sub ":"add ";
			ss2+=a1;
			ss2+=",$0, $v0";
		}else{//$
			m_getsym();
			if(m_sym==mdollar){
				intjudge=0;
				m_getsym();
			}
				ss2=neg?"sub ":"add ";
				ss2+=a1;
				ss2+=", $0, $";
				sprintf(tostr,"%d",inum+16);
				ss2+=tostr;
		}
	//outputfile2(ss2);
		return intjudge;
}
void gotostatement(){
	ss2="j ";
	m_getsym();
	ss2+=id;
	outputfile2(ss2);
}
void bzstatement(){
	enum m_symbol op;
	//ident1 +-intcon charcon ident  $
	m_getsym();
	addexps("$t1");
	outputfile2(ss2);
	m_getsym();
	if(8<=m_sym&&m_sym<=13){
		//op
		op=m_sym;
		//ident2
		m_getsym();
		addexps("$t2");
		outputfile2(ss2);
		//
		switch(op){
		case meql://==
			ss2="bne $t1, $t2 ";
			break;
		case mneq://!=
			ss2="beq $t1, $t2 ";
			break;
		case mlss://<
			ss2="bge $t1, $t2 ";
			break;
		case mleq://<=
			ss2="bgt $t1, $t2 ";
			break;
		case mgtr://>
			ss2="ble $t1, $t2 ";
			break;
		case mgeq://>=
			ss2="blt $t1, $t2 ";
			break;
		}
		//label
		m_getsym();
	}else{//label
		ss2="beq $0,$t1 ";
	}
	ss2+=id;
	outputfile2(ss2);
}
void labels(){
	ss2=id;
	ss2+=":";
	outputfile2(ss2);
}
void opeartors(){//t0-t9
	enum m_symbol op=mident;
	string s1,s2;
	int judge=0;
	//ident arrayy $
	if(m_sym==mdollar){
		m_getsym();
		if(m_sym==mdollar) m_getsym();
		s1="$";
		sprintf(tostr,"%d",inum+16);
		s1+=tostr;
	}else{
	
		int result=m_search(id);
		if(tables[result]->level==0){//global
			if(tables[result]->obj==arraysy){//t3????????????????????????????????
				m_getsym();//[
				m_getsym();//exps
				addexps("$t3");
				s2=ss2;
				if(tables[result]->typ==intsy) s2+="\nsll $t3, $t3,2";
				s2+=tables[result]->typ==intsy?"\nsw $t1,":"\nsb $t1,";
				s2+=tables[result]->name;
				s2+="($t3)";
				m_getsym();//]
				s1="$t1";
				judge=1;
			}else{
				s2=tables[result]->typ==intsy?"sw $t1,":"sb $t1,";
				s2+=id;
				s1="$t1";
				judge=1;
			}
		}else{
			if(tables[result]->obj==arraysy){
				m_getsym();//[
				m_getsym();//exps
				addexps("$t3");
				s2=ss2;
				if(tables[result]->typ==intsy) s2+="\nsll $t3, $t3,2";
				s2+="\naddiu $t3, $t3,";
				sprintf(tostr,"%d",tables[result]->adr);
				s2+=tostr;
				s2+="\nadd $t3, $t3, $sp";
				s2+=tables[result]->typ==intsy?"\nsw $t1,0($t3)":"\nsb $t1,0($t3)";
				m_getsym();//]
				s1="$t1";
				judge=1;
			}else{
				if(tables[result]->normal){
					s1="$";
					sprintf(tostr,"%d",tables[result]->adr);
					s1+=tostr;
				}else{
					s2=tables[result]->typ==intsy?"sw $t1,":"sb $t1,";
					sprintf(tostr,"%d",tables[result]->adr);
					s2+=tostr;
					s2+="($sp)";
					s1="$t1";
					judge=1;
				}
			}
		}
	}
	//=
	m_getsym();
	//op2
	m_getsym();
	addexps("$t1");
	outputfile2(ss2);
	if(ch!='\0'){
		//op
		m_getsym();
		op=m_sym;
		//op3
		m_getsym();
		addexps("$t2");
		outputfile2(ss2);
	}
	switch(op){
		case mplus:
			ss2="add ";
			ss2+=s1;
			ss2+=",$t1,$t2";
			break;
		case mminus:
			ss2="sub ";
			ss2+=s1;
			ss2+=",$t1,$t2";
			break;
		case mtimes:
			ss2="mul ";
			ss2+=s1;
			ss2+=",$t1,$t2";
			break;
		case mdivsy:
			ss2="div ";
			ss2+=s1;
			ss2+=",$t1,$t2";
			break;
		case mident:
			ss2="move ";
			ss2+=s1;
			ss2+=",$t1";
			break;
		}
	outputfile2(ss2);
	if(judge) outputfile2(s2);
}
void pushstatement(){//$a0-a3 4-7
	string s1;
	m_getsym();
	if(acount<pcmax){
		s1="$a";
		sprintf(tostr,"%d",acount);
		s1+=tostr;
		addexps(s1);
		outputfile2(ss2);
		acount++;
	}else{//$t4-$t7
		s1="$t";
		sprintf(tostr,"%d",acount);
		s1+=tostr;
		addexps(s1);
		acount++;
		outputfile2(ss2);
	}
}
void callstatement(){
	int sp=0;
	//sp
		int sn=scmax+2;
		ss2="addiu $sp, $sp,";
		sprintf(tostr,"%d",-(sn)*4);
		ss2+=tostr;
		outputfile2(ss2);
		for(int i=0;i<sn;i++){//$s0-$s7(16-23) $t8,t9
			ss2="sw $";
			sprintf(tostr,"%d",16+i);
			ss2+=tostr;
			ss2+=",";
			sprintf(tostr,"%d",sp);
			ss2+=tostr;
			ss2+="($sp)";
			outputfile2(ss2);
			sp+=4;
		}
	
	acount=0;
	//jal
	m_getsym();
	ss2="jal ";
	ss2+=id;
	outputfile2(ss2);
	//sp
	sp=0;
	sn=scmax+2;
		for(int i=0;i<sn;i++){//$s0-$s7(16-23) $t8,t9
			ss2="lw $";
			sprintf(tostr,"%d",16+i);
			ss2+=tostr;
			ss2+=",";
			sprintf(tostr,"%d",sp);
			ss2+=tostr;
			ss2+="($sp)";
			outputfile2(ss2);
			sp+=4;
		}
	ss2="addiu $sp, $sp,";
	sprintf(tostr,"%d",(sn)*4);
	ss2+=tostr;
	outputfile2(ss2);
}
void ret(int sp){//$v0
	if(ismain){
		ss2="addiu $sp, $sp,";
		sprintf(tostr,"%d",(sp+4));
		ss2+=tostr;
		outputfile2(ss2);
		outputfile2("li $v0, 10");
		outputfile2("syscall");
		return;
	}
	int neg=0;
	if(ch!='\0'){
		m_getsym();
		addexps("$v0");
		outputfile2(ss2);
	}
	ss2="lw $ra,";
	sprintf(tostr,"%d",sp);
	ss2+=tostr;
	ss2+="($sp)";
	outputfile2(ss2);
	ss2="addiu $sp, $sp,";
	sprintf(tostr,"%d",(sp+4));
	ss2+=tostr;
	outputfile2(ss2);
	outputfile2("jr $ra");
}
void scan(){
	int result;
	//ident
	m_getsym();
	result=m_search(id);
	if(tables[result]->typ==intsy){
		outputfile2("li $v0 5");
		outputfile2("syscall");
	}else{
		outputfile2("li $v0 12");
		outputfile2("syscall");
	}
	if(tables[result]->level==0){
		ss2=tables[result]->typ==intsy?"sw $v0,":"sb $v0,";
		ss2+=id;
	}else{
		if(tables[result]->normal){
			ss2="add $";
			sprintf(tostr,"%d",tables[result]->adr);
			ss2+=tostr;
			ss2+=",$v0,$zero";
		}else{
			ss2=tables[result]->typ==intsy?"sw $v0,":"sb $v0,";
			sprintf(tostr,"%d",tables[result]->adr);
			ss2+=tostr;
			ss2+="($sp)";
		}
	}
	outputfile2(ss2);
}
void print(){
	string s1;
	outputfile2("move $t1, $a0");
	m_getsym();
	if(m_sym==stringcon){
		outputfile2("li $v0 4");
		ss2="la $a0, ";
		for(int i=0;i<stcount;i++){
			if(strcmp(strs,strtable[i])==0){
				ss2+="a$";
				sprintf(tostr,"%d",i);
				ss2+=tostr;
				break;
			}
		}
		outputfile2(ss2);
	}else{
		//expression +-intcon charcon ident  $
		/*if(m_sym==mplus||m_sym==mminus) m_getsym();
		if(m_sym==mintcon){//int
			outputfile2("li $v0, 1");
		}else if(m_sym==mcharcon){//char
			outputfile2("li $v0, 11");
		}else if(m_sym==mident){
			int result=m_search(id);
			ss2=tables[result]->typ==intsy?"li $v0, 1":"li $v0, 11";
			outputfile2(ss2);
		}else if(m_sym==mretsy){
			if(cfun) outputfile2("li $v0, 11");
			else outputfile2("li $v0, 1");
		}else{//$
			outputfile2("li $v0, 1");
		}*/
		int result=addexps("$a0");
		outputfile2(ss2);
		if(result) outputfile2("li $v0, 1");//int
		else outputfile2("li $v0, 11");//char
	}
	outputfile2("syscall");
	outputfile2("move $a0, $t1");
}