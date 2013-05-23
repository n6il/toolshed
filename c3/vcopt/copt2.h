#define INBUF_SIZE 1024
#define MAXBACK 128		/* backup buffer gets dumped if over this
				 * size */
#define PATHEAPSIZE 20000
#define MAX_VAR 100
#define READ "r"

typedef struct PatTree_s PatTree;

struct PatTree_s
{
	char            chr;
	PatTree        *match;
	PatTree        *notmatch;
};

//char         *memmove(char *, const char *, int);
Variable       *FindVar(const char *varname, int namelen);
int             PatMatch3(PatTree * patp);
Variable       *FindVar(const char *varname, int namelen);
int             NameCmp(const char *name, const char *against, int alen);
void            PrintVar(Variable * varp);
void            Error(const char *errmsg);
void            ShiftBuf(void);
void            FillInbuf(void);
void            ReadMore(void);
int             ReadInt(void);
void            ReadIdent(char *buffer);
void            PatError(const char *errmsg);
const char     *PatMatchLine(const char *pattern);
const char     *AnyMatch(const char *pattern);
int             MatchVar5(PatTree * patp, char *exprptr);
int             MatchVar4(PatTree * patp);
int             MatchVar3(PatTree * patp, int readtoend);
int             MatchVar2(PatTree * patp, char *varp);
int             MatchVar(PatTree * patp);
int             PatMatchChr(PatTree * patp);
int             PatMatch3(PatTree * patp);
int             PatMatch17(PatTree * patp);
int             PatMatch9(PatTree * patp);
int             PatMatch5(PatTree * patp);
int             PatMatch6(PatTree * patp, char *linep);
int             PatMatch17(PatTree * patp);
int             PatMatch16(PatTree * patp);
int             PatMatch7(PatTree * patp);
int             PatMatch13(PatTree * patp);
int             PatMatch4(PatTree * patp);
void            Replace(char *bufptr, const char *replstr);
void            EvalExpr(const char **exprptr, char **strptr);
int             ExprTrue(const char *expr);
int             MakRepl(const char *replpat, char *replstr);
void            RemoveTags(void);
void            ReplPat(char *bufptr, const char *pat);
PatTree        *PatTreeAlloc(void);
void            PrintPatBranch(PatTree * ptp, int depth);
void            PrintPatTree(void);
PatTree       **AddPatTree(PatTree ** ptp, int c);
void            ReadPatFile(const char *filename);
int             ReplMatch2(void);
void            OutputLine(void);
void            Usage(void);
void            AdvLine(void);
void            BackLine(void);
void            Backup(void);
