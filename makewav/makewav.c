/*
	makewav.c
	
	Motorola S Record (and raw binary) to CoCo WAV file
	
	This program will convert a Motorola S record file to
	a WAV file. The format will match Microsoft's Color BASIC and Micro
	Color BASIC cassette format.
	
	A raw binary can be encoded also, the meta data should be taken from
	the command line.
	
	Only S0 records are supported, others are ignored. Execution address
	is taken from the first S0 record.
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <strings.h>
#include <libgen.h>
#include <stdlib.h>
#include <math.h>

#if defined(__linux__) || defined(__CYGWIN__) || defined(BDS)
/* implemented based on OSX man page */
static inline int digittoint(int c)
{
	/* if not 0-9, a-f, or A-F then return 0 */
	if (!isxdigit(c))
		return 0;

	if (isdigit(c))
		return c - '0';

	if (isupper(c))
		return c - 'A' + 10;

	/* not 0-9, not A-F, must be a-f */
	return c - 'a' + 10;
}
#endif

#define MAXPATHLEN 512
#define PI 3.1415926

int             seconds;
int             sample_rate;
int             binary;
char            filename[9];
unsigned char   file_type;
unsigned char   data_type;
char            out_filename[MAXPATHLEN];
char            in_filename[MAXPATHLEN];
int             verbose;
short           start_address;
short           exec_address;

unsigned char  *buffer_1200,
               *buffer_2400;
int             buffer_1200_length,
                buffer_2400_length;

unsigned short  swap_short(unsigned short in)
{
	unsigned short  out = (in << 8) + (in >> 8);

	return out;
}

unsigned int    swap_int(unsigned int in)
{
	unsigned int    out = swap_short(in >> 16) + (swap_short(in & 0x0000ffff) << 16);

	return out;
}

void            fwrite_le_int(unsigned int data, FILE * output)
{
#ifdef __BIG_ENDIAN__
	unsigned int    use_data = swap_int(data);

#else
	unsigned int    use_data = data;

#endif

	fwrite(&use_data, 4, 1, output);
}

void            fwrite_le_short(unsigned short data, FILE * output)
{
#ifdef __BIG_ENDIAN__
	unsigned short  use_data = swap_short(data);

#else
	unsigned short  use_data = data;

#endif
	fwrite(&use_data, 2, 1, output);
}

int             fwrite_audio(char *buffer, int total_length, FILE * output)
{
	int             result = 0,
	                i;

	for (i = 0; i < total_length; i++)
	{
		int             j;

		for (j = 0; j < 8; j++)
		{
			if (((buffer[i] >> j) & 0x01) == 0)
			{
				fwrite(buffer_1200, buffer_1200_length, 1, output);
				result += buffer_1200_length;
			}
			else
			{
				fwrite(buffer_2400, buffer_2400_length, 1, output);
				result += buffer_2400_length;
			}
		}
	}

	return result;
}

int             fwrite_repeat_byte(int length, unsigned char byte, FILE * output)
{
	int             i;

	for (i = 0; i < length; i++)
	{
		fputc(byte, output);
	}

	return length;
}

int             fwrite_audio_repeat_byte(int length, char byte, FILE * output)
{
	int             i,
	                result = 0;

	for (i = 0; i < length; i++)
		result += fwrite_audio(&byte, 1, output);

	return result;
}

unsigned char   Checksum_Buffer(unsigned char *buffer, int count)
{
	unsigned char   result;

	int             i;

	for (result = 0, i = 0; i < count; i++)
		result += buffer[i];

	return result;
}

void            Build_Sinusoidal_Bufer(unsigned char *buffer, int length)
{
	double          increment = (PI * 2.0) / length;

	int             i;

	for (i = 0; i < length; i++)
	{
		buffer[i] = (sin(increment * i + PI) * 110.0) + 127.0;
	}
}

int             main(int argc, char **argv)
{
	char            linebuf[256];
	int             j;

	/* Initialize globals */
	seconds = 2;
	sample_rate = 11250;
	binary = 0;
	memset(filename, 0, 8);
	strncpy(filename, "FILE", 8);
	file_type = 2;
	data_type = 0;
	strncpy(out_filename, "file.wav", MAXPATHLEN);
	verbose = 0;
	start_address = 0;
	exec_address = 0;
	
	if (argc < 2)
	{
		fprintf(stderr, "\%s - S record to CoCo/MC-10 audio WAV file\n", argv[0]);
		fprintf(stderr, "Copyright (C) 2007 tim lindner\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "This program will convert a Motorola S record file to\n");
		fprintf(stderr, "a WAV file. The format will match Microsoft's Color BASIC and Micro\n");
		fprintf(stderr, "Color BASIC cassette format.\n");
		fprintf(stderr, "\n");
		fprintf(stderr, "General options:\n");
		fprintf(stderr, " -l<val>    Length for silent leader (default %d seconds)\n", seconds);
		fprintf(stderr, " -s<val>    Sample rate for WAV file (default %d samples per second)\n", sample_rate);
		fprintf(stderr, " -r         Treat input file as raw binary, not an S Record file.\n" );
		fprintf(stderr, " -n<string> Filename to encode in header (default: %s)\n", filename);
		fprintf(stderr, " -[0-2]     File type (default %d)\n", file_type);
		fprintf(stderr, "            0 = BASIC program\n");
		fprintf(stderr, "            1 = BASIC data file\n");
		fprintf(stderr, "            2 = Machine language program\n");
		fprintf(stderr, " -[a|b]     Data type (a = ASCII, b=binary (default: %s)\n", data_type == 0 ? "binary" : "ASCII");
		fprintf(stderr, " -d<val>    Start address (default: 0x%x)\n", start_address );
		fprintf(stderr, " -e<val>    Execution address (default: 0x%x)\n", exec_address );
		fprintf(stderr, " -o<string> Output file name for WAV file (default: %s)\n", out_filename);
		fprintf(stderr, " -v         Print information about the conversion (default: off)\n\n");
		fprintf(stderr, "For <val> use 0x prefix for hex, 0 prefix for octal and no prefix for decimal.\n");

		exit(1);
	}

	for (j = 1; j < argc; j++)
	{
		if (*argv[j] == '-')
		{
			switch (tolower(argv[j][1]))
			{
			case 'l':
				seconds = strtol(&(argv[j][2]), NULL, 0);
				break;
			case 's':
				sample_rate = strtol(&(argv[j][2]), NULL, 0);
				break;
			case 'r':
				binary = 1;
				break;
			case 'n':
				memset(filename, 0, 8);
				strncpy(filename, &(argv[j][2]), 8);
				break;
			case '0':
				file_type = 0;
				break;
			case '1':
				file_type = 1;
				break;
			case '2':
				file_type = 2;
				break;
			case 'a':
				data_type = 0;
				break;
			case 'b':
				data_type = 0xff;
				break;
			case 'o':
				strncpy(out_filename, &(argv[j][2]), MAXPATHLEN);
				break;
			case 'v':
				verbose = 1;
				break;
			case 'd':
				start_address = strtol(&(argv[j][2]), NULL, 0);
				break;
			case 'e':
				exec_address = strtol(&(argv[j][2]), NULL, 0);
				break;
			default:
				/* Bad option */
				fprintf(stderr, "Unknown option\n");
				exit(0);
			}
		}
		else
		{
			strncpy(in_filename, argv[j], MAXPATHLEN);
		}

	}

	if (verbose)
	{
		printf("makewav - S record to CoCo/MC-10 audio WAV file\n");
		printf("Copyright (C) 2007 tim lindner\n\n");
	}

	/* Open S Record file */

	FILE           *srec = fopen(in_filename, "r");

	if (srec == NULL)
	{
		fprintf(stderr, "Unable to open S Record file name %s\n\n", in_filename);
		return -1;
	}
	
	/* Compute length of data in entire S record */
	int             total_length;

	if( binary )
	{
		/* binary file */
		fseek( srec, 0, SEEK_END );
		total_length = ftell( srec );
	}
	else
	{
		/* srecord */
		total_length = 0;

		while (fgets(linebuf, 255, srec) != NULL)
		{
			if (linebuf[0] != 'S')
			{
				fprintf(stderr, "Not an S record file, Line does not begin with 'S'\n\n");
				return -1;
			}

			if (linebuf[1] != '1')
			{
				if (verbose)
					fprintf(stderr, "Warning: Skipping a non S1 record\n");

				continue;
			}

			total_length += (digittoint(linebuf[2]) * 16) + digittoint(linebuf[3]) - 3;
		}
	}

	if (verbose)
		printf("Total data length is %d bytes\n", total_length);

	/* Load buffer with all data from file */

	char           *buffer = malloc(total_length);
	int             byte = 0;

	if (buffer == NULL)
	{
		fprintf(stderr, "Unable to create buffer for S record data.\n\n");
		return -1;
	}

	rewind( srec );

	if( binary )
	{
		/* binary file */

		int	value;
		value = fread( buffer, total_length, 1, srec );
		
		if( value != 1 )
		{
			fprintf( stderr, "Could not read entire input file. (%d != 1)\n\n", value );
			return -1;
		}
	}
	else
	{
		/* srecord file */
		int             line_data_length,
						program_counter = 0,
						line_address = 0;

		while (fgets(linebuf, 255, srec) != NULL)
		{
			if (linebuf[0] != 'S')
			{
				fprintf(stderr, "Not an S record file, Line does not begin with 'S'\n\n");
				return -1;
			}

			if (linebuf[1] != '1')
			{
				if (verbose)
					fprintf(stderr, "Warning: Skipping a non S1 record\n");

				continue;
			}

			line_address = (digittoint(linebuf[4]) * 4096) + (digittoint(linebuf[5]) * 256) +
				(digittoint(linebuf[6]) * 16) + digittoint(linebuf[7]);

			if (program_counter != 0)
			{
				if (program_counter + line_data_length != line_address)
				{
					fprintf(stderr, "Discontinus S Records not allowed.\n\n");
					return -1;
				}
			}
			else
			{
				if( start_address == 0 )
					start_address = line_address;
					
				if( exec_address == 0 )
					exec_address = line_address;
			}

			line_data_length = (digittoint(linebuf[2]) * 16) + digittoint(linebuf[3]) - 3;
			program_counter = line_address;

			int             i;

			for (i = 8; i < 8 + (line_data_length * 2); i += 2)
			{
				buffer[byte++] = (digittoint(linebuf[i]) * 16) + digittoint(linebuf[i + 1]);
			}
		}

		if (byte != total_length)
		{
			fprintf(stderr, "Data buffer not filled completely (%d != %d).\n\n", total_length, byte);
			return -1;
		}
	}
	
	FILE           *output = fopen(out_filename, "w+");

	if (output == NULL)
	{
		fprintf(stderr, "Could not open/create %s\n\n", out_filename);
		return -1;
	}

	/* Defined values */
//	buffer_1200_length = (double)sample_rate / 1200.0;
//	buffer_2400_length = (double)sample_rate / 2400.0;

	/* Using emperical measurment */
	buffer_1200_length = (double)sample_rate / 1094.68085106384;
	buffer_2400_length = (double)sample_rate / 2004.54545454545;
	
	if( verbose )
	{
		printf( "sample size: 1200 hertz: %d, 2400 hertz: %d\n", buffer_1200_length, buffer_2400_length );
	}
	
	buffer_1200 = malloc(buffer_1200_length);

	if (buffer_1200 == NULL)
	{
		fprintf(stderr, "Could not allocate memory for 1200 hertz buffer\n");
		return -1;
	}

	Build_Sinusoidal_Bufer(buffer_1200, buffer_1200_length);

	buffer_2400 = malloc(buffer_2400_length);

	if (buffer_2400 == NULL)
	{
		fprintf(stderr, "Could not allocate memory for 2400 hertz buffer\n");
		return -1;
	}

	Build_Sinusoidal_Bufer(buffer_2400, buffer_2400_length);

	int             headers_size = 4 +	/* RIFF */
							4 +			/* Data size */
							4 +			/* RIFF type */
							4 +			/* fmt  chunk id */
							4 +			/* fmt  chunk size */
							18 +			/* fmt  chunk data */
							4 +			/* data chunk id */
							4;			/* data chunk size */
	int             sample_count = 0;

	/* Set up WAV file format header */

	fwrite("RIFF", 4, 1, output);
	fwrite_le_int(headers_size - 8, output);
	fwrite("WAVE", 4, 1, output);

	fwrite("fmt ", 4, 1, output);
	fwrite_le_int(16, output);	/* chunk size */
	fwrite_le_short(1, output);	/* compression code: uncompressed */
	fwrite_le_short(1, output);	/* number of channels */
	fwrite_le_int(sample_rate, output);	/* sample rate */
	fwrite_le_int(sample_rate * 1, output);	/* average bytes per second */
	fwrite_le_short(1, output);	/* block align */
	fwrite_le_short(8, output);	/* significant bits per sample */

	fwrite("data", 4, 1, output);
	fwrite_le_int(0, output);	/* chunk size */

	/* Color BASIC and Micro Color BASIC */
	/* Leader */
	sample_count += fwrite_repeat_byte(sample_rate * seconds, 0x80, output);	/* seconds of silence */
	sample_count += fwrite_audio_repeat_byte(128, 0x55, output);	/* leader */

	/* Header block */
	unsigned char   checksum = 0 + 0x0f + Checksum_Buffer((unsigned char *) filename, 8) +
							file_type + data_type + 0 +
							Checksum_Buffer((unsigned char *) &start_address, 2) +
							Checksum_Buffer((unsigned char *) &exec_address, 2);

	sample_count += fwrite_audio("\x55\x3c", 2, output);	/* Block ID */
	sample_count += fwrite_audio("\x00", 1, output);	/* Header block */
	sample_count += fwrite_audio("\x0f", 1, output);	/* Block length */
	sample_count += fwrite_audio(filename, 8, output);	/* File name */
	sample_count += fwrite_audio((char *) &file_type, 1, output);	/* 0: BASIC, 1: Data, 2: M/L */
	sample_count += fwrite_audio((char *) &data_type, 1, output);	/* 0: binary, ff: ASCII */
	sample_count += fwrite_audio("\x00", 1, output);	/* 0: no gaps, ff: gaps) */
	sample_count += fwrite_audio((char *) &start_address, 2, output);	/* Start address */
	sample_count += fwrite_audio((char *) &exec_address, 2, output);	/* Execute address */
	sample_count += fwrite_audio((char *) &checksum, 1, output);	/* checksum */
	sample_count += fwrite_audio("\x55", 1, output);	/* End of block ID */

	if( verbose )
	{
		printf( "Encoded filename: %s\n", filename );
		printf( "File type: 0x%x\n", file_type );
		printf( "Data type: 0x%x\n", data_type );
		printf( "Start address: 0x%x\n", start_address );
		printf( "Exec address: 0x%x\n", exec_address );
		printf( "End address: 0x%x\n", start_address+total_length );
	}
	
	/* Leader for data blocks */
	sample_count += fwrite_repeat_byte(sample_rate / 2, 0x80, output);	/* half second of silence */
	sample_count += fwrite_audio_repeat_byte(128, 0x55, output);	/* leader */

	/* Full data blocks */
	int             full_blocks = total_length / 0xff;

	int             i;

	for (i = 0; i < full_blocks; i++)
	{
		unsigned char   checksum = 1 + 0xff +
		Checksum_Buffer((unsigned char *) &(buffer[i * 0xff]), 0xff);

		sample_count += fwrite_repeat_byte((double)sample_rate * 0.003, 0x80, output);	/* .003 seconds of silence */

		sample_count += fwrite_audio("\x55\x3c\x01\xff", 4, output);	/* Block header, data block and length */
		sample_count += fwrite_audio(&(buffer[i * 0xff]), 0xff, output);	/* data */
		sample_count += fwrite_audio((char *) &checksum, 1, output);	/* checksum */
		sample_count += fwrite_audio("\x55", 1, output);	/* End of block ID */
	}

	/* Last data block */
	unsigned char   last_block_size = total_length - (0xff * full_blocks);

	if (last_block_size > 0)
	{
		unsigned char   checksum = 1 + last_block_size +
		Checksum_Buffer((unsigned char *) &(buffer[0xff * full_blocks]), last_block_size);

		sample_count += fwrite_repeat_byte((double)sample_rate * 0.003, 0x80, output);	/* .003 seconds of silence */

		sample_count += fwrite_audio("\x55\x3c\x01", 3, output);	/* Block header, data block and length */
		sample_count += fwrite_audio((char *) &last_block_size, 1, output);	/* Block Length */
		sample_count += fwrite_audio(&(buffer[0xff * full_blocks]), last_block_size, output);	/* data */
		sample_count += fwrite_audio((char *) &checksum, 1, output);	/* checksum */
		sample_count += fwrite_audio("\x55", 1, output);	/* End of block ID */
	}

	/* Eof block */
	sample_count += fwrite_repeat_byte((double)sample_rate * 0.003, 0x80, output);	/* .003 seconds of silence */
	sample_count += fwrite_audio("\x55\x3c\xff\x00\xff\x55", 6, output);
	sample_count += fwrite_repeat_byte(sample_rate * 2, 0x80, output);	/* 2 seconds of silence */

	/* Go back and fix up WAV format file size headers */
	fseek(output, 4, SEEK_SET);
	fwrite_le_int(headers_size + sample_count - 8, output);
	fseek(output, 40, SEEK_SET);
	fwrite_le_int(sample_count, output);

	fclose(output);
	fclose(srec);
	free(buffer_1200);
	free(buffer_2400);
	free(buffer);
	
	return 0;
}
