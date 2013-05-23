/* comp_01.c */
int main(int argc, char **argv);
void quit_cc(void);
/* comp_02.c */
void null_lbldef(register LBLDEF *srcdef);
void fill_g18(register LBLDEF *dstdef);
void err_quit(char *p1);
void multdef(void);
void reprterr(char *_str);
void comperr(CMDREF *p1, char *_errmsg);
void err_lin(register CMDREF *p1, char *_errmsg);
void e_putc(char ch);
void CCREFtoLeft(register CMDREF *myref);
void mk_lftcmd(register CMDREF *myref);
void CmdrefCpy(CMDREF *src, CMDREF *dest);
int is_vardef(void);
int is_sc_specifier(void);
int MSBrshft2(int p1);
int incptrdpth(int p1);
int numeric_op(int vrtyp);
struct brktdef *prevbrkt(register struct brktdef *p1);
int lookfor(int needed);
void cmma_rbrkt(void);
/* comp_03.c */
CMDREF *get_operand(int maxlblval);
int getconst(int parm1);
int do_math(int vrtyp, int firstval, int secondval);
CMDREF *add_cmdref(int __ccode, CMDREF *_leftcrf, CMDREF *_rightcrf, int myval, int __myline, char *_line_pos);
void exprmsng(void);
/* comp_04.c */
CMDREF *L111d(register CMDREF *curntcmd);
void divby_0(void);
int do_cast(register CMDREF *ptr, int to_typ);
void ck_declared(register CMDREF *regptr);
int get_fttyp(register CMDREF *regptr);
void resetcmdref(register CMDREF *err_ref);
int isintegral(int tstval);
void notintegral(CMDREF *c_ref);
/* comp_05.c */
void do_command(void);
/* comp_06.c */
void L34be(CMDREF *cref);
void L34d9(CMDREF *cref);
void L3898(CMDREF *cref);
void pushslong(CMDREF *cref);
/* comp_07.c */
void L3987(CMDREF *cref);
void L39a2(CMDREF *cref);
/* comp_08.c */
int signextend(int val, int varsiz);
void do_initvar(register LBLDEF *lbldf, int _ftTyp, int gntyp);
void cant_init(void);
/* comp_09.c */
void initbuf0(void);
void skipblank(void);
void getnxtch(void);
/* comp_10.c */
void gencode(int parm1, int parm2, int parm3, CMDREF *parm4);
void prnt_words(int *intarray, int count);
void prt_rgofst(int rgcode, int parm2, int baseval);
void prt_bgnfld(char *opcod);
void prt_opcode(char *opcod);
void prntCR(void);
void prnt_strng(char *strng);
void prnt_integer(int valu);
void loclbl_CR(int valu);
void prt_loclbl(int valu);
void prt_lblnam(char *lblnam);
int prt_rsrvstk(int valu);
void prt_label(char *lblnam, int isglbl);
/* comp_11.c */
void endprog(void);
void rmbnolbl(int parm1, int parm2, int parm3);
void globldata(LBLDEF *lbldf, int datsize, int isDP);
void localdata(LBLDEF *lbldf, int datsize, int isDP);
void prtprofil(char *parm1, int parm2);
void func_prolog(char *ttlnm, int isglobl, int lablnum);
void prt_profend(void);
void prtstkreq(void);
void prnt_fcb(void);
void prnt_fdb(void);
void bgncomment(void);
void prt_vsect(int isdp);
void prt_endsect(void);
void quitcc(void);
/* comp_12.c */
void nxt_word(void);
void tblsetup(void);
LBLDEF *FindLbl(char *fnc);
void *addmem(int siz);
void prnt_rzb(int valu);
/* comp_13.c */
void reg2d_add(CMDREF *cref);
void leax_reg(CMDREF *cref);
void L6eac(CMDREF *cref);
CMDREF *L6f83(CMDREF *cref);
int shftcount(int valu);
void L7567(CMDREF *cref, void (*fnc)(void));
void L75e3(CMDREF *cref);
int L76bf(CMDREF *cref);
int crf_isint(CMDREF *cref);
void L77ac(CMDREF *cref);
void do_asterisk(CMDREF *cref);
int isRegU_Y(int vtyp);
/* comp_14.c */
void funcmain(void);
void do_block(void);
int do_vartype(int *siz, struct brktdef **typdefpt, struct memberdef **strctmbr);
int proc_varname(LBLDEF **parnt_lbldef, struct brktdef **parntbrkt, int parnt_ftyp);
int setlblsize(register LBLDEF *lbldf, struct brktdef *mbrdf, int dfltsiz);
int get_varsize(int ftyp, int dfltsiz, struct brktdef *braceptr);
void finishfunc(LBLDEF **p1);
void noidentf(void);
/* comp_15.c */
void L939d(CMDREF *cref, int lblno, int parm3, int istrue);
int int_usrlbl(CMDREF *cref);
