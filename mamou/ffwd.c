
#include "mamou.h"


coco_path_id	forward_path;		/* temp file's file descriptor	*/


char	fwd_name[] = { "fwd_refs" } ;

/*
 *      fwd_init --- initialize forward ref file
 */
 
void fwd_init(assembler *as)
{
	if (_coco_create(&forward_path, fwd_name, FAM_READ | FAM_WRITE, FAP_READ | FAP_WRITE) != 0)
	{
		fatal("Can't create temp file");
	}

	_coco_close(forward_path); /* close and reopen for reads and writes */

	if (_coco_open(&forward_path, fwd_name, FAM_READ | FAM_WRITE) != 0)
	{
		fatal("Forward ref file has gone.");
	}


	return;
}



void fwd_deinit(assembler *as)
{
	_coco_close(forward_path);

	_coco_delete(fwd_name);


	return;
}


/*
 * fwd_reinit --- reinitialize forward ref file
 */
void fwd_reinit(assembler *as)
{
	int			size;
	
	
	as->F_ref   = 0;
	as->Ffn     = 0;

	_coco_seek(forward_path, 0L, SEEK_SET);   /* rewind forward refs */

	size = sizeof(as->Ffn);
	
	_coco_read(forward_path, (char *)&as->Ffn, &size);

	size = sizeof(as->F_ref);
	
	_coco_read(forward_path, (char *)&as->F_ref, &size); /* read first forward ref into mem */

	if (as->o_debug)
	{
		printf("First fwd ref: %d,%d\n", as->Ffn, as->F_ref);
	}


	return;
}



/*
 * fwd_mark --- mark current file/line as containing a forward ref
 */
void fwd_mark(assembler *as)
{
	int		size;
	
	
	size = sizeof(as->current_filename_index);	
	_coco_write(forward_path, (char *)&as->current_filename_index, &size);

	size = sizeof(as->current_file->current_line);
	_coco_write(forward_path, (char *)&(as->current_file->current_line), &size);


	return;
}



/*
 * fwd_next --- get next forward ref
 */
 
void fwd_next(assembler *as)
{
	int size;
	

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
		printf("Next Fwd ref: %d,%d\n", as->Ffn, as->F_ref);
	}


	return;
}
