#ifndef TRANSIT_RESULT_VIEWER_H
#define TRANSIT_RESULT_VIEWER_H

// 서버 응답을 화면에 출력하고 응답 코드를 확인합니다.
void result_viewer_show(const char *response);
int response_code(const char *response);

#endif
