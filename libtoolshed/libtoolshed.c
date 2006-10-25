/********************************************************************
 * $Id$
 ********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <toolshed.h>


void TSReportError(error_code te, char *errorstr)
{
	switch (te)
	{
		case 0:
			strcpy(errorstr, "SUCCESS!");
			break;

		case EOS_FNA:
			strcpy(errorstr, "the file's permissions make it inaccessible to you");
			break;

		case EOS_EOF:
			strcpy(errorstr, "input past end-of-file");
			break;

		case EOS_FAE:
			strcpy(errorstr, "file already exists");
			break;

		case EOS_BPNAM:
			strcpy(errorstr, "badly formed pathname");
			break;

		case EOS_PNNF:
			strcpy(errorstr, "pathname not found");
			break;

		case EOS_WRITE:
			strcpy(errorstr, "error writing to file");
			break;

		case EOS_DF:
			strcpy(errorstr, "disk is filled to capacity");
			break;

		case EOS_PADROM:
			strcpy(errorstr, "file size insufficient for pad size");
			break;

		default:
			strcpy(errorstr, "unknown error");
			break;
	}
}


error_code TSRBFRename(char *pathlist, char *new_name)
{
    error_code	ec = 0;

    ec = _coco_rename(pathlist, new_name);
	
    return ec;
}



error_code TSPadROM(char *pathlist, int padSize, char padChar)
{
    error_code	ec = 0;
    coco_path_id path;
    int j;
    u_int fileSize;


    ec = _coco_open(&path, pathlist, FAM_WRITE);

    if (ec != 0)
    {
        return ec;
    }


    ec = _coco_gs_size(path, &fileSize);

    if (ec != 0)
    {
        _coco_close(path);

        return ec;
    }

    if (padSize <= fileSize)
    {
        _coco_close(path);

        return EOS_PADROM;
    }

    _coco_seek(path, fileSize, SEEK_SET);

    for (j = 0; j < padSize - fileSize; j++)
    {
        u_int size = 1;

        _coco_write(path, &padChar, &size);
    }

    _coco_close(path);


    return 0;
}



error_code TSRBFAttrGet(char *p, char attr, char *strattr)
{
    error_code	ec = 0;
    os9_path_id path;
	
    /* open a path to the device */
    ec = _os9_open(&path, p, FAM_READ);

    if (ec != 0)
    {
        return(ec);
    }
	
    {
        fd_stats fdbuf;
        int size = sizeof(fdbuf);

        _os9_gs_fd(path, size, &fdbuf);

		attr = fdbuf.fd_att;
		
		if (strattr != NULL)
		{
			OS9AttrToString(attr, strattr);
		}
	}

    _os9_close(path);

    return(0);
}



error_code TSRBFAttrSet(char *file, int attrSetMask, int attrResetMask, char attr, char *strattr)
{
    error_code	ec = 0;
    os9_path_id path;
	
    /* open a path to the device */
    ec = _os9_open(&path, file, FAM_WRITE);

    if (ec != 0)
    {
        return(ec);
    }
	
    {
        fd_stats fdbuf;
        int size = sizeof(fdbuf);

        ec = _os9_gs_fd(path, size, &fdbuf);
        
        if (attrSetMask != 0)
        {
            fdbuf.fd_att |= attrSetMask;
        }

        if (attrResetMask != 0)
        {
            fdbuf.fd_att &= ~attrResetMask;
        }
        
        ec = _os9_ss_fd(path, size, &fdbuf);

		attr = fdbuf.fd_att;
		
		if (strattr != NULL)
		{
			OS9AttrToString(attr, strattr);
		}
	}

    _os9_close(path);

    return(0);
}

