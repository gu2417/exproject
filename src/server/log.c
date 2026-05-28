#include "log.h"

#include "string_utils.h"

#include <stdio.h>
#include <string.h>

static FILE *g_log_file = NULL;

// 로그에 줄바꿈이 섞이지 않게 정리합니다.
static void sanitize_message(char *dst, size_t dst_size, const char *src) {
    size_t i;
    // 결과를 기록할 버퍼가 유효하지 않으면 정리 작업을 수행하지 않습니다.
    if (!dst || dst_size == 0) {
        return;
    }
    // 로그 원본 메시지가 NULL이면 빈 문자열로 대체합니다.
    if (!src) {
        dst[0] = '\0';
        return;
    }
    // 먼저 안전하게 복사합니다.
    safe_strcpy(dst, dst_size, src);
    // 줄바꿈 문자는 로그 한 줄이 깨지지 않게 빈칸으로 바꿉니다.
    for (i = 0; dst[i]; ++i) {
        if (dst[i] == '\n' || dst[i] == '\r') {
            dst[i] = ' ';
        }
    }
}

// 로그 파일을 엽니다.
int log_init(const char *path) {
    // 로그 파일을 이어쓰기 모드로 엽니다.
    g_log_file = fopen(path, "a");
    return g_log_file != NULL;
}

// 로그 파일을 닫습니다.
void log_close(void) {
    // 열려 있는 로그 파일이 있으면 닫습니다.
    if (g_log_file) {
        fclose(g_log_file);
        g_log_file = NULL;
    }
}

// 로그 한 줄을 콘솔과 파일에 같이 씁니다.
static void log_write(const char *level, const char *client_ip, const char *request_type, int result_code, const char *message) {
    char ts[32];
    char msg[512];
    char req[256];

    // 현재 시간과 출력할 문자열을 준비합니다.
    current_timestamp(ts, sizeof(ts));
    sanitize_message(msg, sizeof(msg), message ? message : "");
    sanitize_message(req, sizeof(req), request_type ? request_type : "-");

    // 콘솔에 로그를 출력합니다.
    printf("%s | %s | %s | %s | %d | %s\n",
           ts,
           level,
           client_ip ? client_ip : "-",
           req,
           result_code,
           msg);

    // 로그 파일이 열려 있으면 파일에도 같은 내용을 씁니다.
    if (g_log_file) {
        fprintf(g_log_file, "%s | %s | %s | %s | %d | %s\n",
                ts,
                level,
                client_ip ? client_ip : "-",
                req,
                result_code,
                msg);
        // 바로 파일에 반영되도록 비웁니다.
        fflush(g_log_file);
    }
}

// 일반 처리 로그를 남깁니다.
void log_info(const char *client_ip, const char *request_type, int result_code, const char *message) {
    log_write("INFO", client_ip, request_type, result_code, message);
}

// 오류 처리 로그를 남깁니다.
void log_error(const char *client_ip, const char *request_type, int result_code, const char *message) {
    log_write("ERROR", client_ip, request_type, result_code, message);
}
