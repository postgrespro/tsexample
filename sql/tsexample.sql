CREATE EXTENSION tsexample;

SELECT * FROM ts_parse('sample_parser', 'abc def 123 1xx yy3');
SELECT * FROM ts_parse('sample_parser', 'x-z p;<qwe> pg_config');
SELECT to_tsvector('sample', 'abcdef 12345678 xyz');
SELECT plainto_tsquery('sample', 'abcdef 12345678 xyz');
SELECT * FROM ts_debug('sample', 'abcdef 12345678 xyz');
