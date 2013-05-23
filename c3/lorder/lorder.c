/*
  The following (not a shell archive, since it's only one file) is a
  program that does for Microware C relocatable and library files what
  lorder(1) does for Unix .o files.

  The following brash one-liner makes fun of condescending Unix
  documentation: "Take my wife--please!"

  In conjunction with previously posted stuff (LibSplit), I'm going to
  use this to shuffle the Microware C library, clib.l, to see if it makes
  any difference in speed of compilation. If it helps, I'll let you
  know.

  BTW--I know I took the sleazy way out on looking for duplications in
  ShowDep(). I probably should have kept a high-water mark on ECount
  and allocated that much (it's just a one-shot anyway). Sigh...

                                        James Jones
 */

/*
 * LOrder -- display dependencies among pieces of relocatable and
 *      library files
 *
 * usage: LOrder <pathname> [<pathname> ...]
 *
 * semantics: LOrder reads the specified files and looks at the
 *      global symbols used and defined by the parts thereof. It
 *      then writes to standard output a sequence of lines of the
 *      form "m1 m2," where m1 is the name of a "module" that refers
 *      to a global symbol defined in the "module" m2.
 */

#include <stdio.h>
#include <stdlib.h>
#include <rof.h>

/* what we keep track of for symbols */

typedef struct module {
        char            m_name[MAXNAME];
        struct global   **ref_list;
        struct module   *m_next;
} Module;

typedef struct global {
        char            s_name[MAXNAME];
        struct module   *def_by;
        struct global   *s_next;

} Global;

#define HASHSIZE        151     /* size of symbol hash table */
#define MODULE          1       /* tag for "modules" */
#define GLOBAL          0       /* and for globals */
#define MAXREFS         40      /* arbitrary bound on external references */


binhead header;
FILE    *LibFP;
Global  *HashTable[HASHSIZE];
Module  *MList;

main(argc, argv)
int     argc;
char    *argv[];
{
        int     i;

        if (argc < 2) {
                fprintf(stderr, "usage: LOrder pathname [pathname ...]\n");
                exit(1);
        }

        MList = NULL;
        for (i = 0; i < HASHSIZE; i++)
                HashTable[i] = NULL;

        for (i = 1; i < argc; i++)
                DoLibFile(argv[i]);

        ShowDep();

}

DoLibFile(LibPath)
char    *LibPath;
{
        Module  *NewMod, *GetModule();

        if ((LibFP = fopen(LibPath, "r")) == NULL) {
                fprintf(stderr, "LOrder: can't open %s\n", LibPath);
                exit(1);
        }

        while (fread(&header, sizeof(header), 1, LibFP) > 0) {
                if (header.h_sync != ROFSYNC) {
                        fprintf(stderr, "%s is not a library file\n", LibPath);
                        exit(1);
                }
                NewMod = GetModule();
                DoGlobalDefs(NewMod);
                fseek(LibFP,
                        (long) header.h_ocode + header.h_ddata + header.h_data,
                        1);
                DoExtRefs(NewMod);
                SkipRefs();     /* local references */
        }

        fclose(LibFP);

}

DoGlobalDefs(Mod)
Module  *Mod;
{
        int     GCount;
        Global  *GSym, *GetGlobal();

        GCount = getw(LibFP);

        for (; GCount > 0; GCount--) {
                GSym = GetGlobal();
                GSym->def_by = Mod;
                fseek(LibFP, (long)(sizeof(char) + sizeof(int)), 1);
        }

}

DoExtRefs(Mod)
Module  *Mod;
{
        int     i, ECount;
        Global  *GetGlobal(), **rlist;

        ECount = getw(LibFP);

        rlist = calloc(ECount + 1, sizeof(Global *));
        if (rlist == NULL) {
                exit(1);
        }
        Mod->ref_list = rlist;

        for (i = 0; i < ECount; i++) {
                rlist[i] = GetGlobal();
                SkipRefs();
        }

}

SkipRefs()
{
        int     RCount;

        RCount = getw(LibFP);
        fseek(LibFP, (long) (RCount * sizeof(def_ref)), 1);

}

Global *
GetGlobal()
{
        char    NameIn[MAXNAME];
        Global  *NewGlob;
        int     i, hash;

        for (i = 0; NameIn[i] = getc(LibFP); i++)
                ;

        for (hash = i = 0; NameIn[i]; i++)
                hash ^= NameIn[i] << (i & 7);
        hash = hash % HASHSIZE;

        for (NewGlob = HashTable[hash]; NewGlob != NULL; ) {
                if (strcmp(NameIn, NewGlob->s_name) == 0) {
                        return(NewGlob);
                }
                NewGlob = NewGlob->s_next;
        }

        if ((NewGlob = malloc(sizeof(Global))) == NULL) {
                fprintf(stderr, "LOrder: out of memory\n");
                exit(1);
        }
        strcpy(NewGlob->s_name, NameIn);
        NewGlob->def_by = NULL;
        NewGlob->s_next = HashTable[hash];
        HashTable[hash] = NewGlob;
        return(NewGlob);

}

Module *
GetModule()
{
        Module  *NewMod;
        char    *scan;

        if ((NewMod = malloc(sizeof(Module))) == NULL) {
                fprintf(stderr, "LOrder: out of memory\n");
                exit(1);
        }
        for (scan = NewMod->m_name; *scan = getc(LibFP); scan++)
                ;
        NewMod->ref_list = NULL;
        NewMod->m_next = MList;
        MList = NewMod;
        return(NewMod);

}

ShowDep()
{
        int     i, j, rcount;
        Global  **rlist;
        Module  *scan, *deplist[MAXREFS];

        for (scan = MList; scan != NULL; scan = scan->m_next) {
                rcount = 0;
                rlist = scan->ref_list;
                for (i = 0; rlist[i] != NULL; i++) {
                        if ((rlist[i])->def_by == NULL)
                                fprintf(stderr, "LOrder: %s is not defined\n",
                                        (rlist[i])->s_name);
                        else {
                                deplist[rcount] = (rlist[i])->def_by;
                                for (j = 0;
                                        deplist[j] != deplist[rcount];
                                        j++)    ;
                                if (j >= rcount)
                                        rcount++;
                        }
                }
                for (j = 0; j < rcount; j++)
                        printf("%s %s\n", scan->m_name, deplist[j]->m_name);
        }
}
