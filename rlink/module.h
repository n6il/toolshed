typedef enum
{
	object_kind_os9,
	object_kind_rsdos

}               object_type;

struct os9
{
	unsigned short  module_size;
	unsigned short  offset_to_module_name;
	int             type_language;
	int             attr_rev;
	int             execuation_offset;
	unsigned short  permanent_storage_size;
	char            module_name[SYMLEN + 1];
	int             edition;
};

struct rsdos
{
	int             exeution_offset;
};

struct object_header
{
	object_type     kind;
	union
	{
		struct os9      os9;
		struct rsdos    rsdos;
	};
};

int             XXX_header(struct object_header * obh, char *filename);
int             XXX_body(struct object_header * obh, char *data, size_t size);
int             XXX_body_byte(struct object_header * obh, int byte);
int             XXX_tail(struct object_header * obh);
