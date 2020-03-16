#include <stdio.h>
#include <stdlib.h>

typedef enum NodeType_em
{
	NodeNone,
	NodeSec,
	NodeItem
}NodeType;

typedef struct NodePoint_st
{
	NodeType nType;
	char	key[32];
	char	value[32];
	struct NodePoint_st	*next;
	struct NodePoint_st	*son;
}NodePoint;

NodePoint*	cini_parsemem(char *ini)
{
	NodePoint *head	= NULL, *cur = NULL, *son = NULL, *one = NULL;
	int len = 0, ib = 0, in = 0, ieq = 0, ibsec = 0, iesec = 0;
	int chr = 0, nType = 0;
	len = strlen(ini);
	head = (NodePoint *)malloc(sizeof(NodePoint));
	memset(head,0x00,sizeof(NodePoint));
	head->nType = NodeNone;
	cur = head;
	while(ib < len)
	{
		chr = ini[in];
		if('[' == chr)
		{
			ibsec = in;
			nType = NodeSec;
		}
		else if(']' == chr)
		{
			iesec = in;
		}
		else if('='== chr)
		{
			ieq = in;
			nType = NodeItem;
		}
		else if('\n' == chr)
		{
			if(NodeSec == nType || NodeItem == nType)
			{
				one = (NodePoint *)malloc(sizeof(NodePoint));
				memset(one,0x00,sizeof(NodePoint));
				one->nType = nType;
			}
			if(NodeSec == nType)
			{
				memcpy(one->key,ini + ibsec + 1,iesec - ibsec - 1);
				cur->next = one;
				cur = one;
				son = NULL;
			}else if(NodeItem == nType)
			{
				memcpy(one->key,ini + ib,ieq - ib);
				memcpy(one->value,ini + ieq + 1,in - ieq -1);
				if(!son)
				{
					cur->son = one;
					son = one;
				}
				else
				{
					son->next = one;
					son = one;

				}
				
			}
			ib = in+1;//need 
		}else if(!chr)
			break;
		in++;
	}

	return head;	
}

int cini_size(NodePoint *head)
{
	int size = 0;
	NodePoint *cur = head, *son = NULL ;
        if(!head)
        	return 0;
        for(;cur;cur = cur->next)
        {
        	if(NodeSec != cur->nType)
        		continue;
       		son = cur->son;
        	size += strlen(cur->key) + 3;
        	for(;son; son = son->next)
        	{
			size += strlen(son->key) + strlen(son->value) + 2;
        	}
        }
	return size;
}
void cini_printBuf(NodePoint *head, char **buf)
{
	NodePoint *cur = head, *son = NULL ;
	int size = 0;
	char *log = NULL;	
	size = cini_size(head);
	if(!size)
	{
		*buf = NULL;
		return;
	}
	size += 1;
	log = (char *)malloc(size);
	memset(log,0x00,size);
	for(;cur;cur = cur->next)
	{
		if(NodeSec != cur->nType)
			continue;
		son = cur->son;
		sprintf(log + strlen(log),"[%s]\n",cur->key);
		for(;son; son = son->next)
		{
			sprintf(log + strlen(log),"%s=%s\n",son->key,son->value);
		}
	}
	*buf = log;
	return;
}
void cini_print(NodePoint *head)
{
	printf("#################\n");
	char *buff = NULL;
	cini_printBuf(head,&buff);
	printf(buff);
	free(buff);
	printf("#################\n");
}
// 0:none exists; other: exists
NodePoint* cini_exist(NodePoint *head, char *secName, char *key, char *value)
{
	NodePoint *ret = NULL;
	NodePoint *cur = NULL, *son = NULL;
	if(!head || !secName  || (!key && value))
	{
		return 0;
	}
	cur = head;
	for(; cur; cur = cur->next)
	{
		if(NodeSec != cur->nType)
			continue;
		if(0 != strcmp(secName,cur->key))
			continue;
		else if(!key)
		{
			ret = cur;
			break;
		}
		son = cur->son;
		for(; son; son = son->next)
		{
			if(0 != strcmp(key,son->key))
				continue;
			else if(!value || 0 == strcmp(value,son->value))
			{
				ret = son;
				break;
			}
		}
	}
	return ret;
}
NodePoint* cini_last(NodePoint *head, int nType)
{
	NodePoint *ret = NULL, *cur = NULL;
	int match = 0;
	if(!head)
		return NULL;
	if(NodeSec == nType && NodeNone == head->nType)
	{
		if(head->next)
			head = head->next;
		else
			return head;
	}
	if(NodeItem == nType && NodeSec == head->nType)
	{
		if(head->son)
			head = head->son;
		else
			return head;
	}
	if((NodeSec == nType && NodeSec == head->nType) ||
	(NodeItem == nType && NodeItem == head->nType))
	{
		for(cur = head; cur; cur = cur->next)
		{
			if(!cur->next)
			{
				ret = cur;
				break;
			}
		}
	}
	return ret;
}
void cini_addSec(NodePoint *head, char *secName)
{
	NodePoint *one = NULL, *last = NULL;
	if(!head || !secName)
		return;
	one = cini_exist(head,secName,NULL,NULL);
	if(one)
		return;
	last = cini_last(head,NodeSec);
	if(!last)
		return;
	one = (NodePoint *)malloc(sizeof(NodePoint));
	memset(one,0x00,sizeof(NodePoint));
	one->nType = NodeSec;
	strcpy(one->key,secName);
	last->next = one;

}
void cini_addKey(NodePoint *head, char *secName, char *key, char *value)
{
	NodePoint *one = NULL, *last = NULL;
	if(!head || !secName || !key || !value)
		return;
	head = cini_exist(head,secName,NULL,NULL);
	if(!head)
		return;
	one = cini_exist(head,secName,key,NULL);
	if(one)
		return;
	last = cini_last(head,NodeItem);
	if(!last)
		return;
	one = (NodePoint *)malloc(sizeof(NodePoint));
	memset(one,0x00,sizeof(NodePoint));
	one->nType = NodeItem;
	strcpy(one->key,key);
	strcpy(one->value,value);
	if(NodeSec == last->nType)
		last->son = one;
	else
		last->next = one;
}
void cini_add(NodePoint *head, char *secName, char *key, char *value)
{
	if(secName)
	{
		if(!key)
			cini_addSec(head,secName);
		else if(value)
			cini_addKey(head,secName,key,value);
	}
}
void cini_update(NodePoint *head, char *secName, char *key, char *value)
{
	NodePoint *cur = NULL;
	if(!head || !secName || !key || !value)
		return;
	cur = cini_exist(head,secName,key,NULL);
	if(!cur)
		return;
	strcpy(cur->value,value);
}

void cini_deleteSec(NodePoint *head, char *secName)
{
	NodePoint *cur = NULL, *pre = NULL;

	for(cur = head; cur; cur = cur->next)
	{
		if(0 == strcmp(cur->key,secName))
			break;
		pre = cur;
	}
	if(cur)
	{
		pre->next = cur->next;
		free(cur);
	}
}

void cini_deleteItem(NodePoint *head, char *secName, char *key, char *value)
{
	NodePoint *cur = NULL, *pre = NULL;

	head = cini_exist(head,secName,NULL,NULL);
	if(!head)
		return;
	cur = head->son;
	for(; cur; cur = cur->next)
	{
		if(0 == strcmp(cur->key,key))
			break;
		pre = cur;
	}
	if(cur)
	{
		if(!pre)
		{
			head->son = cur->next;
		}else
		{
			pre->next = cur->next;
		}
		free(cur);
	}
}
void cini_delete(NodePoint *head, char *secName, char *key, char *value)
{
	if(secName)
	{
		if(!key)
			cini_deleteSec(head,secName);
		else
		{
			cini_deleteItem(head,secName,key,NULL);
		}
		
	}
}

NodePoint* cini_new()
{
	NodePoint *ret = NULL;

	ret = (NodePoint *)malloc(sizeof(NodePoint));
	memset(ret,0x00,sizeof(NodePoint));
	ret->nType = NodeNone;
	return ret;
}
void testParse()
{
	char str[] = "[section1]\nkey11=value11\nkey12=value12\n[section2]\nkey21=value21\nkey22=value22\nkey23=value23\n[section3]\nkey31=value31\n";
	NodePoint *head = NULL;
	char *buf = NULL;
	head = cini_parsemem(str);
	cini_printBuf(head,&buf);
	printf("%s",buf);
}

void testAdd()
{
	int i = 0, j = 0;
	char secName[32] = {0}, key[32] = {0}, value[32] = {0}, *buff = NULL;
	NodePoint *head = NULL;

	head = cini_new();
	printf("[add test]\n\n");
	for(i = 1; i <= 10; i++)
	{
		sprintf(secName,"secName%i",i);
		cini_add(head,secName,NULL,NULL);
		cini_print(head);
		for(j = 1; j <= i; j++)
		{
			sprintf(key,"key%i-%i",i,j);
			sprintf(value,"value%i-%i",i,j);
			cini_add(head,secName,key,value);
			cini_print(head);
		}
	}
	printf("[update test]\n\n");
	for(i = 1; i <= 10; i++)
	{
		sprintf(secName,"secName%i",i);
		for(j = 1; j <= i; j++)
		{
			sprintf(key,"key%i-%i",i,j);
			sprintf(value,"vvalue%i-%i",i,j);
			cini_update(head,secName,key,value);
			cini_print(head);
		}
	}
	printf("[delete key-value test]\n\n");
	for(i = 1; i <= 10; i++)
	{
		sprintf(secName,"secName%i",i);
		for(j = 1; j <= i; j++)
		{
			sprintf(key,"key%i-%i",i,j);
			sprintf(value,"vvalue%i-%i",i,j);
			cini_delete(head,secName,key,value);
			cini_print(head);
		}
	}
	printf("[delete section test]\n\n");
	for(i = 1; i <= 10; i++)
	{
		sprintf(secName,"secName%i",i);
		cini_delete(head,secName,NULL,NULL);
		cini_print(head);
	}
}
int main(int argc, char *argv[])
{
	int ret = 0;
	testParse();
	testAdd();

}