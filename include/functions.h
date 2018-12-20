typedef struct Fct {
	char *f;
	int (*fct)(void);
} Fct;

extern Fct *functions;
