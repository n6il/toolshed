#include <time.h>

#include "mamou.h"


/*
 *      print_line --- pretty print input line
 */
void print_line(assembler *as, int override, char infochar, int counter)
{
	int i = 0;
	char Tmp_buff[512];
	char Line_buff[512];

	Line_buff[0] = EOS;

	if (override == 0 && (as->pass == 1 || as->o_show_listing == BP_FALSE || as->N_page || !as->conditional_stack[as->conditional_stack_index]))
	{
		as->line->has_warning = 0;
		
		return;
	}

	if (as->o_format_only == 0)
	{
		if (as->current_line == 0)
		{
			/* 1. We're at top of page, print header. */
			
			print_header(as);
		}


		/* 1. Print line number. */
		
		sprintf(Tmp_buff, "%05d ", (int)as->current_file->current_line);
		
		strcat(Line_buff, Tmp_buff);
	
		/* TODO! warnings, errors will go here later */
		if ((infochar == ' ') && as->line->has_warning) infochar = 'W';
		if (as->line->has_warning) as->num_warnings++;
		
		sprintf(Tmp_buff, " %c ", infochar);

		strcat(Line_buff, Tmp_buff);

		if (as->P_total || as->P_force)
		{
			sprintf(Tmp_buff, "%04X ", counter);
			strcat(Line_buff, Tmp_buff);
		}
		else
		{
			strcat(Line_buff, "     ");
		}
	
		if (as->Cflag)
		{
			if (as->cumulative_cycles)
			{
				sprintf(Tmp_buff, "[%2u] ", (unsigned int)as->cumulative_cycles);
				strcat(Line_buff, Tmp_buff);
			}
			else
			{
				strcat(Line_buff, "     ");
			}
		}

		for (i = 0; i < as->P_total && i < 4; i++)
		{
			sprintf(Tmp_buff, "%02X", lobyte(as->P_bytes[i]));
			strcat(Line_buff, Tmp_buff);
		}

		for (; i < 4; i++)
		{
			strcat(Line_buff, "  ");
		}
	
		strcat(Line_buff, "   ");

	}

	as->current_line++;

//	mamou_parse_line(as);
	
	if (*as->line->label == EOS && *as->line->Op == EOS && *as->line->operand == EOS)
	{
		/* possibly a comment? */
		if (*as->line->comment != EOS)
		{
			sprintf(Tmp_buff, "%s", as->line->comment);
			strcat(Line_buff, Tmp_buff);
		}
	}
	else
	{
		if (*as->line->comment == EOS)
		{
			if (as->tabbed)
			{
				sprintf(Tmp_buff, "%s\t%s\t%s", as->line->label, as->line->Op, as->line->operand);
			}
			else
			{
				sprintf(Tmp_buff, "%-8s %-4s  %-10s", as->line->label, as->line->Op, as->line->operand);
			}
			strcat(Line_buff, Tmp_buff);
		}
		else
		{
			if (as->tabbed)
			{
				sprintf(Tmp_buff, "%s\t%s\t%s\t%s", as->line->label, as->line->Op, as->line->operand, as->line->comment);
			}
			else
			{
				sprintf(Tmp_buff, "%-8s %-4s  %-10s %s", as->line->label, as->line->Op, as->line->operand, as->line->comment);
			}
			strcat(Line_buff, Tmp_buff);
		}
	}

	if (as->Opt_G == BP_TRUE)
	{
		int Temp_pc = as->old_program_counter;

		for (; i < as->P_total; i++)
		{
			if (i % 4 == 0)
			{
				as->current_file->current_line++;
				Temp_pc += 4;
				sprintf(Tmp_buff, "\n%05d   %04X ", (int)as->current_file->current_line, Temp_pc);
				strcat(Line_buff, Tmp_buff);
			}
			sprintf(Tmp_buff, "%02x", lobyte(as->P_bytes[i]));
			strcat(Line_buff, Tmp_buff);
		}
	}

	/* print out the built up line */
	strncpy(Tmp_buff, Line_buff, as->o_pagewidth);
	Tmp_buff[as->o_pagewidth] = EOS;
	printf("%s\n", Tmp_buff);

	/* check if we are at last line before footer should be printed */
	if (as->o_format_only == 0)
	{
		if (as->current_line == as->o_page_depth - as->footer_depth)
		{
			print_footer(as);
			as->current_line = 0;
			as->current_page++;
		}
	}

	as->line->has_warning = 0;
	return;
}


void report_summary(assembler *as)
{
	printf("\n");
	printf("Assembler Summary:\n");
	printf(" - %u errors, %u warnings\n", (unsigned int)as->num_errors, (unsigned int)as->num_warnings);
	printf(" - %u lines (%u code, %u blank, %u comment)\n",
		(unsigned int)as->cumulative_total_lines,
		(unsigned int)(as->cumulative_total_lines - (as->cumulative_blank_lines + as->cumulative_comment_lines)),
		(unsigned int)as->cumulative_blank_lines,
		(unsigned int)as->cumulative_comment_lines
	);

	if (as->o_decb == BP_TRUE)
	{
		printf(" - $%04X (%u) bytes generated\n",
			   (unsigned int)as->code_bytes,
			   (unsigned int)as->code_bytes
			);
	}
	else
	{
		printf(" - $%04X (%u) program bytes, $%04X (%u) data bytes\n",
			   (unsigned int)as->code_bytes,
			   (unsigned int)as->code_bytes,
			   (unsigned int)as->data_counter,
			   (unsigned int)as->data_counter
			   );
	}
	

	if (as->object_name[0] == '\0')
	{
		printf(" - No output file\n");
	}
	else
	{
		printf(" - Output file: \"%s\"\n", as->object_name);
	}
	
	
	return;
}


void print_header(assembler *as)
{
	time_t now;
	struct tm *tm;

	now = time(NULL);
	tm = localtime(&now);

	printf("The Mamou Assembler Version 01.00.00    %02d/%02d/%02d %02d:%02d:%02d      Page %03u\n",
	       tm->tm_mon + 1, tm->tm_mday, tm->tm_year + 1900,
	       tm->tm_hour, tm->tm_min, tm->tm_sec,
	       (unsigned int)as->current_page);
	printf("%s - %s\n", as->Nam, as->Ttl);
	printf("\n");
	as->current_line += as->header_depth;

	return;
}


void print_footer(assembler *as)
{
	printf("\n");
	printf("\n");
	printf("\n");
	as->current_line += as->footer_depth;

	return;
}
