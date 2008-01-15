/********************************************************************
 * libcebcwav.c - Cassette BASIC WAVE file routines
 *
 * $Id$
 ********************************************************************/

#include "math.h"
#include "cecbpath.h"

static error_code analyze_wav_leader( cecb_path_id path );
static double movingavg(int which, double newvalue);
static int numbers_close_double( double a, double b, double p );
static int numbers_close_signed( int a, int b, float p );
static error_code advance_to_next_zero_crossing( cecb_path_id path, int *diff );
static error_code advance_to_next_lo_to_hi( cecb_path_id path, int *diff );
static error_code advance_to_next_hi_to_lo( cecb_path_id path, int *diff );

/*
 * _cecb_read_bits_wav()
 *
 * Read 'count' bits and return the result.
 *
 * Count must be from 1 to 8.
 * Returned bit are left aligned in unsigned char.
 */

error_code _cecb_read_bits_wav( cecb_path_id path, int count, unsigned char *result )
{
	error_code ec = 0;
	double freq;
	int diff;
	
	*result = 0;
	
	while( count > 0 )
	{
		if( path->wav_parity == ODD )
			ec = advance_to_next_hi_to_lo( path, &diff );
		else if( path->wav_parity == EVEN )
			ec = advance_to_next_lo_to_hi( path, &diff );
		else
		{
			*result = 0;
			return EOS_IA;
		}
		
		if( ec != 0 ) return ec;
		
		if( path->wav_current_sample > path->wav_total_samples )
		{
			*result = 0;
			return EOS_EOF;
		}
		
		freq = ((float)path->wav_sample_rate/(float)diff);
		
		if( freq < path->wav_frequency_limit ) /* 1200 Hz range */
			(*result) >>= 1;
		else /* 2400 HZ range */
		{
			(*result) >>= 1;
			(*result) |= 0x80;
		}

		count--;
	}
	
	return 0;
}

/*
 * _cecb_parse_riff()
 *
 * Parse RIFF and WAV headers.
 *
 */

error_code _cecb_parse_riff( cecb_path_id path )
{
	error_code ec = 0;
	u_char data[255];
	int found;
	unsigned int chunk_length;

	if( (fread( data, 1, 4, path->fd ) != 4) || (strncmp( (char *)data, "RIFF", 4 ) != 0) )
		return EOS_WT;

	if( fread_le_int( &(path->wav_riff_size), path->fd ) != 4 )
		return EOS_WT;

	if( (fread( data, 1, 4, path->fd ) != 4) || (strncmp( (char *)data, "WAVE", 4 ) != 0) )
		return EOS_WT;
	
	found = 0;
	
	while( found == 0 )
	{
		if( fread( data, 1, 4, path->fd ) != 4 )
			return EOS_WT;
		
		if( fread_le_int( &chunk_length, path->fd ) != 4 )
			return EOS_WT;
		
		if( strncmp( (char *)data, "fmt ", 4 ) == 0 )
		{
			unsigned short fmt_compression, fmt_channel_count, fmt_block_align;
			unsigned int fmt_bytes_per_second;
			
			if( chunk_length != 16 )
				return EOS_WT;

			if( fread_le_short( &fmt_compression, path->fd ) != 2 )
				return EOS_WT;
			if( fread_le_short( &fmt_channel_count, path->fd ) != 2 )
				return EOS_WT;
			if( fread_le_int( &(path->wav_sample_rate), path->fd ) != 4 )
				return EOS_WT;
			if( fread_le_int( &fmt_bytes_per_second, path->fd ) != 4 )
				return EOS_WT;
			if( fread_le_short( &fmt_block_align, path->fd ) != 2 )
				return EOS_WT;
			if( fread_le_short( &(path->wav_bits_per_sample), path->fd ) != 2 )
				return EOS_WT;
			
			if( fmt_compression != 1 )
				return EOS_WT;
				
			if( fmt_channel_count != 1 )
				return EOS_WT;
				
			if( (path->wav_bits_per_sample != 8) && (path->wav_bits_per_sample != 16) )
				return EOS_WT;

			found = 1;
		}
		else
		{
			char *buffer;
			
			buffer = malloc( chunk_length );
			
			if( buffer == NULL )
				return EOS_MF;
			
			if( fread( buffer, 1, chunk_length, path->fd) != chunk_length )
				return EOS_WT;

			free( buffer );
		}
	}
	
	if( found == 0 )
		return EOS_WT;

	found = 0;
	
	while( found == 0 )
	{
		if( fread( data, 1, 4, path->fd ) != 4 )
			return EOS_WT;
		
		if( fread_le_int( &chunk_length, path->fd ) != 4 )
			return EOS_WT;
		
		if( strncmp( (char *)data, "data", 4 ) == 0 )
		{
			path->wav_data_length = chunk_length;
			path->wav_data_start = ftell(path->fd);
			path->tape_type = WAV;

			found = 1;
		}
	}
	
	if( found == 0 )
		return EOS_WT;
	
	path->wav_total_samples = path->wav_data_length / WAV_SAMPLE_MUL;
	fseek( path->fd, path->play_at * WAV_SAMPLE_MUL, SEEK_CUR );
	path->wav_current_sample = path->play_at;
	
	if( (path->wav_frequency_limit == 0) || (path->wav_parity == NONE) )
		ec = analyze_wav_leader( path );
	
	return ec;
}

/*
 * analyze_leader()
 *
 * Analyze leader to determine frequency limit and wave parity.
 *
 */

static error_code analyze_wav_leader( cecb_path_id path )
{
	error_code ec = 0;
	int i, j;
	int diff1, diff2, diff3, diff4, diff5;
	double ma1, ma2, ma3, ma4, mah, mal, ratio;

	ratio = movingavg( 4, 1.0 ); /* Seed ratio with out of range number */

	if( path->wav_frequency_limit == 0 )
	{
		for( i=0; i<128; i++ ) /* Should be about half the leader */
		{
			for( j=0; j<4; j++ )
			{
				diff1 = diff2;
				diff2 = diff3;
				diff3 = diff4;
				diff4 = diff5;
				ec = advance_to_next_zero_crossing( path, &diff5 );
				
				if( ec != 0 ) return ec;
			}

			ma1 = movingavg( 0, (double)path->wav_sample_rate/(diff1+diff2) );
			ma2 = movingavg( 1, (double)path->wav_sample_rate/(diff2+diff3) );
			ma3 = movingavg( 2, (double)path->wav_sample_rate/(diff3+diff4) );
			ma4 = movingavg( 3, (double)path->wav_sample_rate/(diff4+diff5) );

			mal = fmin( fmin( fmin( ma1, ma2 ), ma3 ), ma4 );
			mah = fmax( fmax( fmax( ma1, ma2 ), ma3 ), ma4 );

			ratio = movingavg( 4, mal / mah );
			
			if( numbers_close_double( ratio, 0.5, 0.1 ) == 1 )
			{
				if( i > 64 ) break;			
			}
		}
		
		path->wav_frequency_limit = (mal + mah)/2.0;
	}
	else
	{
		if( path->wav_parity == NONE )
		{
			fprintf( stderr, "Error: If you set frequency limit, you need to set parity.\n" );
			return EOS_IA;
		}
	}

	if( path->wav_parity == NONE )
	{
		if( numbers_close_double( ratio, 0.5, 0.75 ) == 1 )
		{
			ec = advance_to_next_lo_to_hi( path, &diff1 );
			ec = advance_to_next_zero_crossing( path, &diff1 );
			ec = advance_to_next_zero_crossing( path, &diff2 );
			
			if( ec != 0 ) return ec;
			
			if( numbers_close_signed( diff1, diff2, path->wav_threshold ) == 1 )
				path->wav_parity = EVEN;
			else
				path->wav_parity = ODD;
		}
		else
		{
			fprintf( stderr, "Error: Unable to determine wave parity. Set switch manually.\n" );
			return EOS_IA;
		}
	}

	return ec;
}

#define MACOUNT 5
#define MASIZE 20

static double movingavg(int which, double newvalue)
{
	static double sum[MACOUNT] = {0.0, 0.0, 0.0, 0.0, 0.0};
	static int index[MACOUNT] = {0, 0, 0, 0, 0};
	static double history[MACOUNT][MASIZE] = {{ 0.0, 0.0, 0.0, 0.0, 0.0 },
												{ 0.0, 0.0, 0.0, 0.0, 0.0 },
												{ 0.0, 0.0, 0.0, 0.0, 0.0 },
												{ 0.0, 0.0, 0.0, 0.0, 0.0 },
												{ 0.0, 0.0, 0.0, 0.0, 0.0 } };
	static int full[MACOUNT] = {0, 0, 0, 0, 0};

	if( which < MACOUNT )
	{
		sum[which] -= history[which][index[which]];
		sum[which] += (history[which][index[which]++] = newvalue);
		if (index[which] >= MASIZE)
		{
			index[which] -= MASIZE;
			full[which] = 1;
		}

		if (full[which])
			return sum[which] / MASIZE;
		else
			return sum[which] / index[which];
	}
	
	fprintf( stderr, "Error: Moving average call with 'which' >= than %d\n", MACOUNT );
	exit( -1 );
}

/* Determine if A is within P percent of B */
static int numbers_close_signed( int a, int b, float p )
{
	/* Determine if A is within P percent of B */
	
	float x = (float)b - (b*(p/2.0));
	float y = (float)b + (b*(p/2.0));
	
	if( (a>=x) && (a<=y) )
	{
		return 1;
	}

	return 0;
}

/* Determine if A is within P percent of B */
static int numbers_close_double( double a, double b, double p )
{
	/* Determine if A is within P percent of B */
	
	double x = (double)b - (b*(p/2.0));
	double y = (double)b + (b*(p/2.0));
	
	if( (a>=x) && (a<=y) )
	{
		return 1;
	}

	return 0;
}

/* Advance in audio file looking for a low to high or high to low transisition */
static error_code advance_to_next_zero_crossing( cecb_path_id path, int *diff )
{
	int result;
	unsigned char uc;
	
	result = 0;
	
	while( path->wav_current_sample < path->wav_total_samples )
	{
		path->wav_ss1 = path->wav_ss2;
		
		if( path->wav_bits_per_sample == 8 )
		{
			if( fread_le_char( &uc, path->fd ) != 1 )
				return EOS_EOF;
				
			path->wav_ss2 = (short)uc - 127;
		}
		else
		{
			if( fread_le_sshort( &(path->wav_ss2), path->fd ) != 2 )
				return EOS_EOF;
		}

		(path->wav_current_sample)++;
		result++;
		
		if( numbers_close_signed( path->wav_ss2, 0, path->wav_threshold ) == 1 )
			path->wav_ss2 = 0;

		if( (((path->wav_ss1)<=0) && ((path->wav_ss2)>0)) || (((path->wav_ss1)>=0) && ((path->wav_ss2)<0)) )
			break;
	}
	
	*diff = result;
	
	if( path->wav_current_sample >= path->wav_total_samples )
		return EOS_EOF;
		
	return 0;
}

static error_code advance_to_next_lo_to_hi( cecb_path_id path, int *diff )
{
	int result;
	unsigned char uc;
	
	result = 0;
	
	while( path->wav_current_sample < path->wav_total_samples )
	{
		path->wav_ss1 = path->wav_ss2;
		
		if( path->wav_bits_per_sample == 8 )
		{
			if( fread_le_char( &uc, path->fd ) != 1 )
				return EOS_EOF;
				
			path->wav_ss2 = (short)uc - 127;
		}
		else
		{
			if( fread_le_sshort( &(path->wav_ss2), path->fd ) != 2 )
				return EOS_EOF;
		}

		(path->wav_current_sample)++;
		result++;
		
		if( numbers_close_signed( path->wav_ss2, 0, path->wav_threshold ) == 1 )
			path->wav_ss2 = 0;

		if( (((path->wav_ss1)<=0) && ((path->wav_ss2)>0)) )
			break;
	}
	
	*diff = result;
	
	if( path->wav_current_sample >= path->wav_total_samples )
		return EOS_EOF;
		
	return 0;
}

static error_code advance_to_next_hi_to_lo( cecb_path_id path, int *diff )
{
	int result;
	unsigned char uc;
	
	result = 0;
	
	while( path->wav_current_sample < path->wav_total_samples )
	{
		path->wav_ss1 = path->wav_ss2;
		
		if( path->wav_bits_per_sample == 8 )
		{
			if( fread_le_char( &uc, path->fd ) != 1 )
				return EOS_EOF;
				
			path->wav_ss2 = (short)uc - 127;
		}
		else
		{
			if( fread_le_sshort( &(path->wav_ss2), path->fd ) != 2 )
				return EOS_EOF;
		}

		(path->wav_current_sample)++;
		result++;
		
		if( numbers_close_signed( path->wav_ss2, 0, path->wav_threshold ) == 1 )
			path->wav_ss2 = 0;

		if( (((path->wav_ss1)>=0) && ((path->wav_ss2)<0)) )
			break;
	}
	
	*diff = result;
	
	if( path->wav_current_sample >= path->wav_total_samples )
		return EOS_EOF;
		
	return 0;
}

