typedef enum
{
	object_kind_os9,
	object_kind_decb

}               object_type;

#if 0
struct os9
{
	unsigned short  module_size;
	unsigned short  offset_to_module_name;
	int             type_language;
	int             attr_rev;
	int             execution_offset;
	unsigned short  permanent_storage_size;
	char            module_name[29 + 1];
	int             edition;
};

struct decb
{
	uint16_t        segment_size;
	uint16_t        org_offset;
};
#endif

struct object_header
{
	unsigned short  module_size;
	unsigned short  offset_to_module_name;
	int             type_language;
	int             attr_rev;
	int             execution_offset;
	unsigned short  permanent_storage_size;
	char            module_name[29 + 1];
	int             edition;

#if 0
object_type     kind;
	union
	{
		struct os9      os9;
		struct decb     decb;
	};
#endif
};

int     os9_header();
int     os9_body_byte();
int     os9_body();
int     os9_tail();

int     decb_header();
int     decb_body_byte();
int     decb_body();
int     decb_tail();
