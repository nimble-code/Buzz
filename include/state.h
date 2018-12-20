// included in buzz.h

#ifndef SV
	#define SV	    32
#endif

typedef struct State {
	uchar	sv[SV];		// state vector
	AST	*s[MaxProc];	// process states
} State;
