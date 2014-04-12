/********************************************************************
 * os9modbust.c - OS-9 module buster utility
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <cocotypes.h>
#include <cocopath.h>
#include <os9module.h>
#include <cococonv.h>


static int do_modbust(char **argv, char *filename);

/* Help message */
static char const * const helpMessage[] =
{
	"Syntax: modbust {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
	"Usage:  Bust a single merged file of OS-9 modules into separate files.\n",
	"Options:\n",
	NULL
};


int os9modbust(int argc, char **argv)
{
	error_code	ec = 0;
	int i;
	char *p = NULL;

	if (argv[1] == NULL)
	{
		show_help(helpMessage);
		return(0);
	}

	/* walk command line for options */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch(*p)
				{
					case '?':
					case 'h':
						show_help(helpMessage);
						return(0);
	
					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return(0);
				}
			}
		}
	}

	/* walk command line for pathnames */
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];
		}

		ec = do_modbust(argv, p);

		if (ec != 0)
		{
			fprintf(stderr, "%s: error %d opening file %s\n",
				argv[0], ec, p);
			return(ec);
		}
	}

	return(0);
}


static int do_modbust(char **argv, char *filename)
{
	error_code	ec = 0;
	char buffer[256];
	os9_path_id path;
	u_char *module;

	ec = _os9_open(&path, filename, FAM_READ);

	if (ec != 0)
	{
		return(ec);
	}
    
	while (_os9_gs_eof(path) == 0)
	{
		int size = 1;

		ec = _os9_read(path, buffer, &size);
		if (ec != 0)
		{
			fprintf(stderr, "%s: error reading file %s\n",
				argv[0], filename);
			return(ec);
		}
     
		if (buffer[0] == '\x87')
		{
			size = 1;

			ec = _os9_read(path, buffer, &size);

			if (buffer[0] == '\xCD')
			{
				os9_path_id path2;
				char name[256];
				int nameoffset;

				/* We have an OS-9 module - get size */
				size = 2;

				ec = _os9_read(path, buffer, &size);
				size = int2(buffer);
				module = (u_char *)malloc(size);
				if (module == NULL)
				{
					printf("Memory allocation error\n");
					return(1);
				}
				module[0] = 0x87;
				module[1] = 0xCD;
				module[2] = buffer[0];
				module[3] = buffer[1];
				size -= 4;
				ec = _os9_read(path, &module[4], &size);
				nameoffset = int2(&module[4]);
				memcpy(name, &module[nameoffset], OS9NameLen(&module[nameoffset]));
				OS9NameToString(name);
				printf("Busting module %s...\n", name);
				ec = _os9_create(&path2, name, FAM_WRITE, FAP_READ | FAP_WRITE);
				if (ec != 0)
				{
					printf("Error creating file %s\n", name);
					return(1);
				}
				size += 4;
				_os9_write(path2, module, &size);
				_os9_close(path2);

				free(module);
			}
		}
	}

	_os9_close(path);

	return(0);
}
