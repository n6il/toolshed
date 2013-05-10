/***************************************************************************
* ffwd.c: forward reference support
*
* $Id$
*
* The Mamou Assembler - A Hitachi 6309 assembler
*
* (C) 2004 Boisy G. Pitre
***************************************************************************/

#include "mamou.h"

coco_path_id	forward_path;		/* temp file's file descriptor	*/

char	fwd_name[] = { "fwd_refs" } ;

/*!
	@function fwd_init
	@discussion Initializes the forward reference file
	@param as The assembler state structure
 */
void fwd_init(assembler *as)
{
	coco_file_stat fstat;
	
	fstat.perms = FAP_READ | FAP_WRITE;
	if (_coco_create(&forward_path, fwd_name, FAM_READ | FAM_WRITE, &fstat) != 0)
	{
		fatal("Cannot create forward reference file.");
	}

	_coco_close(forward_path); /* close and reopen for reads and writes */

	if (_coco_open(&forward_path, fwd_name, FAM_READ | FAM_WRITE) != 0)
	{
		fatal("Cannot open forward reference file.");
	}

	return;
}


/*!
	@function fwd_deinit
	@discussion Deinitializes the forward reference file
	@param as The assembler state structure
 */
void fwd_deinit(assembler *as)
{
	_coco_close(forward_path);

	_coco_delete(fwd_name);

	return;
}


/*!
	@function fwd_reinit
	@discussion Reinitializes the forward reference file
	@param as The assembler state structure
 */
void fwd_reinit(assembler *as)
{
	u_int			size;
	
	as->F_ref   = 0;
	as->Ffn     = 0;

	_coco_seek(forward_path, 0L, SEEK_SET);   /* rewind forward refs */

	size = sizeof(as->Ffn);
	
	_coco_read(forward_path, (char *)&as->Ffn, &size);

	size = sizeof(as->F_ref);
	
	_coco_read(forward_path, (char *)&as->F_ref, &size); /* read first forward ref into mem */

	if (as->o_debug)
	{
		printf("First fwd ref: %d,%u\n", as->Ffn, (unsigned int)as->F_ref);
	}

	return;
}


/*!
	@function fwd_mark
	@discussion Marks the current file/line as containing a forward reference
	@param as The assembler state structure
 */
void fwd_mark(assembler *as)
{
	u_int		size;
	
	size = sizeof(as->current_filename_index);	
	_coco_write(forward_path, (char *)&as->current_filename_index, &size);

	size = sizeof(as->current_file->current_line);
	_coco_write(forward_path, (char *)&(as->current_file->current_line), &size);

	return;
}


/*!
	@function fwd_next
	@discussion Obtains the next forward reference
	@param as The assembler state structure
 */
void fwd_next(assembler *as)
{
	u_int size;
	
	size = sizeof(as->Ffn);
	_coco_read(forward_path, (char *)&as->Ffn, &size);

	if (as->o_debug)
	{
		printf("Ffn stat=%d ", size);
	}

	size = sizeof(as->F_ref);
	 _coco_read(forward_path, (char *)&as->F_ref, &size);

	if (as->o_debug)
	{
		printf("F_ref stat=%d  ", size);
	}

	if (size < 2)
	{
		as->F_ref = 0;
		as->Ffn = 0;
	}

	if (as->o_debug)
	{
		printf("Next Fwd ref: %d,%u\n", as->Ffn, (unsigned int)as->F_ref);
	}

	return;
}
