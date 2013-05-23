



#define VT_IDENT	100
#define VT_INT		102


// typedef struct Variable Variable_s;
//typedef struct vtype vtype_s;

typedef struct vtype_s
{
	int             intval;
	char            identval[25];
}               vtype;

typedef struct Variable_s
{
	char            name[25];
	int             type;
	vtype           value;

}               Variable;
