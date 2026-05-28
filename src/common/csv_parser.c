#include "csv_parser.h"

#include "string_utils.h"

#include <stdio.h>
#include <string.h>

// CSV 한 줄을 쉼표 기준으로 나눕니다.
int csv_parse_line(const char *line, CsvRow *row) {
    int field = 0;
    int pos = 0;
    int in_quotes = 0;
    const char *p;

    // 입력 행과 결과 구조체가 모두 유효할 때만 파싱을 진행합니다.
    if (!line || !row) {
        return 0;
    }

    // 이전 값이 남지 않도록 행 전체를 비웁니다.
    memset(row, 0, sizeof(*row));

    // 줄 끝이 나올 때까지 글자를 하나씩 확인합니다.
    for (p = line; *p && *p != '\n' && *p != '\r'; ++p) {
        char c = *p;
        // 따옴표 안에서는 쉼표도 글자로 취급합니다.
        if (c == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        // 따옴표 밖의 쉼표를 만나면 한 칸을 끝냅니다.
        if (c == ',' && !in_quotes) {
            row->fields[field][pos] = '\0';
            str_trim(row->fields[field]);
            field++;
            pos = 0;
            // 최대 칸 수를 넘으면 여기서 마무리합니다.
            if (field >= CSV_MAX_FIELDS) {
                row->count = CSV_MAX_FIELDS;
                return 1;
            }
            continue;
        }
        // 한 칸의 길이를 넘지 않는 범위에서 글자를 저장합니다.
        if (pos + 1 < CSV_MAX_FIELD_LEN) {
            row->fields[field][pos++] = c;
        }
    }

    // 마지막 칸을 닫고 전체 칸 개수를 저장합니다.
    row->fields[field][pos] = '\0';
    str_trim(row->fields[field]);
    row->count = field + 1;
    return row->count > 0;
}

// 헤더 행에서 원하는 열 이름의 위치를 찾습니다.
int csv_find_header(const CsvRow *header, const char *name) {
    int i;
    // 헤더나 열 이름이 전달되지 않으면 검색 실패 값(-1)을 반환합니다.
    if (!header || !name) {
        return -1;
    }
    // 앞에서부터 열 이름을 비교합니다.
    for (i = 0; i < header->count; ++i) {
        if (strcmp(header->fields[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

// 행에서 해당 위치의 값을 가져옵니다.
const char *csv_get(const CsvRow *row, int index) {
    // 요청한 인덱스가 행 범위를 벗어나면 빈 문자열을 반환합니다.
    if (!row || index < 0 || index >= row->count) {
        return "";
    }
    return row->fields[index];
}
