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
