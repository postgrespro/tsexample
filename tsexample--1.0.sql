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
