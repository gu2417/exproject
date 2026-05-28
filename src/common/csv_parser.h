#ifndef TRANSIT_CSV_PARSER_H
#define TRANSIT_CSV_PARSER_H

#include <stddef.h>

#define CSV_MAX_FIELDS 32
#define CSV_MAX_FIELD_LEN 256

// CSV 한 줄의 필드들을 담는 구조체입니다.
typedef struct CsvRow {
    char fields[CSV_MAX_FIELDS][CSV_MAX_FIELD_LEN]; // 필드 값
    int count;                                      // 필드 개수
} CsvRow;

// CSV 한 줄을 파싱하고 헤더 위치와 값을 찾습니다.
int csv_parse_line(const char *line, CsvRow *row);
int csv_find_header(const CsvRow *header, const char *name);
const char *csv_get(const CsvRow *row, int index);

#endif
