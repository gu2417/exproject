#ifndef TRANSIT_STRING_UTILS_H
#define TRANSIT_STRING_UTILS_H

#include <stddef.h>

// 문자열 앞뒤 빈칸과 줄바꿈을 정리합니다.
void str_trim(char *s);
// 크기를 넘지 않게 문자열을 복사합니다.
void safe_strcpy(char *dst, size_t dst_size, const char *src);
// 문자열 검사와 숫자 변환에 쓰는 함수들입니다.
int str_is_blank(const char *s);
int str_contains_protocol_delim(const char *s);
int str_to_int(const char *s, int *out);
// 구분자로 나뉜 세 값을 꺼냅니다.
int split_three_fields(const char *input,
                       char sep,
                       char *f1,
                       size_t f1_size,
                       char *f2,
                       size_t f2_size,
                       char *f3,
                       size_t f3_size);
// 문자열 뒤에 내용을 붙입니다.
int str_append(char *dst, size_t dst_size, const char *src);
int str_appendf(char *dst, size_t dst_size, const char *fmt, ...);
// 시간 문자열과 검색어 비교에 쓰는 함수들입니다.
void current_timestamp(char *dst, size_t dst_size);
int keyword_match(const char *text, const char *keyword);
void strip_newline(char *s);

#endif
