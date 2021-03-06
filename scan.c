/****************************************************/
/* File: scan.c                                     */
/* The scanner implementation for the TIGER compiler */

#include "globals.h"
#include "scan.h"
#include "config.h"
#include "util.h"
#ifdef DEBUG
#include <stdio.h>
#endif
TokenType lookahead(int,int);


int stringtoint(char *s)
{
  int temp;
  temp=(s[2]-'0')+((s[1]-'0')*10) +((s[0]-'0')*100);
  return temp;
}


/* states in scanner DFA */
typedef enum
{ 
  START,INGT,INLT,INSTR,INDIV,
  INCOMMENT1,INASSIGN,INCOMMENT2,
  INNUM,INID,DONE,INCOLON,INESC,
  INESC1,INESCD1,INESCD2 
}StateType;

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];

/* BUFLEN = length of the input buffer for
   source code lines */
#define BUFLEN 256

static char lineBuf[BUFLEN]; /* holds the current line */
static int linepos = 0; /* current position in LineBuf */
static int bufsize = 0; /* current size of buffer string */
static int EOF_flag = FALSE; /* corrects ungetNextChar behavior on EOF */

/* getNextChar fetches the next non-blank character
   from lineBuf, reading in a new line if lineBuf is
   exhausted 
*/


int getNextChar(void)
{ 
  if (!(linepos < bufsize))
    { 
      lineno++;
      if (fgets(lineBuf,BUFLEN-1,source))
	{ 
#ifdef DEBUG
	  printf("Current line from tiger source : %s \n", lineBuf);
#endif
	  if (EchoSource) 
	    fprintf(listing,"%4d: %s",lineno,lineBuf);
	  bufsize = strlen(lineBuf);
	  linepos = 0;
	  return lineBuf[linepos++];
	}
      else
	{ 
	  EOF_flag = TRUE;
#ifdef DEBUG DEBUG
	  printf("End of file \n");
#endif
	  return EOF;
	}
    }
  else 
    return lineBuf[linepos++];
}

/* ungetNextChar backtracks one character
   in lineBuf */
static void ungetNextChar(void)
{ 
  if (!EOF_flag) 
    linepos-- ;
}

/* lookup table of reserved words */
static struct
{ 
  char* str;
  TokenType tok;
} reservedWords[MAXRESERVED]= {
  {"array",ARRAY},{"break",BREAK},{"do",DO},{"if",IF},{"for",FOR},{"function",FUNCTION},{"in",IN},{"let",LET},{"nil",NIL},{"of",OF},{"to",TO},{"type",TYPE},{"var",VAR},{"then",THEN},{"else",ELSE},{"end",END},{"while",WHILE},{"int",INT},{"string",STRING}};

/* lookup an identifier to see if it is a reserved word */
/* uses linear search */
static TokenType reservedLookup (char * s)
{ 
  int i;
  for (i=0;i<MAXRESERVED;i++)
    if (!strcmp(s,reservedWords[i].str))
      return reservedWords[i].tok;
  return ID;
}


/* Unget function called from Lookahead() to back up the input*/
static void Ungetlookaheadchar(int *L_linepos)
{
  if (!EOF_flag) 
    (*L_linepos)-- ;
}



/*Gets next char from input
  Takes four parameters so that the function does not have an effect on global variables linepos, linebuf, lineno and bufsize */
static int lookaheadchar(int *L_linepos,char *L_linebuf,int L_lineno,int L_bufsize)
{
  if (!(*L_linepos < L_bufsize))
    {
      L_lineno++;
      if (fgets(L_linebuf,BUFLEN-1,source))
	{
	  L_bufsize = strlen(L_linebuf);
	  *L_linepos = 0;
	  fseek(source,-(L_bufsize+1), SEEK_CUR);
	  return L_linebuf[(*L_linepos)++];
	}
      else
	{ EOF_flag = TRUE;
	  return EOF;
	}
    }
  else 
    return L_linebuf[(*L_linepos)++];
}



/*Lookahead function returns the next token without advancing the input 
	Takes two parameters:	n determines the number of tokens to lookahead
							lcontinue decides whether to continue previous lookahead or not*/
TokenType lookahead(int n,int lcontinue)
{  /* index for storing into tokenString */
   //int tokenStringIndex = 0;
   /* holds current token to be returned */
   TokenType currentToken;
   /* current state - always begins at START */
   StateType state = START;
   /* flag to indicate save to tokenString */
   int save;
 /* character array to hold ascii code of escape sequence inside a string */
   char ascii[4];
   //Create a buffer to hold lineBuf
   static char L_linebuf[BUFLEN];
   //Create a copy of linepos to send to lookahead() 
   static int L_linepos;
   if(!lcontinue)
   {
     L_linepos=linepos;
     strcpy(L_linebuf,lineBuf);
   }	
   while(n>0)
     {   /* Initialize state for every loop iteration */
       state=START;
       while (state != DONE)
	 {
	   /*address of linepos passed since it needs to be accessed by Lookahead() and Ungetlookahead() 
	     and corresponding changes must be reflected */
	   char c = lookaheadchar(&L_linepos,L_linebuf,lineno,bufsize);
	   switch (state)
	     { case START:
		 if (isdigit(c))
		   state = INNUM;
		 else if (isalpha(c))
		   state = INID;
		 else if (c == ':')
		   state = INCOLON;
		 else if ((c == ' ') || (c == '\t') || (c == '\n'))
		   ;
		 else if (c == '/')
		   {  state = INDIV;
		     
		   }
		 else if(c=='<')
		   state=INLT;
		 else if(c=='>')
		   state=INGT;
		 else if(c=='"')
		   { state=INSTR;    }
		 
		 else
		   { state = DONE;
		     switch (c)
		       { 
		       case EOF:
			 currentToken = ENDFILE;
			 printf(" ENDFILE token at line : %d in  %s \n", __LINE__, __FILE__);
			 break;
		       case '=':
			 currentToken = EQ;
			 break;
		       case '<':
			 currentToken = LT;
			 break;
		       case '+':
			 currentToken = PLUS;
			 break;
		       case '-':
			 currentToken = MINUS;
			 break;
		       case '*':
			 currentToken = TIMES;
			 break;
		       case '(':
			 currentToken = LPAREN;
			 break;
		       case ')':
			 currentToken = RPAREN;
			 break;
		       case ';':
			 currentToken = SEMICOLON;
			 break;
		       case ',':
			 currentToken=COMMA;
			 break;
		       case '[':
			 currentToken=LBRACK;
			 break;
		       case ']':
			 currentToken=RBRACK;
			 break;
		       case '{':
			 currentToken = LBRACE;
			 break;
		       case '}':
			 currentToken=RBRACE;
			 break;
		       case '.':
			 currentToken=DOT;
			 break;
		       case '&':
			 currentToken=AND;
			 break;
		       case '|':
			 currentToken=OR;
			 break;
		       default:
			 currentToken = ERROR;
			 printf("File : %s  Line : %d \n", __FILE__, __LINE__);
			 break;
		       }
		   }
		 break;
	     case INDIV:
	       if(c=='*'){ 
		 state=INCOMMENT1; 
	       }else{
		 state=DONE;
		 Ungetlookaheadchar(&L_linepos);
		 c='/';
		 currentToken=DIVIDE;
	       }
	       break;
	     case INCOMMENT1:
	       if (c == EOF)
		 { state = DONE;
		   
		   currentToken = ENDFILE;
		   printf(" ENDFILE token at line : %d in  %s \n", __LINE__, __FILE__);
		 }
	       else if (c == '*')
		 {
		   state = INCOMMENT2;
		 }
	       break;
	     case  INCOMMENT2 :
	       
	       if(c=='/')
		 state=START;
	       else if(c=='*')
		 ;          // do nothing
	       else
		 state=INCOMMENT1;
	       break;
	     case INCOLON:
	       state = DONE;
	       if (c == '=')
		 currentToken = ASSIGN;
	       else
		 { /* backup in the input */
		   Ungetlookaheadchar(&L_linepos);
		   currentToken = COLON;
		 }
	       break;
	     case INGT:
	       state = DONE;
	       if (c == '=')
		 
		 currentToken = GE;
	       else
		 { /* backup in the input */
		   Ungetlookaheadchar(&L_linepos);
		   currentToken = GT;
		 }
	       break;
	     case  INLT:
	       state = DONE;
	       if (c == '=')
		 currentToken = LE;
	       else 
		 { /* backup in the input */
		   Ungetlookaheadchar(&L_linepos);
		   currentToken = LT;
		 }
	       break;
	     case INNUM:
	       if (!isdigit(c))
		 { /* backup in the input */
		   Ungetlookaheadchar(&L_linepos);
		   
		   state = DONE;
		   currentToken = INT;
		 }
	       break;
	     case INID:
	       if ((!isalpha(c))&&(c!='_')&&(!isdigit(c)))
		 { /* backup in the input */
		   Ungetlookaheadchar(&L_linepos);
		   state = DONE;
		   currentToken = ID;
		 }
	       break;
	     case  INSTR:
	       if(c=='"')
		 {state=DONE;
		   currentToken=STRING; 
		 }
	       else if(c=='\\')
		 { state=INESC; }
	       break;
	     case INESC:
	       if(isdigit(c))
		 {
		   ascii[0]=c;
		   state=INESCD1;
		   
		 }
	       else if(c=='"'||c=='\\')
		 {
		   state=INSTR;
		 }
	       else if(c=='n') 
		 { 
		   c=10; //10 is ascii code for new line character
		   state=INSTR; 
		 }
	       else  if(c=='t')
		 {
		   c=9;    // ascii code for tab character	
		   state=INSTR; 
		 }
	       else {  
		 currentToken=ERROR;
		 printf("File : %s  Line : %d \n", __FILE__, __LINE__);
	       }
	       break;
	     case INESCD1:
	       if(isdigit(c))
		 {
		   ascii[1]=c;
		   state=INESCD2;
		 }else{
		 currentToken=ERROR;    
		 printf("File : %s  Line : %d \n", __FILE__, __LINE__);   }
	       break;
	     case INESCD2:
	       
	       if(isdigit(c))
		 {
		   ascii[2]=c;
		   state=INSTR;
		   ascii[3]=0;
		   c=stringtoint(ascii);
		 }
	       else {  
		 currentToken=ERROR;  printf("File : %s  Line : %d \n", __FILE__, __LINE__);  }
	       break;
	     case DONE:
	     default: /* should never happen */
	       fprintf(listing,"Scanner Bug: state= %d\n",state);
	       state = DONE;
	       currentToken = ERROR;
	       printf("File : %s  Line : %d \n", __FILE__, __LINE__);
	       break;
	     } 
	 }
       n--;
     }  
   return currentToken;
} /* end lookahead */
 
 
 /****************************************/
/* the primary function of the scanner  */
/****************************************/
/* function getToken returns the
 * next token in source file
 */
 
TokenType getToken(void){  
  /* index for storing into tokenString */
  int tokenStringIndex = 0;
  /* holds current token to be returned */
  TokenType currentToken;
  /* current state - always begins at START */
  StateType state = START;
  /* flag to indicate save to tokenString */
  int save;
  /* character array to hold ascii code of escape sequence inside a string */
  char ascii[4];
  while (state != DONE){ 
    char c = getNextChar();
    save = TRUE;
    switch (state){ 
    case START:
      if (isdigit(c))
	state = INNUM;
      else if (isalpha(c))
	state = INID;
      else if (c == ':')
	state = INCOLON;
      else if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
	save = FALSE;
      else if (c == '/'){  
	state = INDIV;
	save =FALSE;
      }
      else if(c=='<')
	state=INLT;
      else if(c=='>')
	state=INGT;
      else if(c=='"'){ 
	state=INSTR;   
	save=FALSE;  
      }else{ 
	state = DONE;
	switch (c){ 
	case EOF:
	  save = FALSE;
	  currentToken = ENDFILE;
	  printf(" ENDFILE token at line : %d in  %s \n", __LINE__, __FILE__);
	  break;
	case '=':
	  currentToken = EQ;
	  break;
	case '<':
	  currentToken = LT;
	  break;
	case '+':
	  currentToken = PLUS;
	  break;
	case '-':
	  currentToken = MINUS;
	  break;
	case '*':
	  currentToken = TIMES;
	  break;
	case '(':
	  currentToken = LPAREN;
	  break;
	case ')':
	  currentToken = RPAREN;
	  break;
	case ';':
	  currentToken = SEMICOLON;
	  break;
	case ',':
	  currentToken=COMMA;
	  break;
	case '[':
	  currentToken=LBRACK;
	  break;
	case ']':
	  currentToken=RBRACK;
	  break;
	case '{':
	  currentToken = LBRACE;
	  break;
	case '}':
	  currentToken=RBRACE;
	  break;
	case '.':
	  currentToken=DOT;
	  break;
	case '&':
	  currentToken=AND;
	  break;
	case '|':
	  currentToken=OR;
	  break;
	default:
          printf("File : %s  Line : %d \n", __FILE__, __LINE__);
          printf("Current character : %d \n",(int)c);
	  currentToken = ERROR;
	  break;
	}
      }
      break;
    case INDIV:
      if(c=='*'){ 
	save=FALSE; 
	state=INCOMMENT1; 
      }else {
	state=DONE;
	ungetNextChar();
	save = TRUE;
	c='/';
	currentToken=DIVIDE;
      }
      break;
    case INCOMMENT1:
      save=FALSE;
      if (c == EOF){ 
	state = DONE;
	save=FALSE;
	currentToken = ENDFILE;
         printf(" ENDFILE token at line : %d in  %s \n", __LINE__, __FILE__);
      }
      else if (c == '*'){
	state = INCOMMENT2;
	save=FALSE;
      }
      break;
    case  INCOMMENT2 :
      save=FALSE;
      if(c=='/')
	state=START;
      else if(c=='*')
	;          // do nothing
      else
	state=INCOMMENT1;
      break;
    case INCOLON:
      state = DONE;
      if (c == '=')
	currentToken = ASSIGN;
      else{ /* backup in the input */
	ungetNextChar();
	save=FALSE;
	currentToken = COLON;
      }
      break;
    case INGT:
      state = DONE;
      if (c == '=')
	currentToken = GE;
      else{ /* backup in the input */
	ungetNextChar();
	save=FALSE;
	currentToken = GT;
      }
      break;
    case  INLT:
      state = DONE;
      if (c == '=')
	currentToken = LE;
      else { /* backup in the input */
	ungetNextChar();
	save=FALSE;
	currentToken = LT;
      }
      break;
    case INNUM:
      if (!isdigit(c)){ /* backup in the input */
	ungetNextChar();
	save = FALSE;
	state = DONE;
	currentToken = INT;
      }
      break;
    case INID:
      if ((!isalpha(c))&&(c!='_')&&(!isdigit(c))){ /* backup in the input */
	ungetNextChar();
	save = FALSE;
	state = DONE;
	currentToken = ID;
      }
      break;
    case  INSTR:
      if(c=='"')
	{save=FALSE ;state=DONE;
	  currentToken=STRING; 
	}
      else if(c=='\\')
	{ state=INESC; save=FALSE;  }
      break;
    case INESC:
      save=FALSE;
      if(isdigit(c))
	{
	  ascii[0]=c;
	  state=INESCD1;
	}
      else if(c=='"'||c=='\\')
	{
	  save=TRUE; state=INSTR;
	}
      else if(c=='n') 
	{ 
	  c=10; //10 is ascii code for new line character
	  save=TRUE;
	  state=INSTR; 
	}
      else  if(c=='t')
	  {
	    c=9;    // ascii code for tab character
	    save=TRUE; 
	    state=INSTR; 
	  }
      else  { 
	currentToken=ERROR;
      printf("File : %s  Line : %d \n", __FILE__, __LINE__); }
      break;
    case INESCD1:
      save=FALSE;
      if(isdigit(c))
	{
	  ascii[1]=c;
	  state=INESCD2;
	}
      else{ 
	currentToken=ERROR; 
	printf("File : %s  Line : %d \n", __FILE__, __LINE__); 
      }
      break;
    case INESCD2:
      save=FALSE;
      if(isdigit(c))
	{
	  ascii[2]=c;
	  state=INSTR;
	  ascii[3]=0;
	  c=stringtoint(ascii);
	  save=TRUE;
	}else{  
	currentToken=ERROR;  
	printf("File : %s  Line : %d \n", __FILE__, __LINE__); }
      break;
    case DONE:
    default: /* should never happen */
      fprintf(listing,"Scanner Bug: state= %d\n",state);
      state = DONE;
      currentToken = ERROR;
      printf("File : %s  Line : %d \n", __FILE__, __LINE__);
      break;
    }
    if ((save) && (tokenStringIndex <= MAXTOKENLEN))
      tokenString[tokenStringIndex++] = (char) c;
    if (state == DONE)
      { 
	tokenString[tokenStringIndex] = '\0';
	if (currentToken == ID)
	  currentToken = reservedLookup(tokenString);
      }
  }
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
    printf("Token: <  %s  >  at Line : %d \n", tokenString, lineno);
  }
  return currentToken;
} /* end getToken */

