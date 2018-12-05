#include"words.h"
#include"const.h"
#include"grammar.h"
#include<iostream>

extern int inum;
extern char id[30];				//identifier form getsym
extern enum symbol sym;			//last symbol read
extern char preid[30];
extern enum symbol presym;
extern enum symbol stop[41];
int pre = 0;
extern struct table* tables[tmax];	//table
extern int tablecount;				//count total in table
extern string symname[40];
extern char strtable[stmax][strmax];	//string table
extern int stcount;				//count total in string table
extern char strss[strmax];

string ss;
int sreg[scmax];			//1-which one used
//int fresult=0;					//temp register
int iflabel=0;					//end if laebl
int elselabel=0;				//else label
int whilelabel=0;				//while label
int endwlabel=0;				//end while label
int caselabel=0;				//end case label
char name[30];
enum symbol obj;//ident(var) array function const
enum symbol typ;//int char void
int level=0;
int adr;
struct func *tof;
struct arrays *toa;
char tostr[20];
int isreturn; //judge if have return
enum symbol returntyp;//judge function return typ

void entertable(char name[30], enum symbol obj, enum symbol typ, int level, int adr, struct func *tof, struct arrays *toa){
	if(tablecount==tmax){
		error("table is full");
		exit(0);
	}
	struct table* p;
	p=(struct table*)malloc(sizeof(struct table));
	strcpy(p->name, name);
	p->obj=obj;
	p->typ=typ;
	p->level=level;
	p->adr=adr;
	p->tof=tof;
	p->toa=toa;
	tables[tablecount++] = p;
}
int search(char name[30]){
	for(int i=tablecount-1; i>=0; i--){
		if(strcmp(name, tables[i]->name)==0 && tables[i]->level<=level){
			return i;
		}
	}
	return -1;
}

bool del(){
	for(int i=0; i<tablecount;i++){
		if(tables[i]->level>level){
			tablecount = i;
			return true;
		}
	}
}

int searchreg(int reg1, int reg2){
	if(reg1!=-1) sreg[reg1]=0;
	if(reg2!=-1) sreg[reg2]=0;
	for(int i=0; i<scmax;i++){
		if(sreg[i]==0){
			sreg[i]=1;
			return i;
		}
	}
	return -1;
}

// the whole program
void program(){
	int judge=0;
	if(sym==constsy){
		constdeclare(); //const
	}
l1:	if(sym==charsy || sym==intsy){
		typ = sym;
		getsym();// ident
		if(sym!=ident){
			error("lack an identify");
			stop[0] = semicolon;
			stop[1] = comma;
			stop[2] = lbrack;
			stop[3] = lparen;
			stop[4] = lbrace;
			stop[5] = ident;
			int njudge = skip(6);
			if(njudge==-1){//; , [ ( { ident nextline
				goto l1;
			}
			if(njudge==5) getsym();
		}else{
			strcpy(name, id);
			getsym();
		}
		if(sym==semicolon || sym==comma || sym==lbrack){ //; , [
			obj = ident;
			pre=1;
			vardeclare();//var
		}
		if(sym==lparen || sym==lbrace){ //( {
			obj = funcsy;
			pre=1;
			rfuncdeclare();//return function
		}
	}
	//fucntions
	while(sym==voidsy || sym==intsy || sym==charsy){
		obj = funcsy;
		typ = sym;
		if(sym==voidsy){
			getsym();
			if(sym==ident){
				voidfuncdeclare();//void function
			}else if(sym==mainsy){
				mainfunc();//main function
				judge = 1;
				break;
			}else {
				voidfuncdeclare();//void function
			}
		}else{
			getsym();// ident
			rfuncdeclare();//return function
		}
	}
	if(judge==0) error("no main function"); 
}

void constdeclare(){
	int rnumber=0;
	char chars;
l2:	while(sym==constsy){
		obj = sym;
		//start const define
		getsym();
		if(sym==intsy){// int ot char
			typ = sym;
			do{
				getsym();
				if(sym!=ident){ // idnet
					error("lack an identify");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = constsy;
					stop[3] = ident;
					int njudge = skip(4);//, ; const ident nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==3) getsym();
					if(njudge==-1 || njudge==2) goto l2;
				}else{
					strcpy(name, id);
					getsym();
				}
				if(sym!=becomes){// =
					error("lack \'=\'");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = constsy;
					stop[3] = becomes;
					int njudge = skip(4);//, ; =  const nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==3) getsym();
					if(njudge==-1 || njudge==2) goto l2;
				}else getsym();// integer
				rnumber = integer();
				int result = search(name);
				if(result>=0 && tables[result]->level==level){
					string ss = name;
					ss += " redefinition";
					error(ss);
				}else{
					tof=NULL;
					toa=NULL;
					entertable(name, obj, typ, level, rnumber, tof,toa);
					ss = "const int ";
					ss += name; 
					ss +=" = " ;
					sprintf(tostr,"%d",rnumber);
					ss +=tostr;
					outputfile1(ss);
				}
			}while(sym==comma);
		}else if(sym==charsy){
			typ = sym;
			do{
				getsym();
				if(sym!=ident){ // idnet
					error("lack an identify");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = constsy;
					stop[3] = ident;
					int njudge = skip(4);//, ; const ident nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==3) getsym();
					if(njudge==-1 || njudge==2) goto l2;
				}else{
					strcpy(name, id);
					getsym();
				}
				if(sym!=becomes){// =
					error("lack '='");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = constsy;
					stop[3] = becomes;
					int njudge = skip( 4);//, ; =  const nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==3) getsym();
					if(njudge==-1 || njudge==2) goto l2;
				}else getsym();// character
				if(sym!=charcon){
					error("not a char");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = constsy;
					stop[3] = charsy;
					int njudge = skip(4);//, ; char const nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==3) getsym();
					if(njudge==-1 || njudge==2) goto l2;
				}else {
					chars = id[0];
					getsym();
				}
				int result = search(name);
				if(result>=0 && tables[result]->level==level){
					string ss = name;
					ss += " redefinition";
					error(ss);
				}else{
					tof=NULL;
					toa=NULL;
					entertable(name, obj, typ, level, rnumber, tof,toa);
					ss = "const char ";
					ss += name;
					ss +=" = ";
					ss +="\'";
					ss += chars;
					ss +="\'";
					outputfile1(ss);
				}
			}while(sym==comma);
		}else{
			error("lack int or char");
			stop[0] = semicolon;
			stop[1] = constsy;
			int njudge = skip(2);//; const nextline
			if(njudge==-1 || njudge==1) goto l2;
		}
		//end const define
		if(sym!=semicolon){
			error("lack a semicolon");
			stop[0] = constsy;
			skip(1);//const nextline
		}else getsym();
		//cout << "this is a const" <<endl;
	}
}
void vardeclare(){
	int judge = 0;
	do{
		//begin var define
		do{
			obj = ident;
			if(pre){
				pre = 0;
			}else{
				getsym();
				if(sym!=ident){
					error("lack an identify");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = intsy;
					stop[3] = charsy;
					stop[4] = ident;
					int njudge = skip( 5);//, ; int char ident nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==4) getsym();
					if(njudge==-1 || njudge==2 || njudge==3) goto l3;
				}else {
					strcpy(name, id);
					getsym();
				}
			}
			toa = NULL;
			if(sym==lbrack){//[
				obj = arraysy;
				toa = (struct arrays*)malloc(sizeof(struct arrays));
				toa->low=0;
				getsym();
				if(sym!=intcon){
					error("not a integer");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = intsy;
					stop[3] = charsy;
					stop[4] = intcon;
					int njudge = skip( 5);//, ; int char intcon nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==4) getsym();
					if(njudge==-1 || njudge==2 || njudge==3) goto l3;
				}else{
					toa->hight=inum;
					getsym();
				}
				if(sym!=rbrack){
					error("lack \']\'");
					stop[0] = comma;
					stop[1] = semicolon;
					stop[2] = intsy;
					stop[3] = charsy;
					stop[4] = rbrack;
					int njudge = skip( 5);//, ; int char ] nextline
					if(njudge==0) continue;
					if(njudge==1) break;
					if(njudge==4) getsym();
					if(njudge==-1 || njudge==2 || njudge==3) goto l3;
				}else getsym();
			}
			int result = search(name);
				if(result>=0  && tables[result]->level==level){
					string ss = name;
					ss += " redefinition";
					error(ss);
				}else{
					adr=0;
					tof=NULL;
					entertable(name, obj, typ, level, adr, tof,toa);
					ss = "VAR ";
					ss += typ==intsy ? "int ":"char ";
					ss += name; 
					if(toa!=NULL){
						ss += " [";
						sprintf(tostr,"%d",toa->hight);
						ss +=tostr;
						ss += "]";
					}
					outputfile1(ss);
				}
		}while(sym==comma);
		//end var define
		if(sym!=semicolon) error("lack a semicolon");
		else getsym();
	//	cout << "this is a var" <<endl;
		//judge if var define
l3:		judge = 0;
		if(sym==charsy || sym==intsy){
			typ = sym;
			getsym();
			if(sym==ident){
				strcpy(name, id);
				getsym();
				if(sym==semicolon || sym==comma || sym==lbrack){//; , [
					obj = ident;
					pre=1;
					judge=1;
				}
			}
		}
	}while(judge);
}
void rfuncdeclare(){
	returntyp=typ;
	isreturn=0;
	obj = funcsy;
	tof = (struct func*)malloc(sizeof(struct func));
	tof->ptotal=0;
	if(pre){
		pre=0;
	}else{
		if(sym!=ident){
			error("lack an identify");
			stop[0] = lparen;
			stop[1] = lbrace;
			stop[2] = ident;
			int njudge = skip( 3);//(  { ident nextline
			if(njudge==2) getsym();
			if(njudge==-1) return;
		}else{
			strcpy(name, id);
			getsym();
		}
	}
	int result = search(name);
	if(result>=0  && tables[result]->level==level){
		string ss = name;
		ss += " redefinition";
		error(ss);
	}else{
		adr=0;
		toa=NULL;
		entertable(name, obj, typ, level, adr, tof,toa);
		ss = typ==intsy?"int ":"char ";
		ss += name; 
		ss +="()" ;
		outputfile1(ss);
	}
	if(sym==lparen){//(
		level++;
		getsym();	
		parameters();
		if(sym!=rparen){
			error("lack \')\'");
			stop[0] = lbrace;
			stop[1] = rparen;
			int njudge = skip( 2);// { ) nextline
			if(njudge==1) getsym();
		}else getsym();
		level --;
	}
	level ++;
	if(sym!=lbrace){
		error("lack \'{\'");
	}else getsym();
	cstatements();
	if(sym!=rbrace){
		error("lack \'}\'");
	}else getsym();
	level --;
	del();
//	cout << "this is a return function" <<endl;
	if(!isreturn) error("no return in return function");
}
void voidfuncdeclare(){
	returntyp=voidsy;
	isreturn=0;
	obj = funcsy;
	typ = voidsy;
	tof = (struct func*)malloc(sizeof(struct func));
	tof->ptotal=0;
	if(sym!=ident) {
		error("lack an identify");
		stop[0] = lparen;
		stop[1] = lbrace;
		stop[2] = ident;
		int njudge = skip( 3);//(  { ident nextline
		if(njudge==2) getsym();
		if(njudge==-1) return;
	}else{
		strcpy(name, id);
		getsym();
	}
	int result = search(name);
	if(result>=0  && tables[result]->level==level){
		string ss = name;
		ss += " redefinition";
		error(ss);
	}else{
		adr=0;
		toa=NULL;
		entertable(name, obj, typ, level, adr, tof,toa);
		ss = "void ";
		ss += name; 
		ss +="()" ;
		outputfile1(ss);
	}
	if(sym==lparen){//(
		level++;
		getsym();	
		parameters();
		if(sym!=rparen) {
			error("lack \')\'");
			stop[0] = lbrace;
			stop[1] = rparen;
			int njudge = skip( 2);// { ) nextline
			if(njudge==1) getsym();
		}else getsym();
		level--;
	}
	level++;
	if(sym!=lbrace) error("lack \'{\'");
	else getsym();
	cstatements();
	if(sym!=rbrace) error("lack \'}\'");
	else getsym();
	level--;
	del();
//	cout << "this is a void function" <<endl;
}
void mainfunc(){
	returntyp=voidsy;
	ss = "void main()";
	outputfile1(ss);
	getsym();
	if(sym!=lparen) error("lack \'(\'");
	else getsym();
	if(sym!=rparen) error("lack \')\'");//)
	else getsym();
	if(sym!=lbrace) error("lack \'{\'");//{
	else getsym();
	level++;
	cstatements();
	level--;
	del();
	if(sym!=rbrace) error("lack \'}\'");//}
//	cout << "this is a main function" <<endl;
}
void parameters(){
	obj = ident;
	int judge = 0;
	int paracounts=0;
	do{
		if(sym!=intsy && sym!=charsy){
			error("lack int or char");
			stop[0] = comma;
			stop[1] = intsy;
			stop[2] = charsy;
			int njudge = skip( 3);//, int char nextline
			if(njudge==0) goto l4;
			if(njudge==1 || njudge==2) getsym();
			if(njudge==-1) return;
		}else{
			typ = sym;
			if(tof->ptotal<pmax) tof->parameters[tof->ptotal++]=sym;
			if(paracounts==pmax) error("too much parameters");
			paracounts++;
			getsym();
		}
		if(sym!=ident){
			error("lack an identify");
			stop[0] = comma;
			stop[1] = intsy;
			stop[2] = charsy;
			stop[3] = ident;
			int njudge = skip( 4);//, int char nextline
			if(njudge==0) goto l4;
			if(njudge==1 || njudge==2) continue;
			if(njudge==3) getsym();
			if(njudge==-1) return;
		}else{
			strcpy(name, id);
			getsym();
		}
		int result = search(name);
		if(result>=0  && tables[result]->level==level){
			string ss = name;
			ss += " redefinition";
			error(ss);
		}else{
			adr=0;
			toa=NULL;
			entertable(name, obj, typ, level, adr, NULL,toa);
			ss = typ==intsy?"PARA int ":"PARA char ";
			ss += name; 
			outputfile1(ss);
		}
		//jduge a list of parameters
		judge = 0;
l4:		if(sym==comma){
			getsym();
			judge = 1;
		}
	}while((sym==intsy || sym == charsy) && judge==1);
}

//combine statements
void cstatements(){
	if(sym==constsy){
		constdeclare(); //const
	}
	if(sym==charsy || sym==intsy){
		obj = ident;
		typ = sym;
		vardeclare();
	}
	lstatements();
}
// a list of statements
void lstatements(){
	while(sym==ifsy || sym==whilesy || sym==lbrace || sym==ident || sym==scanfsy || sym==printfsy || sym==switchsy || sym==returnsy || sym==semicolon){
		statement();
	}
}
//a statement
void statement(){
	int result;
	for(int i=0;i<scmax;i++) sreg[i]=0;
	switch(sym){
			case ifsy:
				ifstatement();
				break;
			case whilesy:
				whilestatement();
				break;
			case lbrace:
				getsym();
				lstatements();
				if(sym!=rbrace) error("lack \'}\'");
				else getsym();
				break;
			case ident:
				result = search(id);
				if(result==-1){
					error("not find");
					getsym();
					return;
				}
				if(tables[result]->obj==funcsy) funcall();
				else assignstatement();
				if(sym!=semicolon) error("lack a semicolon");
				else getsym();
				break;
			case scanfsy:
				scanfstatement();
				if(sym!=semicolon) error("lack a semicolon");
				else getsym();
				break;
			case printfsy:
				printfstatement();
				if(sym!=semicolon) error("lack a semicolon");
				else getsym();
				break;
			case switchsy:
				switchstatement();
				break;
			case returnsy:
				returnstatement();
				if(sym!=semicolon) error("lack a semicolon");
				else getsym();
				break;
			case semicolon:
			//	cout << "this is null statement" <<endl;
				getsym();
				break;
			default:
				error("not a statement");
		}
}
void ifstatement(){
	enum symbol cmp=errors;
	string s1, s2;
	int elsel=elselabel;
	int endl=iflabel;
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	elselabel++;
	iflabel++;
	if(sym!=ifsy) return;
	else getsym();
	if(sym!=lparen) error("lack \'(\'");//(
	else getsym();
	//begin condition
	//fresult=0;
	s1=expression(judge, regs);
	if(9<sym && sym<16){//compare operator
		cmp = sym;
		getsym();
		s2=expression(judge, regs);
	}
	//end condition
	//condition (go to elselabel)
	if(cmp!=errors){
		ss = "BZ "+s1;
		ss += symname[cmp];
		ss += s2;
		ss += " ELSELABEL" ;
		sprintf(tostr,"%d",elsel);
		ss += tostr;
	}
	else{
		ss = "BZ "+s1;
		ss += " ELSELABEL" ;
		sprintf(tostr,"%d",elsel);
		ss += tostr;
	}
	outputfile1(ss);
	if(sym!=rparen) error("lack \')\'");//)
	else getsym();
	statement();
	//go to end if label
	sprintf(tostr,"%d",endl);
	ss = "GOTO ENDIFLABEL" ;
	ss += tostr;
	outputfile1(ss);
	if(sym!=elsesy){
		error("lack else");//else
		return;
	}
	getsym();
	//else label 
	sprintf(tostr,"%d",elsel);
	ss = "ELSELABEL";
	ss += tostr;
	ss += ":";
	outputfile1(ss);
//	cout << "this is a if statement" <<endl;
	statement();
	//end if label 
	sprintf(tostr,"%d",endl);
	ss = "ENDIFLABEL";
	ss += tostr;
	ss += ":";
	outputfile1(ss);
}
void whilestatement(){
	enum symbol cmp=errors;
	string s1, s2;
	int whilel = whilelabel;
	int endl = endwlabel;
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	whilelabel++;
	endwlabel++;
	if(sym!=whilesy) return;
	getsym();
	if(sym!=lparen) error("lack \'(\'");//(
	else getsym();
	//while label
	sprintf(tostr,"%d",whilel);
	ss = "WLABEL" ;
	ss += tostr;
	ss += ":";
	outputfile1(ss);
	//begin condition
	//fresult=0;
	s1=expression(judge, regs);
	if(9<sym && sym<16){//compare operator
		cmp = sym;
		getsym();
		s2=expression(judge,regs);
	}
	//end condition
	
	//condition (goto end while label)
	if(cmp!=errors){
		ss = "BZ "+s1;
		ss += symname[cmp];
		ss += s2;
		ss +=" EWLABEL" ;
		sprintf(tostr,"%d",endl);
		ss += tostr;
	}
	else{
		ss = "BZ "+s1;
		ss +=" EWLABEL" ;
		sprintf(tostr,"%d",endl);
		ss += tostr;
	}
	outputfile1(ss);
	if(sym!=rparen) error("lack \')\'");//)
	else getsym();
	//cout << "this is a while statement" <<endl;
	statement();
	//goto while label
	sprintf(tostr,"%d",whilel);
	ss = "GOTO WLABEL";
	ss += tostr;
	outputfile1(ss);
	//label 2
	sprintf(tostr,"%d",endl);
	ss = "EWLABEL";
	ss += tostr;
	ss += ":";
	outputfile1(ss);
}
void funcall(){
	int reged[scmax];
	int isfind=1;
	int result;
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	string s1, s2;
	s2="";
	for(int i=0;i<scmax;i++) reged[i]=0;
	if(sym!=ident) return;
	result = search(id);
	if(result==-1){
		error("not find");
		isfind=0;
	}
	if(isfind && tables[result]->obj!=funcsy) error("not a function");
	getsym();
	if(sym==lparen){//(
		//value list
		int parascount=0;
		do{
			getsym();
			s1=expression(judge, regs);
			if((*regs)!=-1) reged[(*regs)]=1;
			s2 += "\nPUSH "+s1;
			//judge para type
			if(isfind ){
				if(!((tables[result]->tof->parameters[parascount]==intsy && (*judge)==0)||(tables[result]->tof->parameters[parascount]==charsy && (*judge==1))))
					error("wrong type parameter");
			}
			parascount++;
			//outputfile1(ss);
		}while(sym==comma);
		//empty reg
		for(int i=0;i<scmax;i++){
			if(reged[i]==1){
				sreg[i]=0;
			}
		}
		if(sym!=rparen) error("lack \')\'");//)
		else getsym();
		//judge number of paras
		if(isfind && parascount!=tables[result]->tof->ptotal) error("not match the number of parameters");
	}else{
		if(isfind && tables[result]->tof->ptotal!=0) error("not match the number of parameters");
	}
	s2 += "\nCALL ";
	s2 += (tables[result]->name);
	outputfile1(s2);
	//cout << "this is a function call statement" <<endl;
}
void assignstatement(){
	int result;
	int isfind=1;
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	string s1;
	//fresult=0;// empty
	if(sym!=ident) return;
	result = search(id);
	if(result==-1){
		isfind=0;
		error("not find");
	}
	getsym();
	s1=tables[result]->name;
	if(sym==lbrack){
		s1+="[";
		if(isfind && tables[result]->obj!=arraysy) error("not an array");
		getsym();
		s1+=expression(judge, regs);
		if(sym!=rbrack) error("lack \']\'");
		else getsym();
		s1+="]";
	}else{
		if(isfind && tables[result]->obj!=ident) error("not a var");
	}
	if(sym!=becomes) error("lack \'=\'");
	else getsym();
	s1 += "=";
	s1+=expression(judge,regs);
	//judge type
	if(isfind){
		if(tables[result]->typ==charsy && (*judge)==0) error("int can not to char");
	}
	ss=s1;
	outputfile1(ss);
	//cout << "this is an assign statement" <<endl;
}
void scanfstatement(){
	int result;
	ss = "scanf ";
	if(sym!=scanfsy) return;
	getsym();
	if(sym!=lparen) error("lack \'(\'");//(
	else getsym();
	if(sym!=ident){ 
		error("lack an identify");
		stop[0] = comma;
		stop[1] = semicolon;
		stop[2] = rparen;
		stop[3] = ident;
		int njudge = skip( 4);//, ; )  ident nextline
		if(njudge==3) getsym();
		if(njudge==-1 || njudge==1) return;
	}else{
		result=search(id);
		if(result==-1) error("not find");
		if(result!=-1 && tables[result]->obj!=ident) error("not legal var");
		if(result!=-1) outputfile1(ss+tables[result]->name);
		getsym();
	}
	while(sym==comma){
		getsym();
		if(sym!=ident){
			error("lack an identify");
			stop[0] = comma;
			stop[1] = semicolon;
			stop[2] = rparen;
			stop[3] = ident;
			int njudge = skip( 4);//, ; )  ident nextline
			if(njudge==3) getsym();
			if(njudge==-1 || njudge==1) return;
		}else {
			result=search(id);
			if(result==-1) error("not find");
			if(result!=-1 && tables[result]->obj!=ident) error("not legal var");
			if(result!=-1) outputfile1(ss+tables[result]->name);
			getsym();
		}
	}
	if(sym!=rparen) error("lack \')\'");
	else getsym();
	//cout << "this is a scanf statement" <<endl;
}
void printfstatement(){
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	string s2 = "printf ";
	string s1;
	if(sym!=printfsy) return;
	getsym();
	if(sym!=lparen) error("lack \'(\'");//(
	else getsym();
	if(sym==stringcon){
		if(strss[0]!='\0')
			outputfile1(s2+"\""+strss+"\"");
		getsym();
		if(sym==comma){
			getsym();
		//	fresult=0;//empty
			s1=expression(judge,regs);
			outputfile1(s2+s1);
		}
		if(sym!=rparen) error("lack \')\'");
		else getsym();
	}else{
		//fresult=0;//empty
		s1=expression(judge,regs);
		outputfile1(s2+s1);
		if(sym!=rparen) error("lack \')\'");
		else getsym();
	}
	//cout << "this is a printf statement" <<endl;
}
void switchstatement(){
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	string s1;
	int endl = caselabel;
	caselabel++;
	int lab;
	if(sym!=switchsy) return;
	getsym();
	if(sym!=lparen) error("lack \'(\'");//(
	else getsym();
	//fresult=0;//empty
	s1=expression(judge,regs);
	if(sym!=rparen) error("lack \')\'");//)
	else getsym();
	if(sym!=lbrace) error("lack \'{\'");//{
	else getsym();
//	cout << "this is a switch statement" <<endl;
	//ss = "case "+s1;
	//outputfile1(ss);
	lab=switchtable(endl, s1);
	//label
	ss = "LABEL";
	sprintf(tostr,"%d",lab);
	ss += tostr;
	ss += "c";
	sprintf(tostr,"%d",endl);
	ss += tostr;
	ss += ":";
	outputfile1(ss);
	if(sym==defaultsy){
		defaultstatement();
	}
	if(sym!=rbrace) error("lack \'}\'");//}
	else getsym();
	//end caselabel
	sprintf(tostr,"%d",endl);
	ss = "CLABEL" ;
	ss += tostr;
	ss += ":";
	outputfile1(ss);
}
void returnstatement(){
	isreturn=1;//judge return
	int addr,reg1;
	int *judge, *regs;
	judge=&addr;
	regs=&reg1;
	string s1;
	//fresult=0;//empty
	if(sym!=returnsy) return;
	getsym();
	if(sym==lparen){
		getsym();
		s1=expression(judge,regs);
		//judge type
		if(!((returntyp==intsy&&(*judge)==0)||(returntyp==charsy&&(*judge)==1))) error("return type wrong");
		if(sym!=rparen) error("lack \')\'");
		else getsym();
		ss = "RET "+s1;
	}else{
		if(returntyp!=voidsy) error("return type wrong");
		ss = "RET";
	}
	outputfile1(ss);
//	cout << "this is a return statement" <<endl;
}

string expression(int *judge, int *regs){
	string s1="", s2, s3,sresult;
	int neg=0, addr1, addr2, regs1=-1,regs2=-1;
	char op2;
	int *judge1, *judge2, *reg1, *reg2;
	judge1=&addr1;
	judge2=&addr2;
	reg1=&regs1;
	reg2=&regs2;
	if(sym==plus || sym==minus){
		if(sym==minus){
			neg=1;
			s1="-";
		}
		getsym();
	}
	s3=term(judge1,reg1);
	(*judge)=(*judge1);
	(*regs)=(*reg1);
	if(s3.data()[0]=='-'&&neg){
		s1="";
		for(int i=1;s3.data()[i]!='\0';i++){
			s1+=s3.data()[i];
		}
	}
	else s1+=s3;
	sresult=s1;
	while(sym==plus || sym==minus){
		op2=sym==plus?'+':'-';
		getsym();
		s2=term(judge2,reg2);
		(*regs)=searchreg((*reg1),(*reg2));
		sprintf(tostr,"%d",(*regs));
		sresult=((*judge1)&&(*judge2))?"$$":"$";
		(*judge)=((*judge1)&&(*judge2))?1:0;
		sresult += tostr;
		outputfile1(sresult+"="+s1 + op2 + s2);
		//update s1
		s1=sresult;
		(*judge1)=(*judge);
		(*reg1)=(*regs);
	}
	return sresult;
}
string term(int *judge,int *regs){
	string s1, s2, sresult;
	char op;
	int addr1, addr2, regs1=-1,regs2=-1;
	int *judge1, *judge2, *reg1, *reg2;
	judge1=&addr1;
	judge2=&addr2;
	reg1=&regs1;
	reg2=&regs2;
	s1 = factor(judge1,reg1);
	sresult = s1;
	(*judge)=(*judge1);
	(*regs)=(*reg1);
	while(sym==divsy || sym==times){
		op=sym==divsy?'/':'*';
		getsym();
		s2=factor(judge2,reg2);
		(*regs)=searchreg((*reg1),(*reg2));
		sprintf(tostr,"%d",(*regs));
		sresult=((*judge1)&&(*judge2))?"$$":"$";//judge char $,$$,ident,  s1,s2
		(*judge)=((*judge1)&&(*judge2))?1:0;
		sresult += tostr;
		outputfile1(sresult+"="+s1 + op + s2);
		//update s1
		s1=sresult;
		(*judge1)=(*judge);
		(*reg1)=(*regs);
	}
	return sresult;
}
string factor(int *judge,int *regs){
	int result, addr1,regs1=-1;
	int *judge1,*reg1;
	judge1=&addr1;
	reg1=&regs1;
	(*regs)=-1;
	string sresult;
	if(sym==ident){//＜标识符＞｜＜标识符＞‘[’＜表达式＞‘]’  ＜标识符＞‘(’＜值参数表＞‘)’|<标识符> 
		result = search(id);
		if(result==-1){
			error("not find");
			getsym();
			return "";
		}
		(*judge)=tables[result]->typ==charsy?1:0;
		if(result!=-1&&tables[result]->obj==funcsy){//function
			if(tables[result]->typ==intsy || tables[result]->typ==charsy) funcall();
			else error("not return function");
			(*regs)=searchreg(-1,-1);
			sprintf(tostr,"%d",(*regs));
			ss =tables[result]->typ==charsy?"$$":"$";
			ss +=tostr;
			sresult =tables[result]->typ==charsy?"$$": "$";
			sresult +=tostr;
			ss += "=RET";
			outputfile1(ss);
			//sresult =tables[result]->typ==intsy?"RET$1":"RET$0";
			return sresult;
		}
		getsym();
		sresult = tables[result]->name;
		if(sym==lbrack){//arrray
			sresult += "[";
			if(result!=-1&&tables[result]->obj!=arraysy) error("not an array");
			getsym();
			string str = expression(judge1, reg1);
			string mstr =(*judge1)?"$$": "$";
			sresult += (*judge1)?"$$":"$";
			(*regs)=searchreg((*reg1),-1);
			sprintf(tostr,"%d",(*regs));
			mstr += tostr;
			sresult += tostr;
			mstr += "=";
			mstr += str;
			outputfile1(mstr);

			//sresult += str;
			if(sym!=rbrack) error("lack \']\'");
			else getsym();
			sresult += "]";
		}
	}else if(sym==lparen){//‘(’＜表达式＞‘)’
		getsym();
		sresult=expression(judge1, reg1);
		(*judge)=(*judge1);
		(*regs)=(*reg1);
		if(sym!=rparen) error("lack \')\'");//)
		else getsym();
	}else if(sym==charcon){//＜字符＞
		(*judge)=1;
		sresult = "\'";
		sresult += id[0];
		sresult += "\'";
		getsym();
	}else{//＜整数＞
		(*judge)=0;
		int rr=integer();
		sprintf(tostr,"%d",rr);
		sresult=tostr;
	}
	return sresult;
}

int switchtable(int endl, string s1){
	int lab=0;
	do{
		//label
		ss = "LABEL";
		sprintf(tostr,"%d",lab);
		ss += tostr;
		ss += "c";
		sprintf(tostr,"%d",endl);
		ss += tostr;
		ss += ":";
		outputfile1(ss);
		lab++;
		casestatement(lab, endl, s1);
		// goto end caselabel
		sprintf(tostr,"%d",endl);
		ss = "GOTO CLABEL";
		ss +=tostr;
		//cout << tostr <<endl;
		outputfile1(ss);
	}while(sym==casesy);
	return lab;
}
void casestatement(int lab, int endl, string s1){
	int judge=0;
	char c;
	int inte;
	if(sym!=casesy) return;
	getsym();
	//conststatement
	if(sym==charcon){
		c=id[0];
		getsym();
	}else{
		judge=1;
		inte=integer();
	}
	//end conststatement
	if(sym!=colon) error("lack \':\'");//:
	else getsym();
	//cout << "this is a case statement" <<endl;
	//condition (goto next label)
	if(judge){
		sprintf(tostr,"%d",inte);
		ss = "BZ ";
		ss += s1;
		ss += "==";
		ss += tostr;
		ss +=" LABEL" ;
		sprintf(tostr,"%d",lab);
		ss +=tostr;
		ss +="c";
		sprintf(tostr,"%d",endl);
		ss +=tostr;
	}
	else{
		ss = "BZ ";
		ss += s1;
		ss += "==\'";
		ss += c;
		ss +="\' LABEL";
		sprintf(tostr,"%d",lab);
		ss +=tostr;
		ss +="c";
		sprintf(tostr,"%d",endl);
		ss +=tostr;
	}
	outputfile1(ss);
	statement();
}
void defaultstatement(){
	if(sym!=defaultsy) return;
	getsym();
	if(sym!=colon) error("lack \':\'");//:
	else getsym();
	//cout << "this is a default statement" <<endl;
	statement();
}

int integer(){
	int number=0;
	int op=0;
	if(sym==plus || sym==minus){
		op= sym==plus?0:1;
		getsym();
	}
	if(sym!=intcon) error("not a integer");
	else{
		if(op) number = 0-inum;
		else number = inum;
		getsym();
	}
	return number;
}