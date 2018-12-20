
// test the quality of the hash
// by plotting an x,y chart
//	x: chain lengths 
//	y: nr of items in all chains of that length

void
store_test(void)
{	ulong i, n;
	ulong cnt[16];
	ulong over = 0;
	volatile Htable *s;

	if (!B_test)
	{	return;
	}

	memset(cnt, 0, sizeof(cnt));

	for (i = 0; i < (1UL<<B_width); i++)
	{	n = 0;
		for (s = htab[i]; s; s = s->nxt)
		{	n++;
		}
		if (n < 16)
		{	cnt[n]++;
		} else
		{	over++;
	}	}

	fprintf(stderr, "\nstore test (length of hash chains):\n");
	for (i = 1; i < 16; i++)
	{	if (cnt[i])
		{	fprintf(stderr, "\t%3lu\t%6lu\n", i, cnt[i]);
	}	}
	if (over)
	{	fprintf(stderr, "\t---\t%6lu\n", over);
	}
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
