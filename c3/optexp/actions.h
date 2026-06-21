/* Action Instruction Strings */

static char
		one[] = "1",
                d[] = "d",
                x[] = "x",
                y[] = "y",
                u[] = "u",
                dx[] = "d,x",
                dy[] = "d,y",
                du[] = "d,u",
                dxu[] = "d,x,u",
                dxyu[] = "d,x,y,u",
                xupc[] = "x,u,pc",
                yd[] = "y,d",
                yu[] = "y,u",
                ud[] = "u,d",
                upc[] = "u,pc",
                clra[] = "clra",
                clrb[] = "clrb",
                cmpb[] = "cmpb",
                cmpd[] = "cmpd",
                cmpx[] = "cmpx",
                cmpy[] = "cmpy",
                cmpu[] = "cmpu",
                ldb[] = "ldb",
                ldd[] = "ldd",
                ldx[] = "ldx",
                ldy[] = "ldy",
                ldu[] = "ldu",
                stb[] = "stb",
                std[] = "std",
                stx[] = "stx",
                sty[] = "sty",
                stu[] = "stu",
                pshs[] = "pshs",
                puls[] = "puls",
                leax[] = "leax",
                leay[] = "leay",
                leau[] = "leau",
                leas[] = "leas",
                sex[] = "sex",
                tfr[] = "tfr",
                zero[] = "#0",
                p2x[] = "2,x",
                p1x[] = "1,x",
                zx[] = "0,x",
                m1x[] = "-1,x",
                m2x[] = "-2,x",
                p2y[] = "2,y",
                p1y[] = "1,y",
                zy[] = "0,y",
                m1y[] = "-1,y",
                m2y[] = "-2,y",
                p2u[] = "2,u",
                p1u[] = "1,u",
                zu[] = "0,u",
                m1u[] = "-1,u",
                m2u[] = "-2,u",
                p2s[] = "2,s",
                zs[] = "0,s",
                m2s[] = "-2,s",
                litnum[] = "#<";

typedef struct ins
{
	struct ins     *nxtins;
	char           *mnp,
	               *opp;
}               instr;

static          instr
                act1b = {NULL, std, NULL},
                act1a = {&act1b, ldd, one}, rep1a = {NULL, std, NULL},

                act2f = {NULL, clra, NULL},
                act2e = {&act2f, clrb, NULL},
                act2d = {&act2e, pshs, d}, rep2d = {NULL, clra, NULL},
                act2c = {&act2d, clra, NULL}, rep2c = {&rep2d, clrb, NULL},
                act2b = {&act2c, clrb, NULL}, rep2b = {&rep2c, pshs, d},
                act2a = {&act2b, pshs, d}, rep2a = {&rep2b, pshs, d},

                act3b = {NULL, leas, p2s},
                act3a = {&act3b, pshs, d}, rep3a = {NULL, std, zs},

                act4b = {NULL, leas, p2s},
                act4a = {&act4b, std, m2s}, rep4a = {NULL, std, ",s++"};

typedef struct act
{
	struct act     *nxtact;
	instr          *actp,
	               *repp;
}               action;

action          actions[] = {
	{actions, &act1a, &rep1a},
	{actions, &act2a, &rep2a},
	{actions, &act3a, &rep3a},
	{actions, &act4a, &rep4a},
	{0, NULL, NULL},
};
