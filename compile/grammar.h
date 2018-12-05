
void program();// the whole program

void constdeclare();
void vardeclare();
void rfuncdeclare();
void voidfuncdeclare();
void mainfunc();
void parameters();
//combine statements
void cstatements();
// a list of statements
void lstatements();
//a statement
void statement();
void ifstatement();
void whilestatement();
void funcall();
void assignstatement();
void scanfstatement();
void printfstatement();
void switchstatement();
void returnstatement();

string expression(int *judge, int *regs);
string term(int *judge, int *regs);
string factor(int *judge, int *regs);

int switchtable(int endl, string s1);
void casestatement(int lab, int endl, string s1);
void defaultstatement();

int integer();

