#ifndef TRANSIT_DATA_LOADER_H
#define TRANSIT_DATA_LOADER_H

#include "types.h"

// CSV 원본을 적재하고 환승 그래프까지 구성하는 로딩 함수를 제공합니다.
int data_loader_load(AppData *data);
int data_loader_build_graph(AppData *data);

#endif
