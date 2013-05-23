
void            PrintName(const char *name, int namelen);
void            RdToken(void);
void            RdPriExpr(Variable * value);
void            RdPreExpr(Variable * value);
void
                RdBinExpr(
			                  Variable * value,
			                  int (*chkfunct) (void),
			                  int (*calcfunct) (int, int, int),
			                  void (*rdfunct) (Variable *)
);
	void            RdIBExpr(
				                 Variable * value,
				                 int (*chkfunct) (void),
			                int (*intcalcfunct) (int, int, int),
                    int (*identcalcfunct) (const char *, const char *, int),
				                 void (*rdfunct) (Variable *)
);
	int             ChkMDExpr(void);
	int             CalcMDExpr(int lnum, int rnum, int op);
	void            RdMDExpr(Variable * value);
	int             ChkPMExpr(void);
	int             CalcPMExpr(int lnum, int rnum, int op);
	void            RdPMExpr(Variable * value);
	int             ChkShiftExpr(void);
	int             CalcShiftExpr(int lnum, int rnum, int op);
	void            RdShiftExpr(Variable * value);
	int             ChkRelExpr(void);
	int             CalcRelExpr(int lnum, int rnum, int op);
	int             CalcIRelExpr(const char *lident, const char *rident, int op);
	void            RdRelExpr(Variable * value);
	int             ChkEqExpr(void);
	int             CalcEqExpr(int lnum, int rnum, int op);
	int             CalcIEqExpr(const char *lident, const char *rident, int op);
	void            RdEqExpr(Variable * value);
	int             ChkBAndExpr(void);
	int             CalcBAndExpr(int lnum, int rnum, int op);
	void            RdBAndExpr(Variable * value);
	int             ChkXOrExpr(void);
	int             CalcXOrExpr(int lnum, int rnum, int op);
	void            RdXOrExpr(Variable * value);
	int             ChkBOrExpr(void);
	int             CalcBOrExpr(int lnum, int rnum, int op);
	void            RdBOrExpr(Variable * value);
	void            RdLAndExpr(Variable * value);
	void            RdLOrExpr(Variable * value);
	void            RdCondExpr(Variable * value);
	void            RdExpr(Variable * value);
	const char     *ReadExpr(const char *expr, Variable * value);
