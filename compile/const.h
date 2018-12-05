#define nkw 14		//number of key words
#define alng 10		//number of significant chars in identifiers
#define llng 1024	//input line length
#define kmax 15		//max number of significant digits
#define nmax 2147483647	//max number
#define tmax 500	//size of table 
#define lmax 7		//maximum level
#define pmax 8		//maximum number of parameters
#define stmax 100	//maximum number of string table
#define strmax 100	//max length of string
#define tregs 2		//regs for the whole
//define symbol as enum
enum symbol{ident,intcon,charcon,stringcon,funcsy,arraysy,plus,minus,times,divsy,eql,neq,lss,
       leq,gtr,geq,lparen,rparen,lbrack,rbrack,lbrace,rbrace,comma,semicolon,becomes,colon,
       constsy,intsy,charsy,voidsy,mainsy,ifsy,elsesy,whilesy,switchsy,casesy,defaultsy,scanfsy,printfsy,returnsy,errors,para};
#define m_nkw 14		//number of key words
#define pcmax 4			//$a0-$a3
#define scmax 8			//$s0-$s7
//m code symbol
enum m_symbol{mident,mintcon,mcharcon,mstringcon,mplus,mminus,mtimes,mdivsy,meql,mneq,mlss,mleq,mgtr,mgeq,
	mlparen,mrparen,mlbrack,mrbrack,mbecomes,mcolon,mdollar,mconstsy,mintsy,mcharsy,mvoidsy,mmainsy,mscanfsy,mprintfsy,
	mvarsy,mparasy,mretsy,mpushsy,mcallsy,mgotosy,mbzsy,mlarray,mrarray};
//table
struct table{
	char name[30];
	enum symbol obj;//ident(var) array function const
	enum symbol typ;//int char void
	int level;
	int adr;
	bool normal;
	//if ref==1
	struct func *tof;
	struct arrays *toa;
};
//function
struct func{
	int ptotal;
	enum symbol parameters[pmax];//intsy, charsy
};
//array
struct arrays{
	int low;
	int hight;
};


//opt
#define blockmax 600//max num of opt block
#define dagmax 100	//max num of dag node
#define dagtbmax 600	//max num of dag table
struct block{
	int num;
	int start;
	int end;
	//next
	int judge;//0-else 1-label 2-goto 3-label&goto
	int nextnum;//1-next 2-label 3-next label
	int next;
	char label[30];
	char label1[30]; //1-label
	//active
	int usenum;
	char use[tmax][30];
	int defnum;
	char def[tmax][30];
	int innum;
	char in[tmax][30];
	int outnum;
	char out[tmax][30];
};
struct labelbk{
	int judge;
	int blocknum;
	char lname[30];
	//judge==1 printf string
	char s1[llng];
	//var
	int isvar;
};
struct dag{
	int judge;//0-ident 1-op
	int num;
	char name[30]; //+- ret/$/ident/char/int  scanf
	enum m_symbol cmp;//< > <= >= == != mident
	enum m_symbol op;//+-*/  bz(label)/ret/push/call(label)/printf(string)  array(=right:[]  =left:[]=) mident
	struct dag * lchild;
	struct dag * rchild;
	int pnum;
	struct dag * parents[dagmax];
	//call
	int cnum;//2
	struct dag * children[dagmax];
	//reg
	int reg;
	//char
	int ischar;
};