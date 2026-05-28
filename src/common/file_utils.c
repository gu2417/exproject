#include "file_utils.h"

#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// 작업에 필요한 폴더의 존재 여부를 확인하고 없을 때 생성합니다.
int ensure_dir(const char *path) {
    struct stat st;

    // 폴더 경로가 비어 있으면 생성 대상이 없으므로 실패를 반환합니다.
    if (!path || !*path) {
        return 0;
    }

    // 이미 존재하면 폴더인지 확인합니다.
    if (stat(path, &st) == 0) {
        return (st.st_mode & S_IFDIR) != 0;
    }

    // 폴더가 존재하지 않을 때는 운영체제별 mkdir 호출로 생성합니다.
    if (MKDIR(path) == 0) {
        return 1;
    }

    // 다른 흐름에서 같은 폴더를 먼저 만든 경우도 정상 상태로 인정합니다.
    return errno == EEXIST;
}
