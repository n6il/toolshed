

#include "mamou.h"

void env_init(assembler *as)
{
    char *include;

    /* 1. Get defs directory environment variable. */
	
    include = getenv("MAMOU_INCLUDE");

    if (include != NULL)
    {
        as->includes[as->include_index++] = include;
    }


    return;
}
