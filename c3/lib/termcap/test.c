#define unix
#ifdef unix
static char term_buffer[2048];
#else
#define term_buffer 0
#endif
#ifdef UNIX
/* Here we assume that an explicit term_buffer
   was provided to tgetent.  */
char *buffer = (char *) malloc (strlen (term_buffer));
#define BUFFADDR &buffer
#else
#define BUFFADDR 0
#endif
char *temp;
char *tgetstr ();
char *cl_string, *cm_string;
int height;
int width;
int auto_wrap;

char *BC;  /* For tgoto.  */
char *UP;


main()
{
     char *termtype = "tvi925";
     int success;

     if (termtype == 0) {
          printf("Specify a terminal type with `setenv TERM <yourtype>'.\n");
          exit(0);
     }

     success = tgetent (term_buffer, termtype);

     if (success < 0) {
          printf("Could not access the termcap data base.\n");
          exit(0);
     }
     if (success == 0) {
          printf("Terminal type `%s' is not defined.\n", termtype);
          exit(0);
     }
     /* Extract information we will use.  */
     cl_string = tgetstr ("cl", BUFFADDR);
     cm_string = tgetstr ("cm", BUFFADDR);
     auto_wrap = tgetflag ("am");
     height = tgetnum ("li");
     width = tgetnum ("co");

     printf("%d  %d\n", height, width);

     /* Extract information that termcap functions use.  */
/*     temp = tgetstr ("pc", BUFFADDR);
     PC = temp ? *temp : 0;
     BC = tgetstr ("le", BUFFADDR);
     UP = tgetstr ("up", BUFFADDR);
*/
}
