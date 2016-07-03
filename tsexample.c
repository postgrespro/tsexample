/*-------------------------------------------------------------------------
 *
 * tsexample.c
 *     Example of custom full text search parser and dictionaries
 *
 * Copyright (c) 2016, Postgres Professional
 * Author: Alexander Korotkov <a.korotkov@postgrespro.ru>
 *
 * IDENTIFICATION
 *    contrib/tsexample/tsexample.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"
#include "commands/defrem.h"
#include "tsearch/ts_public.h"
#include "tsearch/ts_locale.h"

typedef struct
{
	char   *begin;
	char   *end;
	char   *p;
} SParserStatus;

/* Output token categories */

#define WORD_TOKEN		1
#define NUMBER_TOKEN	2

#define LAST_TOKEN_NUM	2

static const char *const tok_alias[] = {
	"",
	"word",
	"number"
};

static const char *const lex_descr[] = {
	"",
	"Word, all alphanumeric characters",
	"Number, all digits"
};

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(sparser_start);
PG_FUNCTION_INFO_V1(sparser_nexttoken);
PG_FUNCTION_INFO_V1(sparser_end);
PG_FUNCTION_INFO_V1(sparser_lextype);
PG_FUNCTION_INFO_V1(cutdict_init);
PG_FUNCTION_INFO_V1(cutdict_lexize);

Datum
sparser_start(PG_FUNCTION_ARGS)
{
	SParserStatus  *status = (SParserStatus *) palloc0(sizeof(SParserStatus));

	status->begin = (char *) PG_GETARG_POINTER(0);
	status->end = status->begin + PG_GETARG_INT32(1);
	status->p = status->begin;

	PG_RETURN_POINTER(status);
}

Datum
sparser_nexttoken(PG_FUNCTION_ARGS)
{
	SParserStatus  *status = (SParserStatus *) PG_GETARG_POINTER(0);
	char		  **t = (char **) PG_GETARG_POINTER(1);
	int			   *tlen = (int *) PG_GETARG_POINTER(2);
	bool			found = false,
					has_nondigit = false;

	while (status->p < status->end)
	{
		int p_len = pg_mblen(status->p);

		if (t_isalpha(status->p) || t_isdigit(status->p) ||
			(p_len == 1 && *status->p == '_'))
		{
			if (!t_isdigit(status->p))
				has_nondigit = true;
			if (!found)
			{
				*t = status->p;
				found = true;
			}
		}
		else
		{
			if (found)
				break;
		}
		status->p += p_len;
	}

	if (found)
	{
		*tlen = status->p - *t;
		if (has_nondigit)
			PG_RETURN_INT32(WORD_TOKEN);
		else
			PG_RETURN_INT32(NUMBER_TOKEN);
	}
	else
	{
		PG_RETURN_INT32(0);
	}
}

Datum
sparser_end(PG_FUNCTION_ARGS)
{
	SParserStatus  *status = (SParserStatus *) PG_GETARG_POINTER(0);

	pfree(status);
	PG_RETURN_VOID();
}


Datum
sparser_lextype(PG_FUNCTION_ARGS)
{
	LexDescr   *descr = (LexDescr *) palloc(sizeof(LexDescr) * (LAST_TOKEN_NUM + 1));
	int			i;

	for (i = 1; i <= LAST_TOKEN_NUM; i++)
	{
		descr[i - 1].lexid = i;
		descr[i - 1].alias = pstrdup(tok_alias[i]);
		descr[i - 1].descr = pstrdup(lex_descr[i]);
	}

	descr[LAST_TOKEN_NUM].lexid = 0;

	PG_RETURN_POINTER(descr);
}

typedef struct
{
	int		nbegin;
	int		nend;
} CutDict;

Datum
cutdict_init(PG_FUNCTION_ARGS)
{
	List	   *dictoptions = (List *) PG_GETARG_POINTER(0);
	CutDict	   *d = (CutDict *) palloc0(sizeof(CutDict));
	bool		nbegin_loaded = false,
				nend_loaded = false;
	ListCell   *l;

	foreach(l, dictoptions)
	{
		DefElem    *defel = (DefElem *) lfirst(l);

		if (pg_strcasecmp("nbegin", defel->defname) == 0)
		{
			if (nbegin_loaded)
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						 errmsg("multiple nbegin parameters")));
			d->nbegin = atoi(defGetString(defel));
			nbegin_loaded = true;
		}
		else if (pg_strcasecmp("nend", defel->defname) == 0)
		{
			if (nend_loaded)
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						 errmsg("multiple nend parameters")));
			d->nend = atoi(defGetString(defel));
			nend_loaded = true;
		}
		else
		{
			ereport(ERROR,
					(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
					 errmsg("unrecognized cut dictionary parameter: \"%s\"",
						    defel->defname)));
		}
	}

	if (!nbegin_loaded || !nend_loaded)
	{
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("both nbegin and nend parameters of cut dictionary must be specified")));
	}

	PG_RETURN_POINTER(d);
}

Datum
cutdict_lexize(PG_FUNCTION_ARGS)
{
	CutDict	   *d = (CutDict *) PG_GETARG_POINTER(0);
	char	   *in = (char *) PG_GETARG_POINTER(1);
	int32		len = PG_GETARG_INT32(2);
	char	   *txt;
	int			strlen;
	TSLexeme   *res;
	int			residx = 0;
	uint16		nvariant = 1;

	res = palloc0(sizeof(TSLexeme) * 4);
	txt = lowerstr_with_len(in, len);
	strlen = pg_mbstrlen(txt);

	if (strlen <= d->nbegin + d->nend)
	{
		res[residx].nvariant = nvariant;
		res[residx++].lexeme = txt;
		nvariant++;
	}


	if (strlen > d->nbegin)
	{
		int		i;
		char   *p = txt;

		for (i = 0; i < d->nbegin; i++)
			p += pg_mblen(p);
		res[residx].nvariant = nvariant;
		res[residx++].lexeme = pnstrdup(txt, p - txt);
	}

	if (strlen > d->nend)
	{
		int		i;
		char   *p = txt;

		for (i = 0; i < strlen - d->nend; i++)
			p += pg_mblen(p);
		res[residx].nvariant = nvariant;
		res[residx++].lexeme = pstrdup(p);
	}

	PG_RETURN_POINTER(res);
}
