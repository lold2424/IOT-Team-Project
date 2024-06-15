#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>

#define TACT "/dev/tactsw"

int getTACT() {
    unsigned char b;
    int tactswFd = open(TACT, O_RDONLY);
    if (tactswFd < 0) {
        std::perror("tact device error");
        return -1;
    }

    if (read(tactswFd, &b, sizeof(b)) < 0) {
        std::perror("read error");
        close(tactswFd);
        return -1;
    }

    close(tactswFd);

    switch (b) {
    case 1: return '1'; // 1번 버튼
    case 2: return '2'; // 2번 버튼
    case 3: return '3'; // 3번 버튼
    case 4: return '4'; // 4번 버튼
    case 5: return '5'; // 5번 버튼
    case 6: return '6'; // 6번 버튼
    case 7: return '7'; // 7번 버튼
    case 8: return '8'; // 8번 버튼
    case 9: return '9'; // 9번 버튼
    case 10: return '0'; // 0번 버튼
    case 11: return '*'; // * 버튼
    case 12: return '#'; // # 버튼
    default: return -1;
    }
}

int main() {
    while (true) {
        int tact_input = getTACT();
        if (tact_input != -1) {
            switch (tact_input) {
            case '1': std::printf("Button 1 pressed\n"); break;
            case '2': std::printf("Button 2 pressed\n"); break;
            case '3': std::printf("Button 3 pressed\n"); break;
            case '4': std::printf("Button 4 pressed\n"); break;
            case '5': std::printf("Button 5 pressed\n"); break;
            case '6': std::printf("Button 6 pressed\n"); break;
            case '7': std::printf("Button 7 pressed\n"); break;
            case '8': std::printf("Button 8 pressed\n"); break;
            case '9': std::printf("Button 9 pressed\n"); break;
            case '0': std::printf("Button 0 pressed\n"); break;
            case '*': std::printf("Button * pressed\n"); break;
            case '#': std::printf("Button # pressed\n"); break;
            default: std::printf("Unknown button pressed: %d\n", tact_input); break;
            }
        }
        usleep(100000);  // 0.1초 대기
    }

    return 0;
}
