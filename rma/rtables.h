GLOBAL CODFORM S_Psec[]
#ifdef MAIN
= {
                {"psect",0x00,0}, {"csect",0x00,1},
                {"vsect",0x00,2}, {"end",0x06,3}
}
#endif
;

GLOBAL CODFORM S_Lbr[]
#ifdef MAIN
= {
               {"lbra",0x16, 0},                {"lbsr",0x17, 0},
               {"orcc",0x1a, 1},                {"andcc",0x1c,1},
               {"cwai",0x3c, 1},                {"addd",0xc3, 2},
               {"ldd", 0xcc, 2},                {"subd",0x83, 2},
               {"ldw", 0x86, PRE_10|2},         {"subw",0x80,PRE_10|2},
               {"ldx", 0x8e, 2},                {"ldu", 0xce, 2},
               {"cmpx",0x8c, 2},                {"jsr", 0x8d, NO_IMMED|2},
               {"std", 0xcd, NO_IMMED|2},       {"stx", 0x8f, NO_IMMED|2},
               {"stu", 0xcf, NO_IMMED|2},       {"cmpu",0x83, PRE_11|2},
               {"cmps",0x8c, PRE_11|2},         {"cmpd",0x83, PRE_10|2},
               {"cmpy",0x8c, PRE_10|2},         {"cmpw",0x81, PRE_10|2},
               {"ldy", 0x8e, PRE_10|2},         {"lds", 0xce, PRE_10|2},
               {"sty", 0x8f, NO_IMMED|PRE_10|2},{"addw",0x8b, PRE_10|2},
               {"sts", 0xcf, NO_IMMED|PRE_10|2},{"ldq", 0xcd, 14},
               {"stw", 0x87, NO_IMMED|PRE_10|2},{"divq",0x8e, PRE_11|2},
               {"ste", 0x87, NO_IMMED|PRE_11|2},{"divd",0x8d, PRE_11|19},
               {"stf", 0xc7, NO_IMMED|PRE_11|2},{"muld",0x8f, PRE_11|2},
               {"stq", 0xcd, NO_IMMED|PRE_10|2},{"bitmd",0x3c, PRE_11 | 15},
               {"ldbt",0x36, NO_IMMED|PRE_11 | 17},
               {"stbt",0x37, NO_IMMED|PRE_11 | 17},
               {"addr",0x30, PRE_10 | 20},      {"adcr",0x31, PRE_10 | 20},
               {"subr",0x32, PRE_10 | 20},      {"sbcr",0x33, PRE_10 | 20},
               {"andr",0x34, PRE_10 | 20},      {"orr", 0x35, PRE_10 | 20},
               {"eorr",0x36, PRE_10 | 20},      {"cmpr",0x37, PRE_10 | 20},
               {"add", 0x8b, 21},               {"ldmd",0x3d, PRE_11 | 15},
               {"cmp", 0x81, 21},               {"sub", 0x80, 21},
               {"sbc", 0x82, 3},                {"and", 0x84, 3},
               {"bit", 0x85, 3},                {"ld",  0x86, 21},
               {"st",  0x87, 3},                {"eor", 0x88, 3},
               {"adc", 0x89, 3},                {"org", 0x08, 12},
               {"or",  0x8a, 3},                {"neg", 0x00, 4},
               {"common",0x0b,11},              {"com", 0x03, 4},
               {"lsr", 0x04, 4},                {"ror", 0x06, 4},
               {"asr", 0x07, 4},                {"lsl", 0x08, 4},
               {"asl", 0x08, 4},                {"rol", 0x09, 4},
               {"dec", 0x0a, 4},
               {"inc", 0x0c, 4},                {"tst", 0x0d, 4},
               {"jmp", 0x0e, NO_IMMED|4},       {"clr", 0x0f, 4},
               {"rts", 0x39, 5},                {"mul", 0x3d, 5},
               {"nop", 0x12, 5},                {"sync",0x13, 5},
               {"sexw",0x14, 5},                {"daa", 0x19, 5},
               {"sex", 0x1d, 5},                {"pshsw",0x38,PRE_10|5},
               {"pulsw",0x39,PRE_10 | 5},       {"pshuw",0x3a,PRE_10|5},
               {"puluw",0x3b,PRE_10 | 5},       {"abx", 0x3a, 5},
               {"rti", 0x3b, 5},                {"swi2",0x3f, PRE_10|5},
               {"swi3",0x3f, PRE_11 | 5},       {"swi", 0x3f, 5},
               {"leax",0x30, 6},                {"leay",0x31, 6},
               {"leas",0x32, 6},                {"leau",0x33, 6},
               {"tfr", 0x1f, 7},                {"exg", 0x1e, 7},
               {"pshs",0x34, 8},                {"puls",0x35, 8},
               {"pshu",0x36, 8},                {"pulu",0x37, 8},
               {"lb",  0x00, PRE_10 | 9},       {"fcc", 0x01, 11},
               {"fdb", 0x02, 11},               {"fcs", 0x03, 11},
               {"fcb", 0x04, 11},               {"rzb", 0x0a, 11},
               {"vsect",0x06,11},               {"csect",0x00,13},
               {"ends",0x07, 11},               {"setdp",0x07,12},
               {"os9", 0x09, 11},               {"tfm", 0x38, PRE_11|16},
               {"band",0x30, NO_IMMED|PRE_11 | 17},
               {"biand",0x31, NO_IMMED|PRE_11 | 17},
               {"bor", 0x32, NO_IMMED|PRE_11 | 17},
               {"bior", 0x33, NO_IMMED|PRE_11 | 17},
               {"beor",0x34, NO_IMMED|PRE_11 | 17},
               {"bieor",0x35, NO_IMMED|PRE_11 | 17},
               {"oim", 0x01, NO_IMMED|18},      {"aim",  0x02, NO_IMMED|18},
               {"eim", 0x05, NO_IMMED|18},      {"tim",  0x0b, NO_IMMED|18}
}
#endif
;

GLOBAL CODFORM S_Rmb[]
#ifdef MAIN
= {         {"rmb",0x00,0},            {"ends",0x00,1} }
#endif
;

GLOBAL CODFORM Data_Equ[]
#ifdef MAIN
= {
               {"rmb",0x00,0}, {"fcc",0x01,0},
               {"fdb",0x02,0}, {"fcs",0x03,0},
               {"fcb",0x04,0}, {"rzb",0x0a,0},
               {"ends",0x00,1}
}
#endif
;

GLOBAL CODFORM S_Rmb3[]
#ifdef MAIN
= {             {"rmb",0x00,0},     {"ends",0x00,1} }
#endif
;

GLOBAL CODFORM S_Nam[]
#ifdef MAIN
= {
             {"nam",0x00,0},    {"opt",0x01,0},
             {"ttl",0x02,0},    {"pag",0x03,0},
             {"spc",0x04,0},    {"use",0x05,0},
             {"fail",0x09,0},   {"rept",0x0a,0},
             {"endr",0x0b,0},   {"ifeq",0x00,1},
             {"ifne",0x01,1},   {"iflt",0x02,1},
             {"ifle",0x03,1},   {"ifge",0x04,1},
             {"ifgt",0x05,1},   {"ifp1",0x06,1},
             {"endc",0x00,2},   {"else",0x01,2},
             {"equ",0x05,3},    {"set",0x08,3},
             {"macro",0x00,4},  {"endm",0x00,5}
}
#endif
;

GLOBAL CODFORM S_Bsr[]
#ifdef MAIN
= {
             {"bsr",0x8d,10},  {"bra",0x20,10},
             {"brn",0x21,10},  {"bhi",0x22,10},
             {"bls",0x23,10},  {"bhs",0x24,10},
             {"bcc",0x24,10},  {"blo",0x25,10},
             {"bcs",0x25,10},  {"bne",0x26,10},
             {"beq",0x27,10},  {"bvc",0x28,10},
             {"bvs",0x29,10},  {"bpl",0x2a,10},
             {"bmi",0x2b,10},  {"bge",0x2c,10},
             {"blt",0x2d,10},  {"bgt",0x2e,10},
             {"ble",0x2f,10}
}
#endif
;

GLOBAL direct CODFORM *PSecEnd
#ifdef MAIN
= endof(S_Psec)
#endif
,
            *SLbrEnd
#ifdef MAIN
= endof(S_Lbr)
#endif
,
            *S_Rmbend
#ifdef MAIN
= endof(S_Rmb)
#endif
,
            *DtaEqEnd
#ifdef MAIN
= endof(Data_Equ)
#endif
,
            *Rmb3End
#ifdef MAIN
= endof(S_Rmb3)
#endif
,
            *SNamEnd
#ifdef MAIN
= endof(S_Nam)
#endif
,
            *SBsr_End
#ifdef MAIN
= endof(S_Bsr)
#endif
;

	/* The next section comes from part3.c */

int    asm_dirct(), do_ifs(), do_endc(),
       equset(), setmac(), noMac(),
       _psect(), do_csec(), setrmb3(),
       l_brnch(), cc_stuff(), int_stuf(),
       chr_stuf(), bit_stuf(), no_opcod(),
       lea_s(), tfr_exg(), stk_stuf(),
       lb_(), do_brnch(), equset(),
       asm_dirct(), l0ef0(),
       codsetup(), setsect(), q_immed(),
       do_md(),do_tfm(), do_band(),
       do_aim(), do_divd(), do_addr(),
       chr_ef();

GLOBAL JMPTBL Drectives[]
#ifdef MAIN
={
       asm_dirct, do_ifs, do_endc,
       equset, setmac, noMac
}
#endif
;
GLOBAL JMPTBL j_secs[]
#ifdef MAIN
= {
       _psect, do_csec,
       setrmb3, asm_dirct
}
#endif
;
GLOBAL JMPTBL codtabl[]
#ifdef MAIN
= {
       l_brnch,   cc_stuff,  int_stuf,
       chr_stuf,  bit_stuf,  no_opcod,
       lea_s,     tfr_exg,   stk_stuf,
       lb_,       do_brnch,  equset,
       asm_dirct, do_csec,   q_immed,
       do_md,     do_tfm,    do_band,
       do_aim,    do_divd,   do_addr,
       chr_ef
}
#endif
;
GLOBAL JMPTBL d035c[]
#ifdef MAIN
= {
        equset, l0ef0,
        equset, codsetup
}
#endif
;

GLOBAL JMPTBL d0364[]
#ifdef MAIN
= { equset, setsect }
#endif
;

	/* Following is from part5.c */

int    l16f3(), set_fcc(), set_fdb(), set_fcs(),
       set_fcb(), do_equ(), set_vsec(), do_set(),
       do_os9(), do_rzb(), do_comon(), do_nam(),
       set_opts(), do_ttl(), do_pag(), do_spc(),
       do_use(), l199e(), l1ae2(), l198d(),
       setsect(),
       tel_fail(), do_rept(), end_rept(), _isnul(),
       not_nul(), is_neg(), le_zero(),
       ge_zero(), is_pos(), is_pass1();

GLOBAL JMPTBL CnstTbl[]
#ifdef MAIN
= {
        l16f3, set_fcc, set_fdb,
        set_fcs, set_fcb, do_equ,
        set_vsec, setsect, do_set,
        do_os9, do_rzb, do_comon
}
#endif
;

GLOBAL JMPTBL d03b6[]
#ifdef MAIN
= {
        do_nam,    set_opts, do_ttl,
        do_pag,    do_spc,    do_use,
        l199e,    l1ae2,    l198d,
        tel_fail, do_rept,  end_rept
}
#endif
;

GLOBAL JMPTBL _if_tbl[]
#ifdef MAIN
= {
        _isnul,  not_nul, is_neg,
        le_zero, ge_zero,
        is_pos, is_pass1
}
#endif
;

