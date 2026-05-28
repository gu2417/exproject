#ifndef TRANSIT_LOG_H
#define TRANSIT_LOG_H

// 로그 파일을 열고 닫습니다.
int log_init(const char *path);
void log_close(void);
// 일반 로그와 오류 로그를 남깁니다.
void log_info(const char *client_ip, const char *request_type, int result_code, const char *message);
void log_error(const char *client_ip, const char *request_type, int result_code, const char *message);

#endif
