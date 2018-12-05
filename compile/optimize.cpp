#include"optimize.h"
#include"code.h"
#include"const.h"
#include<iostream>
#include<fstream>
#include<string>
#include<sstream>
using namespace std;

extern char line[llng];			//save one line of code
extern char id[30];				//identifier form getsym
extern int cc;						//character count
extern char ch;					//last character read
extern enum m_symbol m_sym;
extern char strs[strmax];
extern char tostr[20];
extern ifstream mcodef;
extern ofstream codef;
struct block* blocks[blockmax]; //blocks
int blockcount=0;
struct labelbk * lblocks[100]; //label block
int lblockcount=0;
int linenum=0;				//count line of text
struct block* nowblock;	//present block
//table
struct labelbk* charident[tmax];	//char ident table
int chidentnum=0;			//count char ident table
extern int level;

int tops[blockmax][dagmax]; //no parents dag node
int dagnum[blockmax];			// the no of dag node
struct dag* dags[blockmax][dagmax];	//total number of dag node
struct labelbk* dagtable[blockmax][dagtbmax];//dag table
int dagtbcount[blockmax];	

//conflict graph
int cgraphcount = 0;
struct dag * cgraph[tmax];

int pushs[pmax];		//all paras of a function
int parcount=0;			//number of paras
int rfuncall=-1;
int usedreg[scmax];			//1-which one used
int lastreg;			//the last reg used for switch
//table
void opt_entertable(int ischar, int level, char name[30], int isvar){
	struct labelbk* p;
	p=(struct labelbk*)malloc(sizeof(struct labelbk));
	p->judge=ischar;
	p->blocknum=level;
	strcpy(p->lname,name);
	p->isvar=isvar;
	charident[chidentnum++]=p;
}
int opt_searchtable(char name[30]){
	for(int i=chidentnum-1; i>=0; i--){
		if(strcmp(name, charident[i]->lname)==0 &&  charident[i]->blocknum<=level){
			return i;
		}
	}
	return -1;
}
void opt_deltable(){
	struct labelbk* p;
	for(int i=chidentnum-1; i>=0;i--){
		if(charident[i]->blocknum<=level){
			chidentnum = i+1;
			break;
		}else{
			p=charident[i];
			free(p);
		}
	}
}

void outputfile3(string s) {
	if (!codef) {
		cout << "mips_code.txt can't open" << endl;
		return;
		
	}
	codef << s;
	codef << endl;
	//opt_f.close();
}

void inlabel(int num, char name[30]){
	struct labelbk *p;
	p=(struct labelbk *)malloc(sizeof(struct labelbk ));
	strcpy(p->lname,name);
	p->blocknum=num;
	lblocks[lblockcount++]=p;
}
int labelsearch(char name[30]){
	for(int i=0;i<lblockcount;i++){
		if(strcmp(name, lblocks[i]->lname)==0) return i;
	}
	return -1;
}

int opt_nextline(){
	do{
		if(mcodef.eof()){
			mcodef.close();
			return -1;
		}
		mcodef.getline(line, sizeof(line));
		cc=0;
		if(line[0]!='\0') linenum++;
	}while(line[0]=='\0');
	ch=' ';
	return 0;
}

void optmain(){
	int charjudge=0, varjudge=0, endjudge=0;
	char name[30];
	m_setup();
	mcodef.open("code.txt");
	if (!mcodef) {
		cout << "code can't open" << endl;
		return;
	}
	//opt_f.open("opt_code.txt", ios::out);
	//each function
	opt_nextline();
	m_getsym();
	level=0;
	while(m_sym==mconstsy||m_sym==mvarsy){
		charjudge=0;
		varjudge=0;
		if(m_sym==mvarsy) varjudge=1;
		m_getsym();
		if(m_sym==mcharsy) charjudge=1;
		m_getsym();
		strcpy(name, id);
		if(ch!='\0') varjudge=0;
		opt_entertable(charjudge, level, name, varjudge);
		outputfile3(line);
		opt_nextline();
		m_getsym();
	}
	if(m_sym==mintsy||m_sym==mcharsy||m_sym==mvoidsy){
		do{
			level++;
			blockcount=0;
			lblockcount=0;	
			if(toblock()==-1) endjudge=1;
			regopt();
			printtof();
			del();
			level--;
			opt_deltable();
			if(endjudge) break;
		}while(m_sym==mintsy||m_sym==mcharsy||m_sym==mvoidsy);
	}
	
}

int toblock(){//first statement, label, after bz/goto/ret
	int isend=0;
	int judge=0;
	int bcount=0;
	struct block* p;
	outputfile3(line);
	if(opt_nextline()==-1) return -1;
	m_getsym();
	//para
	while(m_sym==mparasy){
		outputfile3(line);
		m_getsym();
		if(m_sym==mcharsy){
			m_getsym();
			opt_entertable(1, level, id, 1);
		}else{
			m_getsym();
			opt_entertable(0, level, id, 1);
		}
		if(opt_nextline()==-1) return -1;
		m_getsym();
	}
	//const var
	while(m_sym==mconstsy||m_sym==mvarsy){
		outputfile3(line);
		m_getsym();
		if(m_sym==mcharsy){
			m_getsym();
			if(ch!='\0') opt_entertable(1, level, id,0);
			else opt_entertable(1, level, id,1);
		}else{
			m_getsym();
			if(ch!='\0') opt_entertable(0, level, id,0);
			else opt_entertable(0, level, id,1);
		}
		if(opt_nextline()==-1) return -1;
		m_getsym();
	}
	//statements bz/goto/ret/label  push/call/scanf/printf  opt(array)  +- ret/$/ident/char/int
	if(!(m_sym==mintsy||m_sym==mcharsy||m_sym==mvoidsy)){
		char last[30];
		last[0]='\0';
		bcount++;
		p = (struct block*)malloc(sizeof(struct block));
		nowblock = p;
		p->start=linenum;
		p->num=bcount;
		p->next=0;
		p->nextnum=0;
		p->innum=0;
		p->outnum=0;
		p->usenum=0;
		p->defnum=0;
		p->judge=0;
		//initate block
		dagnum[blockcount]=0;
		dagtbcount[blockcount]=0;
		for(int i=0;i<scmax;i++) usedreg[i]=0;	
		//end initate
		while(!(m_sym==mintsy||m_sym==mcharsy||m_sym==mvoidsy)){
			judge=0;
			if((m_sym== mident&&id[0]>='A' && id[0]<='Z')||m_sym==mbzsy||m_sym==mgotosy||m_sym==mretsy){//bz/goto/ret/label
				if(m_sym== mident&&id[0]>='A' && id[0]<='Z'){//label //
					//outputfile3(line);
					//p->judge=1;
					//label
					//strcpy(p->label,id);
					strcpy(last,id);
				//	inlabel(bcount+1, id);
					//block
					judge=1;
					p->end=linenum-1;
					p->nextnum=1;
					p->next=bcount+1;
				}else{
					p->end=linenum;
					if(m_sym==mbzsy){//bz
						setdag();
						p->nextnum=3;
						p->next=bcount+1;
						while(!(m_sym== mident&&id[0]>='A' && id[0]<='Z')) m_getsym();
						strcpy(p->label,id);
					}else if(m_sym==mgotosy){//goto //
						//last=line;
						if(p->judge==1) p->judge=3;
						else p->judge=2;
						m_getsym();
						p->nextnum=2;
						strcpy(p->label,id);
					}else{//ret
						setdag();
						p->nextnum=1;
						p->next=-1;
					}
				}
				if(p->end>=p->start){
					blocks[blockcount++] = p;
					//next
				//	blockopt();//opt print
					bcount++;
					p = (struct block*)malloc(sizeof(struct block));
					nowblock = p;
					if(judge){
						p->start=linenum;
					}else{
						p->start=linenum+1;
					}
					p->num=bcount;
					p->next=0;
					p->nextnum=0;
					p->innum=0;
					p->outnum=0;
					p->usenum=0;
					p->defnum=0;
					p->judge=0;
					if(last[0]!='\0'){
						inlabel(bcount, last);
						p->judge=1;
						strcpy(p->label1,last);
						last[0]='\0';
					}
					//initate block
					dagnum[blockcount]=0;
					dagtbcount[blockcount]=0;
					for(int i=0;i<scmax;i++) usedreg[i]=0;	
					//end initate
				}else {
					if(last[0]!='\0'){
						inlabel(bcount, last);
						p->judge=1;
						strcpy(p->label1,last);
						last[0]='\0';
					}
				}
			}else {//push/call/scanf/printf  opt
				judge=1;
				setdag();
			}
			if(opt_nextline()==-1){
				isend=-1;
				break;
			}
			m_getsym();
		}
		if(judge){// last statement label/assign
			p->end=linenum;
			p->nextnum=1;
			p->next=-1;
			if(p->end>=p->start){
				blocks[blockcount++] = p;
				//blockopt();//opt print
				if(last[0]!='\0'){
					inlabel(bcount, last);
					p->judge=1;
					//label
					strcpy(p->label1,last);
					last[0]='\0';
				}
			}
		}
	}
	return isend;
}

int searchdagtable(char name[30]){
	for(int i=dagtbcount[blockcount]-1;i>=0;i--){
		if(strcmp(dagtable[blockcount][i]->lname,name)==0) return dagtable[blockcount][i]->blocknum;
	}
	return -1;
}
void indagtable(int num, char lname[30]){
	struct labelbk*p;
	p=(struct labelbk*)malloc(sizeof(struct labelbk));
	p->judge=0;
	p->blocknum=num;
	strcpy(p->lname,lname);
	dagtable[blockcount][dagtbcount[blockcount]++]=p;
}
void indagtable(int num, char lname[30],char s1[llng]){//print string
	struct labelbk*p;
	p=(struct labelbk*)malloc(sizeof(struct labelbk));
	p->judge=1;
	p->blocknum=num;
	strcpy(p->lname,lname);
	strcpy(p->s1, s1);
	dagtable[blockcount][dagtbcount[blockcount]++]=p;
}

int searchdag(int i, int j, enum m_symbol op, enum m_symbol cmp, char name[30]){
	//+-*/  bz(label)/ret/push/call(label)/scanf/printf  array(=right:[]  =left:[]=)
	struct dag* node, *node1;
	if(j==-1){
		for(int k=0;k<dags[blockcount][i]->pnum;k++){
			node=dags[blockcount][i]->parents[k];
			if(node->judge==1 && node->op==op){
				if(op==mbzsy){
					if(node->cmp==cmp && strcmp(name,node->name)==0) return node->num;
				}else{
					return node->num;
				}
			}
		}
	}else if(i==-1){
		for(int k=0;k<dags[blockcount][j]->pnum;k++){
			node=dags[blockcount][j]->parents[k];
			if(node->judge==1 && node->op==op){
				if(op==mbzsy){
					if(node->cmp==cmp && strcmp(name,node->name)==0) return node->num;
				}else{
					return node->num;
				}
			}
		}
	}else{
		for(int k=0;k<dags[blockcount][i]->pnum;k++){
			for(int n=0;n<dags[blockcount][j]->pnum;n++){
				node=dags[blockcount][i]->parents[k];
				node1=dags[blockcount][j]->parents[n];
				if(node->num==node1->num && node->judge==1 && node->op==op &&((node->lchild->num==i && node->rchild->num==j)||(node->lchild->num==j && node->rchild->num==i))){
					if(op==mbzsy){
						if(node->cmp==cmp && strcmp(name,node->name)==0) return node->num;
					}else{
						return node->num;
					}
				}
			}
		}
	}
	return -1;
}
int indag(char name[30]){//ident
		//new node
		struct dag* p;
		p=(struct dag*)malloc(sizeof(struct dag));
		p->judge=0;
		p->num=dagnum[blockcount];
		strcpy(p->name,name);
		p->cmp=mident;
		p->op=mident;
		p->lchild=NULL;
		p->rchild=NULL;
		p->pnum=0;
		p->cnum=0;
		p->ischar=0;
		dags[blockcount][dagnum[blockcount]]=p;
		tops[blockcount][dagnum[blockcount]]=1;
		indagtable(dagnum[blockcount], name);
		dagnum[blockcount]++;
		return dagnum[blockcount]-1;
}
int indag(char name[30], enum m_symbol cmp, struct dag * lchild, struct dag * rchild){//op bz
		struct dag* p;
		p=(struct dag*)malloc(sizeof(struct dag));
		p->judge=1;
		p->num=dagnum[blockcount];
		strcpy(p->name,name);
		p->cmp=cmp;
		p->op=mbzsy;
		p->lchild=lchild;
		p->rchild=rchild;
		p->pnum=0;
		p->cnum=0;
		p->ischar=0;
		dags[blockcount][dagnum[blockcount]]=p;
		tops[blockcount][dagnum[blockcount]]=1;
		//indagtable(dagnum, name);
		dagnum[blockcount]++;
		//children
		if(lchild!=NULL){
			lchild->parents[lchild->pnum++]=p;
			tops[blockcount][lchild->num]=0;
		}
		if(rchild!=NULL){
			rchild->parents[rchild->pnum++]=p;
			tops[blockcount][rchild->num]=0;
		}
		return dagnum[blockcount]-1;
}
int indag(char name[30], enum m_symbol op){//op scanf
		struct dag* p;
		p=(struct dag*)malloc(sizeof(struct dag));
		p->judge=1;
		p->num=dagnum[blockcount];
		strcpy(p->name,name);
		p->cmp=mident;
		p->op=op;
		p->lchild=NULL;
		p->rchild=NULL;
		p->pnum=0;
		p->cnum=0;
		p->ischar=0;
		dags[blockcount][dagnum[blockcount]]=p;
		tops[blockcount][dagnum[blockcount]]=1;
		dagnum[blockcount]++;
		return dagnum[blockcount]-1;
}
int indag(struct dag* p){//op call
		p->judge=1;
		p->num=dagnum[blockcount];
		//strcpy(p->name,name);
		p->cmp=mident;
		p->op=mcallsy;
		//p->lchild=lchild;
	//	p->rchild=rchild;
		p->pnum=0;
		p->ischar=0;
		//p->cnum=cnum;
	//	p->children=children;
		dags[blockcount][dagnum[blockcount]]=p;
		tops[blockcount][dagnum[blockcount]]=1;
		//indagtable(dagnum, name);
		dagnum[blockcount]++;
		//children
		if(p->lchild!=NULL){
			p->lchild->parents[p->lchild->pnum++]=p;
			tops[blockcount][p->lchild->num]=0;
		}
		if(p->rchild!=NULL){
			p->rchild->parents[p->rchild->pnum++]=p;
			tops[blockcount][p->rchild->num]=0;
		}
		if(p->cnum>2){
			for(int i=0;i<p->cnum-2;i++){
				p->children[i]->parents[p->children[i]->pnum++]=p;
				tops[blockcount][p->children[i]->num]=0;
			}
		}
		return dagnum[blockcount]-1;
}
int indag(enum m_symbol op, struct dag * lchild, struct dag * rchild, int ischar){//op +-*/ push/printf(string)  array(=right:[]  =left:[]=) 
		struct dag* p;
		p=(struct dag*)malloc(sizeof(struct dag));
		p->judge=1;
		p->num=dagnum[blockcount];
		p->cmp=mident;
		p->op=op;
		p->lchild=lchild;
		p->rchild=rchild;
		p->pnum=0;
		p->cnum=0;
		p->ischar=ischar;
		dags[blockcount][dagnum[blockcount]]=p;
		tops[blockcount][dagnum[blockcount]]=1;
		//indagtable(dagnum, name);
		dagnum[blockcount]++;
		//children
		if(lchild!=NULL){
			lchild->parents[lchild->pnum++]=p;
			tops[blockcount][lchild->num]=0;
		}
		if(rchild!=NULL){
			rchild->parents[rchild->pnum++]=p;
			tops[blockcount][rchild->num]=0;
		}
		return dagnum[blockcount]-1;
}

void setdag(){//bz/ret push/call/scanf/printf  opt(array)  +- ret/$/ident/char/int
	switch(m_sym){
		case mbzsy:
			opt_bzstatement();
			break;
		case mident:case mdollar:
			opt_opeartors(); 
			break;
		case mpushsy:
			opt_push();
			break;
		case mcallsy:
			opt_callstatement();
			break;
		case mretsy:
			opt_ret();
			break;
		case mscanfsy:
			opt_scan();
			break;
		case mprintfsy:
			opt_print();
			break;
		default:
		cout<<"error"<<endl;
	}
}
int opt_exps(char name[30], int judge){//- ret/$/ident/char/int
	int ischar=0;
	int namecount=0;
	if(m_sym==mplus||m_sym==mminus){
			if(m_sym==mminus) name[namecount++]='-';
			m_getsym();
		}
		if(m_sym==mintcon){
			for(int i=0;id[i]!='\0';i++) name[namecount++]=id[i];
			name[namecount] = '\0';
		}else if(m_sym==mcharcon){//-????????????????;
			ischar=1;
			name[namecount++]='\'';
			name[namecount++]=id[0];
			name[namecount++]='\'';
			name[namecount++]='\0';
		}else if(m_sym==mident){//ident const array
			int result=opt_searchtable(id);
			if(charident[result]->judge) ischar=1;
			for(int i=0;id[i]!='\0';i++) name[namecount++]=id[i];
			name[namecount] = '\0';
			if(judge && charident[result]->isvar){
				strcpy(nowblock->use[nowblock->usenum++], id);
			}
			//array
		}else if(m_sym==mretsy){
			name[namecount++]='R';
			name[namecount++]='E';
			name[namecount++]='T';
			name[namecount] = '\0';
		}else{//$
			m_getsym();
			if(m_sym==mdollar){
				name[namecount++]='$';
				ischar=1;
			}
			name[namecount++]='$';
			for(int i=0;id[i]!='\0';i++) name[namecount++]=id[i];
			name[namecount] = '\0';
		}
	return ischar;
}
void opt_bzstatement(){
	char name1[30], name2[30], aname[30];
	enum m_symbol op=mident;
	int result, tr1=-1, tr2=-1, tr3;
	//ident1
	m_getsym();
	result=opt_exps(name1, 1);
	tr1=searchdagtable(name1);
	if(tr1==-1){// new node
		tr1=indag(name1);
	}
	m_getsym();
	
		//array
		if(m_sym==mlbrack){//[
			m_getsym();
			opt_exps(aname, 1);
			tr2=searchdagtable(aname);
			if(tr2==-1){// new node
				tr2=indag(aname);
			}
			m_getsym();//]
			m_getsym();
			int rr=searchdag(tr1,tr2,mrarray,mident," ");
			if(rr==-1){
				tr1=indag(mrarray,dags[blockcount][tr1],dags[blockcount][tr2], result);
			}else {
				tr1=rr;
			}
		}
		tr2=-1;
	
	//op
	if(8<=m_sym&&m_sym<=13){
		op=m_sym;
		//ident2
		m_getsym();
		result=opt_exps(name2, 1);
		tr2=searchdagtable(name2);
		if(tr2==-1){// new node
			tr2=indag(name2);
		}
		m_getsym();
		
			//array
			if(m_sym==mlbrack){//[
				m_getsym();
				opt_exps(aname, 1);
				tr3=searchdagtable(aname);
				if(tr3==-1){// new node
					tr3=indag(aname);
				}
				m_getsym();//]
				int rr=searchdag(tr2,tr3,mrarray,mident," ");
				if(rr==-1){
					tr2=indag(mrarray,dags[blockcount][tr2],dags[blockcount][tr3],result);
				}else {
					tr2=rr;
				}
				m_getsym();//laebl
			}
		
		//laebl
		//m_getsym();
	}
	if(tr2!=-1) indag(id,op,dags[blockcount][tr1],dags[blockcount][tr2]);
	else indag(id,op,dags[blockcount][tr1],NULL);
}
void opt_opeartors(){
	enum m_symbol op;
	char name[30],aname[30], name1[30];
	int tr1=-1, tr2=-1, tr3=-1, result,result0;
	int isarray=0, notone=0, isdef=1;
	//op1
	result0=opt_exps(name, 0);
	m_getsym();
	if(m_sym==mlbrack){//[
		isdef=0;
		isarray=1;
		m_getsym();
		opt_exps(aname, 1);
		m_getsym();//]
		//=
		m_getsym();
	}
	//op2
	m_getsym();
	result=opt_exps(name1, 1);
	//def
	if(strcmp(name, name1)==0) isdef=0;
	if(strcmp(name1,"RET")==0){
		indagtable(rfuncall, name);
		if(result0) dags[blockcount][rfuncall]->ischar=1;
		return;
	}
	tr1=searchdagtable(name1);
	if(tr1==-1){// new node
		tr1=indag(name1);
	}
	if(ch!='\0'){
		m_getsym();
		//array
		if(m_sym==mlbrack){//[
			m_getsym();
			opt_exps(name1, 1);
			//def
			if(strcmp(name, name1)==0) isdef=0;
			tr2=searchdagtable(name1);
			if(tr2==-1){// new node
				tr2=indag(name1);
			}
			m_getsym();//]
			if(ch!='\0') m_getsym();
			int rr=searchdag(tr1,tr2,mrarray,mident," ");
			if(rr==-1){
				tr1=indag(mrarray,dags[blockcount][tr1],dags[blockcount][tr2],result);
			}else {
				tr1=rr;
			}
		}
	}
	if(ch!='\0'){
		//op
		op=m_sym;
		//op3
		notone=1;
		m_getsym();
		result=opt_exps(name1, 1);
		//def
		if(strcmp(name, name1)==0) isdef=0;
		tr2=searchdagtable(name1);
		if(tr2==-1){// new node
			tr2=indag(name1);
		}
		if(ch!='\0'){
			m_getsym();
			//array
			if(m_sym==mlbrack){//[
				m_getsym();
				opt_exps(name1, 1);
				//def
				if(strcmp(name, name1)==0) isdef=0;
				tr3=searchdagtable(name1);
				if(tr3==-1){// new node
					tr3=indag(name1);
				}
				m_getsym();//]
				int rr=searchdag(tr2,tr3,mrarray,mident," ");
				if(rr==-1){
					tr2=indag(mrarray,dags[blockcount][tr2],dags[blockcount][tr3],result);
				}else {
					tr2=rr;
				}
			}
		}
	}
	//
	if(notone) result=searchdag(tr1,tr2,op,mident,"");
	else result=tr1;
	if(result==-1){
		result=indag(op,dags[blockcount][tr1],dags[blockcount][tr2],result0);
	}
	if(isarray){//array
		tr1=searchdagtable(aname);
		if(tr1==-1){// new node
			tr1=indag(aname);
		}
		int rr=searchdag(tr1,result,mlarray,mident," ");
		if(rr==-1){
			result=indag(mlarray,dags[blockcount][tr1],dags[blockcount][result],0);
		}else {
			result=rr;
		}
	}
	indagtable(result,name);
	//def
	result = opt_searchtable(name);
	if(isdef && result!=-1 && charident[result]->isvar){
		strcpy(nowblock->def[nowblock->defnum++], name);
	}
}
void opt_push(){
	char name[30], aname[30];
	int result, tr1=-1, tr2=-1;
	m_getsym();
	result=opt_exps(name, 1);
	tr1=searchdagtable(name);
	if(tr1==-1){// new node
		tr1=indag(name);
	}
	if(ch!='\0'){
		m_getsym();//[
		m_getsym();
		opt_exps(aname, 1);
		tr2=searchdagtable(aname);
		if(tr2==-1){// new node
			tr2=indag(aname);
		}
		m_getsym();//]
		int rr=searchdag(tr1,tr2,mrarray,mident," ");
		if(rr==-1){
			tr1=indag(mrarray,dags[blockcount][tr1],dags[blockcount][tr2],result);
		}else {
			tr1=rr;
		}
	}
	indag(mpushsy,dags[blockcount][tr1],NULL,0);
	pushs[parcount++]=dagnum[blockcount]-1;
}
void opt_callstatement(){//name[30], lchild  rchild cnum children
	m_getsym();
	struct dag*p;
	p=(struct dag*)malloc(sizeof(struct dag));
	strcpy(p->name,id);
	p->cnum=0;
	p->lchild=NULL;
	p->rchild=NULL;
	for(int i=parcount-1;i>=0;i--){
		p->cnum++;
		if(i==parcount-1){
			p->lchild=dags[blockcount][pushs[i]];
		}else if(i==parcount-2){
			p->rchild=dags[blockcount][pushs[i]];
		}else{
			p->children[p->cnum-3]=dags[blockcount][pushs[i]];
		}
	}
	indag(p);
	parcount=0;
	rfuncall=p->num;
}
void opt_ret(){
	if(ch=='\0'){
		indag(mretsy,NULL,NULL,0);
		return;
	}
	char name[30], aname[30];
	int result, tr1=-1, tr2=-1;
	m_getsym();
	result=opt_exps(name, 1);
	tr1=searchdagtable(name);
	if(tr1==-1){// new node
		tr1=indag(name);
	}
	if(ch!='\0'){
		m_getsym();//[
		m_getsym();
		opt_exps(aname, 1);
		tr2=searchdagtable(aname);
		if(tr2==-1){// new node
			tr2=indag(aname);
		}
		m_getsym();//]
		int rr=searchdag(tr1,tr2,mrarray,mident," ");
		if(rr==-1){
			tr1=indag(mrarray,dags[blockcount][tr1],dags[blockcount][tr2],result);
		}else {
			tr1=rr;
		}
	}
	indag(mretsy,dags[blockcount][tr1],NULL,0);
}
void opt_scan(){
	m_getsym();
	if(dagtbcount[blockcount]<=0){
		indagtable(-1,"scanf",line);
		//outputfile3(line);
	}else{
		int i;
		for(i=dagtbcount[blockcount]-1;i>=0;i--){
			if(dagtable[blockcount][i]->blocknum==-1) continue;
			if(dags[blockcount][dagtable[blockcount][i]->blocknum]->op!=mscanfsy) break;
		}
		if(i<0) indagtable(-1,"scanf",line);
		else indagtable(dagtable[blockcount][i]->blocknum,"scanf",line);
	}
	int num=indag(id,mscanfsy);
	indagtable(num,id);
	//def
	strcpy(nowblock->def[nowblock->defnum++],id);
}
void opt_print(){
	char name[30];
	int tr1,tr2,result;
	m_getsym();
	if(m_sym==stringcon){
		if(dagtbcount[blockcount]<=0){
			indagtable(-1,"printf",line);
			//outputfile3(line);
		}else{
			int i;
			/*for(i=dagtbcount[blockcount]-1;i>=0;i--){
				if(dagtable[blockcount][i]->blocknum==-1) continue;
				if(dags[blockcount][dagtable[blockcount][i]->blocknum]->op!=mscanfsy) break;
			}*/
			for(i=dagnum[blockcount]-1;i>=0;i--){
				if(dags[blockcount][i]->op != mscanfsy) break;
			}
			if(i<0) indagtable(-1,"printf",line);
			//else indagtable(dagtable[blockcount][i]->blocknum,"printf",line);
			else indagtable(i,"printf",line);
		}
	}else{
		result=opt_exps(name, 1);
		tr1=searchdagtable(name);
		if(tr1==-1){// new node
			tr1=indag(name);
		}
		//array
		if(ch!='\0'){
			m_getsym();//[
			m_getsym();
			opt_exps(name, 1);
			tr2=searchdagtable(name);
			if(tr2==-1){// new node
				tr2=indag(name);
			}
			m_getsym();//]
			int rr=searchdag(tr1,tr2,mrarray,mident," ");
			if(rr==-1){
				tr1=indag(mrarray,dags[blockcount][tr1],dags[blockcount][tr2],result);
			}else {
				tr1=rr;
			}
		}
		indag(mprintfsy,dags[blockcount][tr1],NULL,0);
	}
}

int searchchild(struct dag* node, int que[], int qcount){
	int in=0;
	for(int i=0;i<node->pnum;i++){
		for(int j=0;j<qcount;j++){
			if(node->parents[i]->num==que[j]){
				in++;
				break;
			}
		}
	}
	if((node->pnum==0) || in==node->pnum){
		que[qcount++]=node->num;
		if(node->lchild!=NULL){
			qcount=searchchild(node->lchild,que,qcount);
		}
	}
	return qcount;
}

int searchreg(){
	for(int i=0; i<scmax;i++){
		if(usedreg[i]==0){
			usedreg[i]=1;
			lastreg=i;
			return i;
		}
	}
	return -1;
}
string searchgvar(char name[30], int gvar[tregs]){//var to global reg
	string result;
	int num;
	for(int i=0;i<tregs;i++){
		num=gvar[i];
		if(num!=-1 && strcmp(cgraph[num]->name,name)==0){
			result=cgraph[num]->ischar?"$$":"$";
			sprintf(tostr,"%d",i+8);
			result+=tostr;
			return result;
		}
	}
	result=name;
	return result;
}
void dagprint(int num, int bno, int gvar[tregs]){//var to reg
	string outs1;
	struct dag* node;
	node=dags[bno][num];
	int find=0,returnf=0;
	node->reg=-1;
	//bz(label)/ret/printf(string)/push    
	if(node->judge==1){//op
		if(node->op==mbzsy){
			outs1 = "BZ ";
			//op1
			if(node->lchild->judge){//op
				if(node->lchild->op==mscanfsy){
					outs1+=searchgvar(node->lchild->name, gvar);
					outs1+=" ";
				}else{
					outs1+=node->lchild->ischar?"$$":"$";
					sprintf(tostr,"%d",node->lchild->reg);
					outs1+=tostr;
					outs1+=" ";
				}
			}else{//ident
				if(node->lchild->name[0]=='$'){
					outs1+="$";
					if(node->lchild->name[1]=='$') outs1+="$";
					sprintf(tostr,"%d",lastreg);
					outs1+=tostr;
					outs1+=" ";
				}else{
					outs1+=searchgvar(node->lchild->name,gvar);
					outs1+=" ";
				}
			}
			if(node->cmp!=mident){
				//op
				switch(node->cmp){
				case meql://==
					outs1+="==";
					break;
				case mneq://!=
					outs1+="!=";
					break;
				case mlss://<
					outs1+="<";
					break;
				case mleq://<=
					outs1+="<=";
					break;
				case mgtr://>
					outs1+=">";
					break;
				case mgeq://>=
					outs1+=">=";
					break;
				}
				//op2
				if(node->rchild->judge){//op
					if(node->rchild->op==mscanfsy){
						outs1+=searchgvar(node->rchild->name,gvar);
						outs1+=" ";
					}else{
						outs1+=node->rchild->ischar?"$$":"$";
						sprintf(tostr,"%d",node->rchild->reg);
						outs1+=tostr;
						outs1+=" ";
					}
				}else{//ident
					outs1+=searchgvar(node->rchild->name,gvar);
					outs1+=" ";
				}
			}
			outs1+=node->name;
			outputfile3(outs1);
		}else if(node->op==mretsy){
			outs1 = "RET ";
			if(node->lchild==NULL){
			}else{
				if(node->lchild->judge){//op
					if(node->lchild->op==mscanfsy){
						outs1+=searchgvar(node->lchild->name,gvar);
					}else{
						outs1+=node->lchild->ischar?"$$":"$";
						sprintf(tostr,"%d",node->lchild->reg);
						outs1+=tostr;
					}
				}else{//ident
					outs1+=searchgvar(node->lchild->name,gvar);
				}
			}
			outputfile3(outs1);
		}else if(node->op==mprintfsy){
			outs1 = "printf ";
			if(node->lchild->judge){//op
				if(node->lchild->op==mscanfsy){
					outs1+=searchgvar(node->lchild->name,gvar);
				}else{
					outs1+=node->lchild->ischar?"$$":"$";
					sprintf(tostr,"%d",node->lchild->reg);
					outs1+=tostr;
				}
			}else{//ident
				outs1+=searchgvar(node->lchild->name,gvar);
			}
			outputfile3(outs1);
		}else if(node->op==mpushsy){
			outs1 = "PUSH ";
			if(node->lchild->judge){//op
				if(node->lchild->op==mscanfsy){
					outs1+=searchgvar(node->lchild->name,gvar);
				}else{
					outs1+=node->lchild->ischar?"$$":"$";
					sprintf(tostr,"%d",node->lchild->reg);
					outs1+=tostr;
				}
			}else{//ident
				outs1+=searchgvar(node->lchild->name,gvar);
			}
			outputfile3(outs1);
		}else if(node->op==mcallsy){
			outs1="CALL ";
			outs1+=node->name;
			outputfile3(outs1);
			outs1=node->ischar?"$$":"$";
			for(int i=0; i<dagtbcount[bno];i++){
				if(dagtable[bno][i]->blocknum==num&&dagtable[bno][i]->judge==0){
					returnf=1;
					break;
				}
			}
			//reg
			if(returnf){
				node->reg=searchreg();
				sprintf(tostr,"%d",node->reg);
				outs1+=tostr;
				outs1+="=RET";
				outputfile3(outs1);
			}
		}else ;
	}
	enum m_symbol op = node->op;
	//reg
	if(op==mplus||op==mminus||op==mtimes||op==mdivsy||op==mrarray){
		outs1=node->ischar?"$$":"$";
		node->reg=searchreg();
		sprintf(tostr,"%d",node->reg);
		outs1+=tostr;
		outs1+="=";
		if(op==mrarray){
			if(node->lchild->judge){//op []=
				for(int i=0; i<dagtbcount[bno];i++){
					if(dagtable[bno][i]->blocknum==node->lchild->num && dagtable[bno][i]->lname[0]!='$'){
						outs1+=dagtable[bno][i]->lname;
						break;
					}
				}
			}else{
				outs1+=node->lchild->name;
			}
			outs1+="[";
			if(node->rchild->judge){//op
				if(node->rchild->op==mscanfsy){
					outs1+=searchgvar(node->rchild->name,gvar);
				}else{
					outs1+=node->rchild->ischar?"$$":"$";
					sprintf(tostr,"%d",node->rchild->reg);
					outs1+=tostr;
				}
			}else{//ident
				outs1+=searchgvar(node->rchild->name,gvar);
			}
			outs1+="]";
		}else{
			//num1
			if(node->lchild->judge){//op
				if(node->lchild->op==mscanfsy){
					outs1+=searchgvar(node->lchild->name,gvar);
				}else{
					outs1+=node->lchild->ischar?"$$":"$";
					sprintf(tostr,"%d",node->lchild->reg);
					outs1+=tostr;
				}
			}else{//ident
				outs1+=searchgvar(node->lchild->name,gvar);
			}
			//op
			switch(op){
				case mplus:
					outs1+="+";
					break;
				case mminus:
					outs1+="-";
					break;
				case mtimes:
					outs1+="*";
					break;
				case mdivsy:
					outs1+="/";
					break;
			}
			//num2
			if(node->rchild->judge){//op
				if(node->rchild->op==mscanfsy){
					outs1+=searchgvar(node->rchild->name,gvar);
				}else{
					outs1+=node->rchild->ischar?"$$":"$";
					sprintf(tostr,"%d",node->rchild->reg);
					outs1+=tostr;
				}
			}else{//ident
				outs1+=searchgvar(node->rchild->name,gvar);
			}

		}
		outputfile3(outs1);
	}
	for(int i=0; i<dagtbcount[bno];i++){
		if(dagtable[bno][i]->blocknum==num){
			if(dagtable[bno][i]->judge){//printf string
				outputfile3(dagtable[bno][i]->s1);
				continue;
			}
			if(op==mcallsy||op==mplus||op==mminus||op==mtimes||op==mdivsy||op==mrarray||op==mlarray||op==mscanfsy){
				//1-op call(label)  +-*/  array(=right:[]   =left:[]=) scanf  
				if(dagtable[bno][i]->lname[0]!='$'){//ident
					if(op==mscanfsy){
						if(strcmp(dagtable[bno][i]->lname,node->name)!=0){
							outs1=searchgvar(dagtable[bno][i]->lname,gvar);
							outs1+="=";
							outs1+=searchgvar(node->name,gvar);
							outputfile3(outs1);
						}else{
							string sname1=searchgvar(node->name,gvar);
							string sname2=node->name;
							if(sname1!=sname2){
								outs1=sname1;
								outs1+="=";
								outs1+=sname2;
								outputfile3(outs1);
							}
							//outs1="scanf ";
							//outs1+=node->name;
							//outputfile3(outs1);
						}
					}else if(op==mlarray){
						outs1=dagtable[bno][i]->lname;
						outs1+="[";
						//op1
						if(node->lchild->judge){//op
							if(node->lchild->op==mscanfsy){
								outs1+=searchgvar(node->lchild->name,gvar);
							}else{
								outs1+=node->lchild->ischar?"$$":"$";
								sprintf(tostr,"%d",node->lchild->reg);
								outs1+=tostr;
							}
						}else{//ident
							outs1+=searchgvar(node->lchild->name,gvar);
						}
						outs1+="]=";
						//op2
						if(node->rchild->judge){//op
							if(node->rchild->op==mscanfsy){
								outs1+=searchgvar(node->rchild->name,gvar);
							}else{
								outs1+=node->rchild->ischar?"$$":"$";
								sprintf(tostr,"%d",node->rchild->reg);
								outs1+=tostr;
							}
						}else{//ident
							outs1+=searchgvar(node->rchild->name,gvar);
						}
						outputfile3(outs1);
					}else{
						outs1=searchgvar(dagtable[bno][i]->lname,gvar);
						outs1+=node->ischar?"=$$":"=$";
						sprintf(tostr,"%d",node->reg);
						outs1+=tostr;
						outputfile3(outs1);
					}
				}
			}
			if(node->judge==0){//0-ident  //+- /ident/char/int  
				if(dagtable[bno][i]->lname[0]!='$'){//ident
					if(strcmp(dagtable[bno][i]->lname,node->name)!=0){
						outs1=searchgvar(dagtable[bno][i]->lname,gvar);
						outs1+="=";
						outs1+=searchgvar(node->name,gvar);
						outputfile3(outs1);
					}
				}
			}
		}
	}
}
void del(int num){
	struct labelbk* p;
	struct dag * q;
	//dagtable  
	for(int i=0;i<dagtbcount[num];i++){
		p=dagtable[num][i];
		free(p);
	}
	//dag tops
	for(int i=0;i<dagnum[num];i++){
		q=dags[num][i];
		free(q);
	}
	for(int i=0;i<dagnum[num];i++){
		tops[num][i]=0;
	}
	dagnum[num]=0;
	dagtbcount[num]=0;
}
void blockopt(int num, int gvar[tregs]){
	struct dag* node;
	int que[dagmax];
	int qcount=0;
	int inque=0, in=0;
	for(int i=dagnum[num]-1;i>=0;i--){
		node=dags[num][i];
		inque=0;
		in=0;
		for(int j=0;j<qcount;j++){
			if(node->num==que[j]){
				inque=1;
				break;
			}
		}
		if(inque) continue;
		for(int k=0;k<node->pnum;k++){
			for(int j=0;j<qcount;j++){
				if(node->parents[k]->num==que[j]){
					in++;
					break;
				}
			}
		}
		if(tops[num][i]==1 || in==node->pnum){
			if(node->op==mcallsy){
				que[qcount++]=node->num;
				for(int j=0;j<node->cnum;j++){
					if(j==0){
						que[qcount++]=node->lchild->num;
					}else if(j==1){
						que[qcount++]=node->rchild->num;
					}else{
						que[qcount++]=node->children[j-2]->num;
					}
					
				}
			}
			else qcount=searchchild(dags[num][i],que,qcount);
		}
	}
	//print
	//string
	for(int i=0;i<dagtbcount[num];i++){
		if(dagtable[num][i]->blocknum==-1){
			outputfile3(dagtable[num][i]->s1);
		}
	}

	for(int i=0;i<scmax;i++) usedreg[i]=0;
	enum m_symbol op;
	for(int i=qcount-1;i>=0;i--){
		dagprint(que[i], num, gvar);
		//del reg
		for(int j=qcount-1;j>=i;j--){//1-op /call(label)/ +-*/  array(=right:[] 
			node=dags[num][que[j]];
			op=node->op;
			if(!(op==mcallsy||op==mplus||op==mminus||op==mtimes||op==mdivsy||op==mrarray)) continue;
			if(node->reg==-1) continue;
			int pp=0;
			for(int k1=0;k1<node->pnum;k1++){
				for(int k=qcount-1;k>=i;k--){
					if(node->parents[k1]->num==que[k]) pp++;
				}
			}
			if(pp==node->pnum){
				usedreg[node->reg]=0;
				node->reg=-1;
			}
		}
	}
}

int active(){
	int judges=0,find;
	int blocknum=-1;
	for(int i=blockcount-1;i>=0;i--){
		//out=ins
		if(blocks[i]->nextnum ==1 || blocks[i]->nextnum ==3){
			if(blocks[i]->next!=-1){//exit
				blocknum=blocks[i]->next-1;
				for(int j=0;j<blocks[blocknum]->innum;j++){//in
					//find var
					find=0;
					for(int k=0;k<blocks[i]->outnum;k++){
						if(strcmp(blocks[i]->out[k],blocks[blocknum]->in[j])==0){
							find=1;
							break;
						}
					}
					if(!find){//insert
						judges=1;
						strcpy(blocks[i]->out[blocks[i]->outnum++],blocks[blocknum]->in[j]);
					}
				}
			}
		}
		if(blocks[i]->nextnum ==2 || blocks[i]->nextnum ==3){//label
			blocknum=labelsearch(blocks[i]->label);
			blocknum=lblocks[blocknum]->blocknum-1;
				for(int j=0;j<blocks[blocknum]->innum;j++){//in
					//find var
					find=0;
					for(int k=0;k<blocks[i]->outnum;k++){
						if(strcmp(blocks[i]->out[k],blocks[blocknum]->in[j])==0){
							find=1;
							break;
						}
					}
					if(!find){//insert
						judges=1;
						strcpy(blocks[i]->out[blocks[i]->outnum++],blocks[blocknum]->in[j]);
					}
				}
			
		}
		//in=use+(out-def)
		//use
		for(int j=0;j<blocks[i]->usenum;j++){
			find=0;
			for(int k=0;k<blocks[i]->innum;k++){
				if(strcmp(blocks[i]->in[k],blocks[i]->use[j])==0){
					find=1;
					break;
				}
			}
			if(!find){//insert
				judges=1;
				strcpy(blocks[i]->in[blocks[i]->innum++],blocks[i]->use[j]);
			}
		}
		//out-def
		for(int j=0;j<blocks[i]->outnum;j++){
			find=0;
			for(int k=0;k<blocks[i]->innum;k++){
				if(strcmp(blocks[i]->in[k],blocks[i]->out[j])==0){
					find=1;
					break;
				}
			}
			for(int k=0;k<blocks[i]->defnum;k++){
				if(strcmp(blocks[i]->def[k],blocks[i]->out[j])==0){
					find=1;
					break;
				}
			}
			if(!find){//insert
				judges=1;
				strcpy(blocks[i]->in[blocks[i]->innum++],blocks[i]->out[j]);
			}
		}
	}
	return judges;
}
int searchcgraph(char name[30]){
	int result=-1;
	for(int i=0;i<cgraphcount;i++){
		if(strcmp(cgraph[i]->name,name)==0) return i;
	}
	return result;
}
int sblocks(char name[30]){
	int result=-1;
	for(int i=0;i<blockcount;i++){
		for(int j=0;j<blocks[i]->innum ;j++)
			if(strcmp(blocks[i]->in[j],name)==0) return 1;
	}
	return result;
}
int incgraph(char name[30]){
	struct dag *p;
	p=(struct dag *)malloc(sizeof(struct dag));
	//num name cnum children reg ischar
	int result=opt_searchtable(name);
	p->ischar=charident[result]->judge;
	p->num=cgraphcount;
	p->reg=-1;
	strcpy(p->name,name);
	p->cnum=0;
	cgraph[cgraphcount++]=p;
	return p->num;
}
void setedge(int num1, int num2){
	int judge=0;
	struct dag *p, *q;
	p=cgraph[num1];
	q=cgraph[num2];
	for(int i=0;i<p->cnum;i++){
		if(p->children[i]->num==q->num){
			judge=1;
			break;
		}
	}
	if(judge==0){
		p->children[p->cnum++]=q;
		q->children[q->cnum++]=p;
	}
}
int sregs(int used[tregs]){
	for(int i=0;i<tregs;i++){
		if(used[i]==0) return i;
	}
	return -1;
}
void regopt(){
	int result=1,result1;
	//active var
	while(result) result=active();
	//conflict graph
	for(int i=0;i<blockcount;i++){
		for(int j=0;j<blocks[i]->innum ;j++){
			result=searchcgraph(blocks[i]->in[j]);
			if(result==-1){
				result=sblocks(blocks[i]->in[j]);
				if(result==-1) continue;
				result=incgraph(blocks[i]->in[j]);
			}
			for(int k=j+1;k<blocks[i]->innum ;k++){
				result1=searchcgraph(blocks[i]->in[k]);
				if(result1==-1){
					result1=sblocks(blocks[i]->in[k]);
					if(result1==-1) continue;
					result1=incgraph(blocks[i]->in[k]);
				}
				setedge(result, result1);
			}
		}
	}
	//reg tregs
	struct dag * node;
	int innodes[tmax];
	int nodecount=0;
	result=1;
	//in node
	while(nodecount!=cgraphcount){
		if(result==0){//no node sides < reg
			for(int i=0;i<cgraphcount;i++){
				node = cgraph[i];
				result1=0;
				for(int j=0;j<nodecount;j++){
					if(innodes[j]==node->num){
						result1=1;
						break;
					}
				}
				if(!result1){
					innodes[nodecount++]=node->num;
					break;
				}
			}
		}
		result=0;
		for(int i=0;i<cgraphcount;i++){
			node = cgraph[i];
			result1=0;
			for(int j=0;j<nodecount;j++){
				if(innodes[j]==node->num){
					result1=1;
					break;
				}
			}
			if(result1) continue;
			//sides
			int side=0;
			for(int j=0;j<node->cnum;j++){
				int judge=0;
				for(int k=0;k<nodecount;k++){
					if(innodes[k]==node->children[j]->num){
						judge=1;
						break;
					}
				}
				if(!judge) side++;
			}
			if(side<tregs){
				result=1;
				innodes[nodecount++]=node->num;
			}
		}
	}
	//reg
	int usedregs[tregs];
	for(int i=0;i<tregs;i++) usedregs[i]=0;
	for(int i=nodecount-1;i>=0;i--){
		node = cgraph[innodes[i]];
		for(int j=0;j<tregs;j++) usedregs[j]=0;//empty
		for(int j=0;j<node->cnum;j++){
			if(node->children[j]->reg==-1) continue;
			usedregs[node->children[j]->reg]=1;
		}
		result=sregs(usedregs);
		node->reg=result;
	}
}
void del(){
	//del blocks
	struct block* p;
	struct labelbk * q;
	struct dag* k;
	for(int i=0;i<blockcount;i++){
		p=blocks[i];
		free(p);
	}
	for(int i=0;i<lblockcount;i++){
		q=lblocks[i];
		free(q);
	}
	for(int i=0;i<cgraphcount;i++){
		k=cgraph[i];
		free(k);
	}
	blockcount=0;
	lblockcount=0;
	cgraphcount=0;
	//del block

}

void printtof(){//order????
	int result;
	int gvar[tregs];
	string fss;
	for(int j=0;j<tregs;j++) gvar[j]=-1;
	for(int i=0;i<blockcount;i++){
		//global reg  sw to var
		for(int j=0;j<blocks[i]->innum;j++){
			result=searchcgraph(blocks[i]->in[j]);
			if(result!=-1 && cgraph[result]->reg!=-1){
				if(result!=gvar[cgraph[result]->reg]){
					//sw
					if(gvar[cgraph[result]->reg]!=-1){
						fss = cgraph[gvar[cgraph[result]->reg]]->name;
						fss+="=";
						fss+=cgraph[result]->ischar?"$$":"$";
						sprintf(tostr,"%d",cgraph[result]->reg+8);
						fss+=tostr;
						outputfile3(fss);
					}
					//lw new
					fss=cgraph[result]->ischar?"$$":"$";
					sprintf(tostr,"%d",cgraph[result]->reg+8);
					fss+=tostr;
					fss+="=";
					fss+= cgraph[result]->name;
					outputfile3(fss);
					gvar[cgraph[result]->reg]=result;
				}
			}
		}
		//label
		if(blocks[i]->judge==1 || blocks[i]->judge==3){
			fss=blocks[i]->label1;
			fss+=":";
			outputfile3(fss);
		}

		blockopt(i, gvar);
		del(i);

		//last block sw to global (ret goto label op)
		if(blocks[i]->nextnum==1 && blocks[i]->next==-1){
			for(int j=0;j<tregs;j++){
				if(gvar[j]!=-1){
					result= opt_searchtable(cgraph[gvar[j]]->name);
					if(result!=-1 && charident[result]->blocknum==0){
						fss = cgraph[gvar[j]]->name;
						fss+="=";
						fss+=cgraph[gvar[j]]->ischar?"$$":"$";
						sprintf(tostr,"%d",cgraph[gvar[j]]->reg+8);
						fss+=tostr;
						outputfile3(fss);
					}
				}
			}
		}

		if(blocks[i]->judge==2 || blocks[i]->judge==3){
			fss="GOTO ";
			fss+=blocks[i]->label;
			outputfile3(fss);
		}
	}
}