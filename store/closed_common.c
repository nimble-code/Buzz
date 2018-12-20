#ifndef R
	#define R	(5)
#endif

void
store_test(void)
{	ulong tbin[(1<<R)+1];
	ulong reso = (1UL << (B_width-R));	// store_test
	ulong i;

	if (!B_test)
	{	return;
	}

	memset((void *) tbin, 0, sizeof(tbin));
	for (i = 0; i < (1UL<<B_width); i++)
	{	if (htab[i].len)
		{	tbin[i/reso]++;
	}	}
	fprintf(stderr, "\nstore test:\n\t");
	for (i = 0; i < (1<<R); i++)
	{	fprintf(stderr, "%6.2f ",
			(100.0*(float)tbin[i])/((float)B_nstates));
		if (i>0 && (i+1)%R==0)
		{	fprintf(stderr, "\n\t");
	}	}
	fprintf(stderr, "\n");
}

void
store_stats(void)
{
	if (hash_badness)
	{	fprintf(stderr, "\thash badness  %lu (%luM)",
			hash_badness,  hash_badness/(1000*1000));
	}
	if (hash_mismatch)
	{	fprintf(stderr, "%shash mismatch %lu (%luM)",
			hash_badness?", ":"\t",
			hash_mismatch, hash_mismatch/(1000*1000));
	}
	fprintf(stderr, "\n");
}

void
store_full(void)
{
	fprintf(stderr, "hash table is full (-w%d), losing states\n", B_width);
}

ulong
store_last_slot(void)
{
	return last_stored;
}

void
set_last_slot(ulong n)
{
	last_stored = n;
}
