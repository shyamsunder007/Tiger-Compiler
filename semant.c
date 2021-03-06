/*
 * semant.c - semantic analyser for Tiger
 */
#include"symbol.c"


 void transExp(S_table tenv,S_table venv, TreeNode *t) ;
  void transDecs(S_table tenv,S_table venv, TreeNode *t);

 typedef struct ParameterList_ *ParameterList;
struct ParameterList_
 {
	char *name;
	ExpType exptype;
	Datatype datatype;
	ParameterList next;
 };

 typedef struct EnvEntry_ *EnvEntry;
 static struct EnvEntry_
 {
	enum { VARENTRY, FUNCTIONENTRY, TYPEENTRY } kind;
	ExpType type;
	union{
	Datatype datatype;
	ParameterList params;
 }u;
};

EnvEntry enventry1,enventry2,enventry3;


ParameterList reverseList(ParameterList first)
{
ParameterList cur,temp;
cur=NULL;
while(first!=NULL)
{temp=first;
first=first->next;
temp->next=cur;
cur=temp;
}
return cur;
}

 void semantError(char * message, int lineno)         
{ fprintf(code,"\n>>> ");
  fprintf(code,"Semantic error at line %d: %s\n",lineno,message);
  Error=TRUE;
}


ExpType getNodeType(TreeNode *t, S_table venv, S_table tenv)
{
  if(t->expkind != IdK && t->expkind != LvalueK )
	{
		return t->type;
	}
	enventry1 = S_look(venv,S_Symbol(t->attr.id.name));
	if(enventry1 == NULL)
		enventry1 = S_look(tenv,S_Symbol(t->attr.id.name));
	return enventry1->type;

}

 void show(S_symbol name, void *value)
	{
		//printf("name: %s\n",S_name(name));
		//if(*((int*)value) == SIMPLEID)
			//printf("simpleid\n");
		ParameterList paramtemp;
		if(((EnvEntry)value)->kind == VARENTRY)
			{
			if(((EnvEntry)value)->u.datatype == SIMPLEID)
			printf("simple id, name = %s\n",S_name(name));
			else if(((EnvEntry)value)->u.datatype == ARRAYID)
				printf("array id, name = %s\n",S_name(name));
			if(((EnvEntry)value)->type == ETYPE_INTEGER)
				printf("Type:  Integer\n");
			else if(((EnvEntry)value)->type == ETYPE_STRING)
				printf("Type:  String\n");
			}
		else if(((EnvEntry)value)->kind == FUNCTIONENTRY)
		{
			printf("function, name = %s \n\tparameters:\n",S_name(name));

			paramtemp=((EnvEntry)value)->u.params;
			if(paramtemp == NULL)
				printf("\t(none)");
			for(;paramtemp!=NULL;paramtemp=paramtemp->next)
			{
				if(paramtemp->datatype == SIMPLEID)
					printf("\tsimple id, name = %s\n",paramtemp->name);
				else if(paramtemp->datatype == ARRAYID)
					printf("\tarray , name = %s\n",paramtemp->name);
				if(paramtemp->exptype == ETYPE_INTEGER)
					printf("\tType:  Integer\n");
				else if(paramtemp->exptype  == ETYPE_STRING)
					printf("\tType:  String\n");
			}
			if(((EnvEntry)value)->type == ETYPE_INTEGER)
					printf("Return type:  Integer\n");
				else if(((EnvEntry)value)->type  == ETYPE_STRING)
					printf("Return type:  String\n");
			
		}
		else if(((EnvEntry)value)->kind == TYPEENTRY)
		{
			printf("typeid ");
			if(((EnvEntry)value)->u.datatype == SIMPLEID)
			printf(", name = %s\n",S_name(name));
			else if(((EnvEntry)value)->u.datatype == ARRAYID)
				printf("array , name = %s\n",S_name(name));
			if(((EnvEntry)value)->type == ETYPE_INTEGER)
				printf("Type:  Integer\n");
			else if(((EnvEntry)value)->type == ETYPE_STRING)
				printf("Type:  String\n");
		}
		//printf("value: %d\n",(int)value);
	}




 void SEM_transProg(TreeNode *t)
 {
  S_table tenv = S_empty();
  S_table venv = S_empty();

  if(t==NULL)
	return;


  printf("At line : %d \n", __LINE__);
  transExp(tenv,venv,t);
printf("At line : %d \n", __LINE__);
  S_dump(tenv, show);
printf("At line : %d \n", __LINE__);
  S_dump(venv, show);
printf("At line : %d \n", __LINE__);
 }

 void transExp(S_table tenv,S_table venv, TreeNode *t)
 {
	char semantErrorMsg[200]; 
	S_symbol s; 
	EnvEntry enventry;
	TreeNode *temp;
	ParameterList paramtemp;
	switch(t->expkind)
	{
		case LetK: 
				S_beginScope(tenv);
				S_beginScope(venv);
				transDecs(tenv,venv,t->child[0]);
				transExp(tenv,venv,t->child[1]);
				S_endScope(tenv);
				S_endScope(venv);
				break;
		case IfK: 
				if(t->child[2] != NULL)
					if(getNodeType(t->child[1],venv,tenv) != getNodeType(t->child[2],venv,tenv) )
						semantError("type mismatch: in then and else clause",t->lineno);
				transExp(tenv,venv,t->child[0]);
				transExp(tenv,venv,t->child[1]);
				if(t->child[2] != NULL)
					transExp(tenv,venv,t->child[2]);
				break;
		case WhileK:
				transExp(tenv,venv,t->child[0]);
				transExp(tenv,venv,t->child[1]);
		      break;

		case ForK:
				transExp(tenv,venv,t->child[0]);
				transExp(tenv,venv,t->child[1]);
				if(getNodeType(t->child[0],venv,tenv) != getNodeType(t->child[1],venv,tenv))
					semantError("Low and High expressions must be integer expressions only",t->lineno);
				enventry2 = checked_malloc(sizeof(*enventry1));
				enventry2->kind = VARENTRY;
				enventry2->type = ETYPE_INTEGER;
				enventry2->u.datatype = SIMPLEID;
				S_beginScope(venv);
				s = S_Symbol(t->attr.id.name);
				S_enter(tenv, s ,enventry2);
				transExp(tenv,venv,t->child[2]);
				S_endScope(venv);
				break;
		case Str_constK:
		case IntK:
		case NilK:
			 break;
		case BreakK:
				if(t->attr.loopaddr->expkind!=ForK||t->attr.loopaddr->expkind!=WhileK)
				semantError("Break statement not within while or  for statement",t->lineno);
		      break;
		case OpK: 
				enventry1 = NULL;
				enventry2 = NULL;
				transExp(tenv,venv,t->child[0]);
				transExp(tenv,venv,t->child[1]);
				if(t->attr.op==PLUS||t->attr.op==MINUS||t->attr.op==TIMES||t->attr.op==DIVIDE)
				{
					if(t->child[0]->expkind != IdK && t->child[0]->expkind != LvalueK && t->child[1]->expkind != IdK && t->child[1]->expkind != LvalueK)
					{
						if(getNodeType(t->child[0],venv,tenv)!=getNodeType(t->child[1],venv,tenv))
						{
							semantError("type mismatch in operator expression ",t->lineno);
							break;
						}
					}
					if(t->child[0]->expkind == IdK || t->child[0]->expkind == LvalueK)
						enventry1 = S_look(venv,S_Symbol(t->child[0]->attr.id.name));
					if(t->child[1]->expkind == IdK || t->child[1]->expkind == LvalueK)
						enventry2 = S_look(venv,S_Symbol(t->child[1]->attr.id.name));
					if(enventry1 == NULL)
					{
						if(t->child[0]->type != enventry2->type )
						{
							semantError("type mismatch in operator expression ",t->lineno);
							break;
						}
					}
					else if(enventry2 == NULL)
					{
						if(t->child[1]->type != enventry1->type )
						{
							semantError("type mismatch in operator expression ",t->lineno);
							break;
						}
					}
					else
					{
						if(enventry1->type != enventry2->type )
						{
							semantError("type mismatch in operator expression ",t->lineno);
							break;
						}
					}
					
				}
				break;
		case LvalueK:
					if(t->child[0] != NULL)
						transExp(tenv,venv,t->child[0]);
					if(t->child[1] != NULL)
						transExp(tenv,venv,t->child[1]);
					if(t->child[0] == NULL && t->child[1] == NULL)
						transExp(tenv,venv,t);
					break;
		case IdK:
		      switch(t->attr.id.datatype)
			  {
				case SIMPLEID:
							enventry = S_look(venv,S_Symbol(t->attr.id.name));
							if(enventry == NULL ) 
								{
									strcpy(semantErrorMsg,"Undefined variable -> ");
									strcat(semantErrorMsg,t->attr.id.name);
									semantError(semantErrorMsg,t->lineno);
								}
							else if(enventry->kind != VARENTRY)
							{
								strcpy(semantErrorMsg,"Variable has same name as function ->");
								strcat(semantErrorMsg,t->attr.id.name);
								semantError(semantErrorMsg,t->lineno);
							}
							break;
				case ARRAYID:
							enventry = S_look(venv,S_Symbol(t->attr.id.name));
							if(enventry == NULL ) 
								{
									strcpy(semantErrorMsg,"Undefined array -> ");
									strcat(semantErrorMsg,t->attr.id.name);
									semantError(semantErrorMsg,t->lineno);
								}
							else if(enventry->kind != VARENTRY)
							{
								strcpy(semantErrorMsg,"Array has same name as function ->");
								strcat(semantErrorMsg,t->attr.id.name);
								semantError(semantErrorMsg,t->lineno);
							}
							break;
				case FUNCTIONCALL:
							enventry = S_look(venv,S_Symbol(t->attr.id.name));
							if( !strcmpi(t->attr.id.name,"read") || !strcmpi(t->attr.id.name,"print"))
								break;
							if(enventry == NULL ) 
								{
									strcpy(semantErrorMsg,"Function not defined -> ");
									strcat(semantErrorMsg,t->attr.id.name);
									semantError(semantErrorMsg,t->lineno);
								}
							else if(enventry->kind != FUNCTIONENTRY)
							{
								strcpy(semantErrorMsg,"Function has same name as variable ->");
								strcat(semantErrorMsg,t->attr.id.name);
								semantError(semantErrorMsg,t->lineno);
							}
							else 
							{
								temp = t->child[0];
								for(paramtemp = reverseList(enventry->u.params); paramtemp!=NULL; paramtemp=paramtemp->next)
								{
									if(getNodeType(temp,venv,tenv) != paramtemp->exptype)
									{
										semantError("type mismatch in function arguments",t->lineno);
									}
									temp = temp->sibling;
								}
								if(temp!= NULL || paramtemp != NULL)
								{
									semantError("Number of function arguments do not match",t->lineno);
									break;
								}
							}
							break;
							
			  }
			  break;
		case Par_expK:
					transExp(tenv,venv,t->child[0]);
					break;
		//case Comp_expK:
					
		case AssignK:
					transExp(tenv,venv,t->child[0]);
					if(t->child[0]->type != t->child[1]->type )
						semantError("type mismatch in assignment expression ",t->lineno);
					transExp(tenv,venv,t->child[1]);
					break;
		
		//case Bin_expK:
		case Bin1_expK:
					/*if(t->child[0]->child[0]->expkind == IdK && t->child[0]->child[1]->expkind == IdK)
					{
						enventry1 = S_look(venv,S_Symbol(t->child[0]->child[0]->attr.id.name));
						enventry2 = S_look(venv,S_Symbol(t->child[0]->child[1]->attr.id.name));
						if(enventry1->type != enventry2->type)
							semantError("type mismatch in comparison ",t->lineno);
						transExp(tenv,venv,t->child[0]);
						transExp(tenv,venv,t->child[1]);
						break;
					}
					else if(t->child[0]->child[0]->expkind == IdK)
					{
						enventry1 = S_look(venv,S_Symbol(t->child[0]->attr.id.name));
						if(enventry1->type != t->child[1]->type)
							semantError("type mismatch in comparison ",t->lineno);
						transExp(tenv,venv,t->child[0]);
						transExp(tenv,venv,t->child[1]);
						break;
					}
					else if(t->child[0]->child[1]->expkind == IdK)
					{
						enventry1 = S_look(venv,S_Symbol(t->child[1]->attr.id.name));
						if(enventry1->type != t->child[0]->type)
							semantError("type mismatch in comparison ",t->lineno);
						transExp(tenv,venv,t->child[0]);
						transExp(tenv,venv,t->child[1]);
						break;
					}
					else if(t->child[0]->type != t->child[1]->type) //both are not IdKs
						semantError("type mismatch in comparison ",t->lineno);
					transExp(tenv,venv,t->child[0]);*/
					transExp(tenv,venv,t->child[0]);
					break;
		case NegatedK:
		     if(t->type != ETYPE_INTEGER)
				semantError("Negation cannot be applied to non integer expression",t->lineno);
				break;
		default:
				printf("unexpected error in transExp: Semant.c");
				exit(0);
	}

 }

 void transDecs(S_table tenv,S_table venv, TreeNode *t)
 {

		TreeNode *temp;
		
		for(temp=t->child[0];temp != NULL;temp=temp->sibling) //type declarations : t is pointing to Decl_list node
						 {
						 enventry1 = checked_malloc(sizeof(*enventry1));
							if(temp->child[0] != NULL)
							{
								S_symbol s = S_Symbol(temp->child[0]->attr.id.name);
								//printf("typeid: %s",S_name(s));
								enventry1->kind = TYPEENTRY;
								enventry1->type = (temp->child[0])->type;
								enventry1->u.datatype = (temp->child[0])->attr.id.datatype;
								//printf(" exptype: %d",(temp->child[0])->type);
								if(S_look(tenv,s) != NULL)
								{
									char semantErrorMsg[200] = "Multiple declarations for the same typeid: ";
									strcat(semantErrorMsg,temp->child[0]->attr.id.name);
									semantError(semantErrorMsg,temp->lineno);
								}
								else
								S_enter(tenv, s ,enventry1);
							}
						 }
		for(temp=t->child[1];temp != NULL;temp=temp->sibling) //var declarations : t is pointing to Decl_list node
						 {
							enventry2 = checked_malloc(sizeof(*enventry2));
							if(temp->child[1] != NULL) //if var declaration exists
							{
								S_symbol s = S_Symbol(temp->attr.id.name);
								
								enventry2->kind = VARENTRY;
								if(temp->child[0] != NULL) //if type of var is specified
									if(temp->child[0]->type != temp->child[1]->type )
										semantError("Type mismatch in variable declaration",temp->lineno);
								enventry2->type = temp->type;
								//printf("  type: %d",enventry2->type);
								enventry2->u.datatype = temp->attr.id.datatype;
								if(S_look(venv,s) != NULL)
								{
									char semantErrorMsg[200] = "Multiple declarations for the same typeid: ";
									strcat(semantErrorMsg,temp->child[0]->attr.id.name);
									semantError(semantErrorMsg,temp->lineno);
								}
								else
								S_enter(venv, s ,enventry2);
							}
						 }
			 
		for(temp=t->child[2];temp != NULL;temp=temp->sibling) //function declarations : t is pointing to Decl_list node
						 {
							ParameterList params = NULL,paramtemp =NULL;
							TreeNode *ftemp = NULL;	
							EnvEntry enventry;
							S_symbol s = S_Symbol(temp->attr.id.name);
							enventry3 = checked_malloc(sizeof(*enventry3));
							
							if(temp->child[2] != NULL)     // if  function body is  not empty
							{ 
								for(ftemp = temp->child[0]; ftemp != NULL; ftemp = ftemp->sibling)
								{
									if(params == NULL)
									{
										params = checked_malloc(sizeof(*params));
										params->name = copyString(ftemp->attr.id.name);
										params->exptype = (ftemp->child[0])->type;
										params->datatype = ftemp->attr.id.datatype;
										params->next = NULL;
										
									}
									else
									{
										paramtemp = checked_malloc(sizeof(*paramtemp));
										paramtemp->name = copyString(ftemp->attr.id.name);
										paramtemp->exptype = (ftemp->child[0])->type;
										paramtemp->datatype = ftemp->attr.id.datatype;
										paramtemp->next = params;
										params = paramtemp;
										
									}
								}
								enventry3->kind = FUNCTIONENTRY;
							enventry3->u.params = params;
							enventry3->type = temp->child[1]->type;
							S_enter(venv, s , enventry3);
							S_beginScope(venv);
								for(paramtemp = params; paramtemp != NULL; paramtemp = paramtemp->next)
							{
								enventry = checked_malloc(sizeof(*enventry));
								enventry->kind = VARENTRY;
								enventry->type = paramtemp->exptype;
								enventry->u.datatype = paramtemp->datatype;
								S_enter(venv, S_Symbol(paramtemp->name) , enventry);
							}
								
								
							}
							if(temp->child[1]!=NULL&& getNodeType(temp->child[1],venv,tenv)!=getNodeType(temp->child[2],venv,tenv))
								semantError("Return type mismatch in function declaration",temp->lineno);
									
								else if (temp->child[1]==NULL)
								semantError("Procedure can not return value",temp->lineno);
							
							
							transExp(tenv,venv,temp->child[2]);
							S_endScope(venv);
						 }

	}
