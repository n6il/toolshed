extern unsigned Dasm6309 (char *buffer, int pc, unsigned char *memory, size_t memory_size);

enum {
	HD6309_PC=1, HD6309_S, HD6309_CC ,HD6309_A, HD6309_B, HD6309_U, HD6309_X, HD6309_Y, HD6309_DP, 
	HD6309_E, HD6309_F, HD6309_V, HD6309_MD };

/* 6309 ADDRESSING MODES */
enum HD6309_ADDRESSING_MODES {
	INH,
	DIR,
	IND,
	REL,
	EXT,
	IMM,
	IMM16,
	LREL,
	PG2,						/* PAGE SWITCHES -	Page 2 */
	PG3, 						/*					Page 3 */
	R2R,
};

