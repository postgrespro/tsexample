/* contrib/tsexample/tsexample--1.0.sql */

-- complain if script is sourced in psql, rather than via CREATE EXTENSION
\echo Use "CREATE EXTENSION tsexample;" to load this file. \quit

CREATE OR REPLACE FUNCTION sparser_start(internal, integer)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION sparser_nexttoken(internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION sparser_end(internal)
	RETURNS void
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION sparser_lextype(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE TEXT SEARCH PARSER sample_parser (
	START = sparser_start,
	GETTOKEN = sparser_nexttoken,
	END = sparser_end,
	LEXTYPES = sparser_lextype
);
COMMENT ON TEXT SEARCH PARSER sample_parser IS 'sample word parser';

CREATE OR REPLACE FUNCTION cutdict_init(internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE OR REPLACE FUNCTION cutdict_lexize(internal, internal, internal, internal)
	RETURNS internal
	AS 'MODULE_PATHNAME'
	LANGUAGE C STRICT IMMUTABLE;

CREATE TEXT SEARCH TEMPLATE cutdict (
	INIT = cutdict_init,
	LEXIZE = cutdict_lexize
);
COMMENT ON TEXT SEARCH TEMPLATE cutdict IS 'cut dictionary: lowercase and keep only beginning and ending of long words';

CREATE TEXT SEARCH DICTIONARY cut3 (
	TEMPLATE = cutdict,
	nbegin = 3,
	nend = 3
);
COMMENT ON TEXT SEARCH DICTIONARY cut3 IS 'cut dictionary with nbegin = nend = 3';

CREATE TEXT SEARCH CONFIGURATION sample (
	PARSER = "sample_parser"
);
ALTER TEXT SEARCH CONFIGURATION sample ADD MAPPING FOR word WITH cut3;
ALTER TEXT SEARCH CONFIGURATION sample ADD MAPPING FOR number WITH simple;
COMMENT ON TEXT SEARCH CONFIGURATION sample IS 'sample configuration';
