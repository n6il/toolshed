#include "cj.h"
#include <string.h>


char *kw[200];

static void prtree(int *node, char *title);
static void ptree(expnode *node);

void getkeys(void)
{
        char kname[20];
        register char *p;
        FILE *in1;
        int c;
        int i;

        for (i = 0; i < 200; kw[i++] = "UNKN") ;

        if ((in1 = fopen("/h0/lib/ckeys", "r")) == NULL)
		{
                fprintf(stderr, "no keys file\n",0);
                errexit();
        }
        for (i = c = 0; ++i < 200 && c != EOF ;)
		{
                for (p=kname; (c=getc(in1))!='\n' && c!=EOF ; *p++=c) ;
                *p='\0';
                if(*kname)
                        kw[i] = strcpy(grab(strlen(kname)+1),kname);
        }
        fclose(in1);
}


static void prtree(int *node, char *title)
{
    if (dflag)
	{
        fflush(stdout);
        printf("%s\naddr op             val  mod ", title);
        printf("      type      size sux left right\n");
        ptree(node);
    }
}

static void ptree(expnode *node)
{
	if (node)
	{
		pnode(node);
		ptree(node->left);
		ptree(node->right);
	}
}


void pnode(expnode *node)
{
        int op,val,i;

        op=node->op;
        printf("%04x %04x %-10.8s%04x %04x",
                    node,op,
                    op == NAME ? (int) node->val.sp->sname : kw[op],
                    node->val.num,node->modifier);
        val=node->type;
        for(i=14; i>=4 ; i-=2) {
                switch ((val>>i)  & 3) {
                        case 1: putchar('P');break;
                        case 2: putchar('A'); break;
                        case 3: putchar('F'); break;
                        case 0: putchar(' ');
                }
        }
        printf(" %-8s %4d %2d  %04x  %04x\n",kw[val & BASICT],node->size,
                                      node->sux, node->left,node->right);
}
