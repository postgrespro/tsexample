CREATE EXTENSION tsexample;

SELECT * FROM ts_parse('sample_parser', 'abc def 123 1xx yy3');
SELECT * FROM ts_parse('sample_parser', 'x-z p;<qwe> pg_config');
