#include "string_utils.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// 문자열 앞뒤의 빈칸과 개행을 지웁니다.
void str_trim(char *s) {
    char *start;
    char *end;

    // 문자열 포인터가 비어 있으면 정리할 대상이 없으므로 반환합니다.
    if (!s) {
        return;
    }

    // 앞쪽 빈칸이 끝나는 지점을 찾습니다.
    start = s;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    // 앞에 빈칸이 있었으면 문자열을 앞으로 당깁니다.
    if (start != s) {
        memmove(s, start, strlen(start) + 1);
    }

    // UTF-8 파일 앞에 붙는 BOM이 있으면 제거합니다.
    if ((unsigned char)s[0] == 0xEF &&
        (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF) {
        memmove(s, s + 3, strlen(s + 3) + 1);
    }

    // 문자열이 비었으면 뒤쪽 확인은 하지 않습니다.
    if (*s == '\0') {
        return;
    }

    // 뒤쪽 빈칸을 끝에서부터 지웁니다.
    end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
}

// 크기를 넘지 않게 문자열을 복사합니다.
void safe_strcpy(char *dst, size_t dst_size, const char *src) {
    // 대상 버퍼가 준비되지 않은 경우 문자열 복사를 수행하지 않습니다.
    if (!dst || dst_size == 0) {
        return;
    }
    // 원본 문자열이 없을 때는 결과를 빈 문자열로 초기화합니다.
    if (!src) {
        dst[0] = '\0';
        return;
    }
    snprintf(dst, dst_size, "%s", src);
}

// 문자열이 비었거나 빈칸만 있는지 확인합니다.
int str_is_blank(const char *s) {
    // NULL도 빈 값으로 봅니다.
    if (!s) {
        return 1;
    }
    // 빈칸이 아닌 글자를 하나라도 찾으면 빈 값이 아닙니다.
    while (*s) {
        if (!isspace((unsigned char)*s)) {
            return 0;
        }
        s++;
    }
    return 1;
}

// 통신 구분자로 쓰는 글자가 들어 있는지 확인합니다.
int str_contains_protocol_delim(const char *s) {
    // 검사 대상이 NULL인 경우 프로토콜을 깨뜨릴 입력이 없는 것으로 간주합니다.
    if (!s) {
        return 0;
    }
    // 요청 형식을 깨뜨릴 수 있는 글자를 찾습니다.
    while (*s) {
        unsigned char c = (unsigned char)*s;
        if (c == '|' || c == ';' || c == '\n' || c == '\r' || c == '\0') {
            return 1;
        }
        s++;
    }
    return 0;
}

// 문자열을 정수로 바꿉니다.
int str_to_int(const char *s, int *out) {
    char *end = NULL;
    long value;

    // 입력값이 비어 있으면 정수 변환을 시도하지 않습니다.
    if (!s || str_is_blank(s)) {
        return 0;
    }

    // 입력 문자열을 10진수 정수 기준으로 해석합니다.
    value = strtol(s, &end, 10);
    // 숫자 뒤에 불필요한 문자가 남으면 올바른 정수 입력으로 보지 않습니다.
    if (!end || *end != '\0') {
        return 0;
    }
    // 결과를 받을 곳이 있으면 저장합니다.
    if (out) {
        *out = (int)value;
    }
    return 1;
}

// 구분자 2개를 기준으로 문자열을 세 칸으로 나눕니다.
int split_three_fields(const char *input,
                       char sep,
                       char *f1,
                       size_t f1_size,
                       char *f2,
                       size_t f2_size,
                       char *f3,
                       size_t f3_size) {
    char buf[2048];
    char *p1;
    char *p2;

    // 입력 문자열과 결과 버퍼가 모두 준비된 경우에만 필드 분리를 진행합니다.
    if (!input || !f1 || !f2 || !f3) {
        return 0;
    }

    // 원본을 바꾸지 않기 위해 임시 버퍼에 복사합니다.
    safe_strcpy(buf, sizeof(buf), input);
    // 첫 번째 구분자를 찾습니다.
    p1 = strchr(buf, sep);
    if (!p1) {
        return 0;
    }
    *p1++ = '\0';
    // 두 번째 구분자를 찾습니다.
    p2 = strchr(p1, sep);
    if (!p2) {
        return 0;
    }
    *p2++ = '\0';

    // 각 칸의 앞뒤 빈칸을 정리합니다.
    str_trim(buf);
    str_trim(p1);
    str_trim(p2);
    // 정리한 값을 결과 변수에 복사합니다.
    safe_strcpy(f1, f1_size, buf);
    safe_strcpy(f2, f2_size, p1);
    safe_strcpy(f3, f3_size, p2);
    return 1;
}

// 문자열 뒤에 다른 문자열을 붙입니다.
int str_append(char *dst, size_t dst_size, const char *src) {
    size_t used;
    size_t need;

    // 대상 문자열, 추가 문자열, 버퍼 크기가 유효할 때만 이어 붙입니다.
    if (!dst || !src || dst_size == 0) {
        return 0;
    }

    // 현재 길이와 붙일 길이를 계산합니다.
    used = strlen(dst);
    need = strlen(src);
    // 전체 크기를 넘으면 붙이지 않습니다.
    if (used + need + 1 > dst_size) {
        return 0;
    }
    // 널 문자를 포함해서 뒤에 복사합니다.
    memcpy(dst + used, src, need + 1);
    return 1;
}

// 형식을 맞춘 문자열을 뒤에 붙입니다.
int str_appendf(char *dst, size_t dst_size, const char *fmt, ...) {
    va_list args;
    size_t used;
    int written;

    // 결과 버퍼와 형식 문자열이 준비된 경우에만 형식 출력을 수행합니다.
    if (!dst || !fmt || dst_size == 0) {
        return 0;
    }

    // 현재 문자열 끝 위치를 찾습니다.
    used = strlen(dst);
    if (used >= dst_size) {
        return 0;
    }

    // 남은 공간에 맞춰 형식 문자열을 출력합니다.
    va_start(args, fmt);
    written = vsnprintf(dst + used, dst_size - used, fmt, args);
    va_end(args);

    return written >= 0 && (size_t)written < dst_size - used;
}

// 현재 시각을 로그와 기록 파일에서 쓰는 문자열 형식으로 변환합니다.
void current_timestamp(char *dst, size_t dst_size) {
    time_t now;
    struct tm *tm_info;

    // 시간 값을 저장할 버퍼가 유효할 때만 현재 시각을 기록합니다.
    if (!dst || dst_size == 0) {
        return;
    }

    // 현재 시간을 가져와 지역 시간으로 바꿉니다.
    now = time(NULL);
    tm_info = localtime(&now);
    // 지역 시간 변환에 실패하면 고정된 기본 시간 문자열을 사용합니다.
    if (!tm_info) {
        safe_strcpy(dst, dst_size, "0000-00-00 00:00:00");
        return;
    }
    // 화면에 쓰기 좋은 모양으로 저장합니다.
    strftime(dst, dst_size, "%Y-%m-%d %H:%M:%S", tm_info);
}

// 문자열 안에 검색어가 들어 있는지 확인합니다.
int keyword_match(const char *text, const char *keyword) {
    // 비교 대상 또는 검색어가 유효하지 않은 경우 매칭 실패를 반환합니다.
    if (!text || !keyword || str_is_blank(keyword)) {
        return 0;
    }
    return strstr(text, keyword) != NULL;
}

// 문자열 끝의 줄바꿈 문자를 지웁니다.
void strip_newline(char *s) {
    size_t len;
    // 대상 문자열이 NULL이면 개행 제거 과정을 생략합니다.
    if (!s) {
        return;
    }
    // 끝에서부터 줄바꿈 문자를 지웁니다.
    len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) {
        s[len - 1] = '\0';
        len--;
    }
}
