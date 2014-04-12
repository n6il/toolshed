/********************************************************************
 * cecbbulkerase.c - Image creation for Cassette BASIC
 *
 * $Id$
 ********************************************************************/
#include <util.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cocotypes.h>
#include <cecbpath.h>

int sample_rate = 22050;
int bits_per_sample = 8;
double silence_length = 0.5;

static int do_bulkerase(char **argv, char *p);

/* Help message */
static char const * const helpMessage[] =
{
	"Syntax: bulkerase {[<opts>]} {<file> [<...>]} {[<opts>]}\n",
	"Usage:  Create cassette image files, WAV for CAS.\n",
	"Options:\n",
	"     -s<num>  = Sample rate of WAV file (11025, 22050, 44100, etc. Default: 22050).\n",
	"     -b<num>  = Bits per sample of WAV file (8 or 16, default: 8).\n",
	"     -l<num>  = Length of silence to record in WAV file. (default: 0.5 seconds).\n",
	NULL
};


int cecbbulkerase(int argc, char *argv[])
{
	char *p = NULL;
	int i;


	/* 1. Walk command line for options. */

	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			for (p = &argv[i][1]; *p != '\0'; p++)
			{
				switch (*p)
				{
					case 'l':
						silence_length = atof(p + 1);
						while (*(p + 1) != '\0') p++;
						break;

					case 's':
						sample_rate = atoi(p + 1);
						while (*(p + 1) != '\0') p++;
						break;

					case 'b':
						bits_per_sample = atoi(p + 1);
						while (*(p + 1) != '\0') p++;
						
						if( (bits_per_sample != 8) && (bits_per_sample != 16 ) )
							bits_per_sample = 8;
							
						break;

					case 'h':
					case '?':
						show_help(helpMessage);
						return(0);
	
					default:
						fprintf(stderr, "%s: unknown option '%c'\n", argv[0], *p);
						return(0);
				}
			}
		}
	}


	/* 2. Walk command line for pathnames. */
	
	for (i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			continue;
		}
		else
		{
			p = argv[i];

			do_bulkerase(argv, p);
			printf("\n");
		}
	}

	if (p == NULL)
	{
		show_help(helpMessage);
		return(0);
	}


	return(0);
}
	

static int do_bulkerase(char **argv, char *p)
{
	error_code	ec = 0;
	native_path_id nativepath;
	int i, headers_size, bytes_per_sample, silent_samples_count, silent_samples_bytes;

	_native_truncate(p, 0);
	
	/* 1. Open a path to the cassette image. */
	
	ec = _native_open(&nativepath, p, FAM_WRITE);

	if (ec != 0)
	{
		ec = _native_create(&nativepath, p, FAM_READ | FAM_WRITE, FAP_READ | FAP_WRITE);

		if (ec != 0)
		{
			fprintf(stderr, "%s: cannot open virtual cassette\n", argv[0]);

			return(ec);
		}
	}

	_native_seek(nativepath, 0, SEEK_SET);
	
	if( strendcasecmp( p, CAS_FILE_EXTENSION ) == 0 )
		return 0;
		
	printf( "Creating WAV file: %s\n", p );
	printf( "      Sample Rate: %d\n", sample_rate );
	printf( "  Bits Per Sample: %d\n", bits_per_sample );
	printf( "   Silence Length: %f\n", silence_length );
	

	bytes_per_sample = bits_per_sample / 8;
	silent_samples_count = sample_rate * silence_length;
	silent_samples_bytes = silent_samples_count * bytes_per_sample;
	
	headers_size = 4 +	/* RIFF */
					4 +			/* Data size */
					4 +			/* RIFF type */
					4 +			/* fmt  chunk id */
					4 +			/* fmt  chunk size */
					18 +		/* fmt  chunk data */
					4 +			/* data chunk id */
					4 +			/* data chunk size */
					silent_samples_bytes;

	/* Set up WAV file format header */

	fwrite("RIFF", 4, 1, nativepath->fd);
	fwrite_le_int(headers_size - 8, nativepath->fd);
	fwrite("WAVE", 4, 1, nativepath->fd);

	fwrite("fmt ", 4, 1, nativepath->fd);
	fwrite_le_int(16, nativepath->fd);	/* chunk size */
	fwrite_le_short(1, nativepath->fd);	/* compression code: uncompressed */
	fwrite_le_short(1, nativepath->fd);	/* number of channels */
	fwrite_le_int(sample_rate, nativepath->fd);	/* sample rate */
	fwrite_le_int(sample_rate * bytes_per_sample, nativepath->fd);	/* average bytes per second */
	fwrite_le_short(1, nativepath->fd);	/* block align */
	fwrite_le_short(bits_per_sample, nativepath->fd);	/* significant bits per sample */

	fwrite("data", 4, 1, nativepath->fd);
	fwrite_le_int(silent_samples_bytes, nativepath->fd);	/* chunk size */
	
	for( i=0; i<silent_samples_count; i++ )
	{
		if( bits_per_sample == 8 )
			fwrite_le_char(127, nativepath->fd);
		else
			fwrite_le_short(0, nativepath->fd);
	}

	_native_close(nativepath);

	return 0;
}
