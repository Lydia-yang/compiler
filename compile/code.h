void m_nextchar();
void m_getsym();
void  m_setup();
void pmain(int isopt);

void gstr(); //.data string char

void dconst();//.data const
void dvar();//.data var
int tvfunction();//void function

int m_statements(int sp);
void tconst();	//const
int tvar(int sp);	//VAR
int paras(int sp);		//PARA
void gotostatement();	//GOTO
void bzstatement();	//BZ
void labels();		//uper letters
void opeartors();	//1=a+b(ident(lower), $)
void pushstatement();	//PUSH
void callstatement();	//CALL
void ret(int sp);		//RET
void scan();	//scanf
void print();	//printf



