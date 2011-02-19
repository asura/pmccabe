/* Copyright (c) 2002 Hewlett-Packard under GPL version 2 or later */
#include <stdio.h>
#if defined MSC6 || defined WIN32
#define MAXPATHLEN 1024
#else
#include <sys/param.h>	/* for MAXPATHLEN */
#endif
#include <stdlib.h>
#include <assert.h>
#include "pmccabe.h"
#include "dmain.h"

/* $Id: nmain.c,v 1.23 2001/01/26 23:00:37 bame Exp $ */

int Exit = 0;

void
file(char *fname, FILE *f)
{
    stats_t *filestats;
    extern int Unbuf[], *Unptr;
    extern int Pass1;

    Input = f;

    if (Pass1)
    {
	int c;
	extern int Cppflag;

	Cppflag = 1;

	while ((c = Getchar()) != EOF)
	{
	    putc(c, stdout);
	}
	return;
    }

    Unptr = Unbuf;
    Line = 1;
    ncss_Line = 0;

    ZERO(filestats);
    filestats = stats_push(fname, STATS_FILE);
    filestats->firstline = 1;

    while (toplevelstatement(filestats) != EOF)
    {
	filestats->nsemicolons++;
    }

    filestats->lastline = Line - 1;
    /* Not 100% sure why I need to subtract 1 here */
    filestats->nLines += ncss_Line - 1;

    if (Files)
    	printstats(filestats);
    stats_pop(filestats);
}

void
cycoprintstats(stats_t *fs, stats_t *fn)
{
    int basic, cycloswitch, cyclocase;

    basic = fn->nfor + fn->nwhile + fn->nif + fn->nand + fn->nor + fn->nq;

    cycloswitch = 1 + basic + fn->nswitch;
    cyclocase = 1 + basic + fn->ncase;

    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\n",
	cycloswitch,
	cyclocase,
	fn->nstatements,
	fn->firstline,
	fn->defline,
	fn->lastline,
	fn->lastline - fn->firstline + 1,
	fs->name,
	fn->name);
}

void
softbuildprintstats(stats_t *fs, stats_t *fn)
{
    int basic, cycloswitch, cyclocase;

    basic = fn->nfor + fn->nwhile + fn->nif + fn->nand + fn->nor + fn->nq;

    cycloswitch = 1 + basic + fn->nswitch;
    cyclocase = 1 + basic + fn->ncase;

    printf("\"%s\", line %d: %s%%\t%d\t%d\t%d\t%d\t%d\n",
	fs->name, fn->defline,
	fn->name,
	cycloswitch,
	cyclocase,
	fn->nstatements,
	fn->firstline,
	fn->lastline - fn->firstline + 1);
}

static void
printname(const stats_t *sp)
{
    if (sp != NULL)
    {
	printname(sp->prev);
        switch(sp->type)
	{
	case STATS_TOTAL:
	case STATS_FILE:
	    break;

	case STATS_FUNCTION:
	    printf("%s", sp->name);
	    break;

	case STATS_CLASS:
	    if (sp->prev != NULL && sp->prev->type == STATS_FUNCTION)
	        printf("/");
	    printf("%s::", sp->name);
	    break;

	case STATS_NAMESPACE:
	    printf("%s::", sp->name);
	    break;
	}
    }
}

typedef struct {
    int basic;
    int cycloswitch;
    int cyclocase;
    int snlines;
} stats_summary_t;

static void
summarize_stats(stats_t* const sp, stats_summary_t* const summary)
{
    summary->basic = sp->nfor + sp->nwhile + sp->nif + sp->nand + sp->nor + sp->nq;

    sp->nstatements = summary->basic - sp->nand - sp->nor + sp->nsemicolons;

    summary->cycloswitch = sp->nfunctions + summary->basic + sp->nswitch;
    summary->cyclocase = sp->nfunctions + summary->basic + sp->ncase;

    if (Ncssfunction)
    {
	summary->snlines = sp->nLines + 1;
    }
    else
    {
	summary->snlines = sp->lastline - sp->firstline + 1;
    }
}
static const stats_t*
backward_stats(const stats_t* const sp)
{
    const stats_t *fsp;

    for (fsp = sp; fsp != NULL && fsp->type != STATS_FILE; fsp = fsp->prev)
    {
    }
    return fsp;
}
static void
print_total_stats(const stats_summary_t* const summary, const stats_t* const sp)
{
	if (Softbuild)
	{
        printf("\"n/a\", line n/a: %s", sp->name);
        printf("%%\t%d\t%d\t%d\tn/a\t%d\n",
               summary->cycloswitch,
               summary->cyclocase,
               sp->nstatements,
               summary->snlines);
	}
	else if (Cyco)
	{
	    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
		summary->cycloswitch,
		summary->cyclocase,
		sp->nstatements,
		sp->firstline,
		sp->defline,
		sp->lastline,
		summary->snlines,
		"Total");
	}
	else
	{
	    printf("%d\t%d\t%d\tn/a\t%d\t",
		    summary->cycloswitch,
		    summary->cyclocase,
		    sp->nstatements,
		    summary->snlines);

	    printf("Total\n");
	}
}
static void
print_file_stats(const stats_summary_t* const summary, const stats_t* const sp, const stats_t* const fsp)
{
	assert(fsp != NULL);
	if (0 != Softbuild)
	{
	    printf("\"%s\", line 1: n/a", fsp->name);
	    printf("%%\t%d\t%d\t%d\t%d\t%d\n",
		    summary->cycloswitch,
		    summary->cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    summary->snlines);
	}
	else if (0 != Cyco)
	{
	    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\n",
		summary->cycloswitch,
		summary->cyclocase,
		sp->nstatements,
		sp->firstline,
		sp->defline,
		sp->lastline,
		summary->snlines,
		fsp->name);
	}
	else
	{
	    printf("%d\t%d\t%d\t%d\t%d\t",
		    summary->cycloswitch,
		    summary->cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    summary->snlines);

	    printf("%s\n", fsp->name);
	}
}
static void
print_function_stats(const stats_summary_t* const summary, const stats_t* const sp, const stats_t* const fsp)
{
	assert(fsp != NULL);
	if (0 != Softbuild)
	{
	    printf("\"%s\", line %d: ", fsp->name, sp->defline);
	    printname(sp);
	    printf("%%\t%d\t%d\t%d\t%d\t%d\n",
		    summary->cycloswitch,
		    summary->cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    summary->snlines);
	}
	else if (0 != Cyco)
	{
	    printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%s\t",
		summary->cycloswitch,
		summary->cyclocase,
		sp->nstatements,
		sp->firstline,
		sp->defline,
		sp->lastline,
		summary->snlines,
		fsp->name);
	    printname(sp);
	    putchar('\n');
	}
	else
	{
	    printf("%d\t%d\t%d\t%d\t%d\t",
		    summary->cycloswitch,
		    summary->cyclocase,
		    sp->nstatements,
		    sp->firstline,
		    summary->snlines);

	    printf("%s(%d): ", fsp->name, sp->defline);

	    printname(sp);
	    printf("\n");
	}
}
static void
print_class_stats(void)
{
	abort();
}

void
printstats(stats_t *sp)
{
    stats_summary_t summary;
    const stats_t *fsp;

    summarize_stats(sp, &summary);

    fsp = backward_stats(sp);

    switch(sp->type)
    {
    case STATS_TOTAL:
        print_total_stats(&summary, sp);
	break;
    case STATS_FILE:
        print_file_stats(&summary, sp, fsp);
	break;
    case STATS_FUNCTION:
        print_function_stats(&summary, sp, fsp);
	break;
    case STATS_CLASS:
        print_class_stats();
	break;
    }
}
