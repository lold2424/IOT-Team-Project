#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define TACT "/dev/tactsw"  // 택트 스위치 장치 파일 경로

typedef struct {
    char storedPassword[5];  // 저장된 비밀번호 (최대 4자리 + null 종단 문자)
    char inputPassword[5];  // 입력된 비밀번호
    int attemptCount;  // 비밀번호 시도 횟수
    int mode;  // 모드 (0: 출근, 1: 퇴근)
    int count;  // 입력 횟수
    bool error;  // 에러 플래그
} SecurityKey;

// 시스템 초기 설정
void setup(SecurityKey *key) {
    // 초기 상태 설정
    key->attemptCount = 0;
    key->mode = 0;  // 기본 모드: 출근 모드
    key->count = 0;
    key->error = false;
    memset(key->inputPassword, 0, sizeof(key->inputPassword));
}

// 비밀번호 확인
void checkPassword(SecurityKey *key) {
    // 입력된 비밀번호가 저장된 비밀번호와 일치하면 메시지 출력
    if (strcmp(key->inputPassword, key->storedPassword) == 0) {
        printf("Password is correct!\n");
    } else {
        // 일치하지 않으면 시도 횟수 증가
        key->attemptCount++;
        printf("Incorrect password. Attempt count: %d\n", key->attemptCount);
    }
    // 입력된 비밀번호 초기화
    memset(key->inputPassword, 0, sizeof(key->inputPassword));
}

// 키패드 입력 받기
char getKeypadInput(SecurityKey *key) {
    unsigned char b;
    int tactswFd = open(TACT, O_RDONLY);
    if (tactswFd < 0) {
        perror("tact device error");
        return '\0';
    }
    read(tactswFd, &b, sizeof(b));
    close(tactswFd);
    // 택트 스위치 입력에 따라 반환 값 결정
    switch (b) {
    case 1: return '1';
    case 2: return '2';
    case 4: return '3';
    case 8: return '4';
    case 16: return '5';
    case 32: return '6';
    case 64: return '7';
    case 128: return '8';
    case 256: return '9';
    case 512: return '0';
    case 1024: return '*';
    case 2048: return '#';
    default: return '\0';
    }
}

// 출근/퇴근 모드 처리 및 비밀번호 확인
void handleKeyPress(SecurityKey *key) {
    char keyInput = getKeypadInput(key);
    if (keyInput) {
        // '*' 키를 누르면 출근 모드로 변경
        if (keyInput == '*') {
            key->mode = 0;  // 출근 모드로 변경
            printf("Mode changed to Work Mode.\n");
        }
        // '#' 키를 누르면 퇴근 모드로 변경
        else if (keyInput == '#') {
            key->mode = 1;  // 퇴근 모드로 변경
            printf("Mode changed to Home Mode.\n");
        }
        // 그 외의 키 입력을 비밀번호로 추가
        else {
            size_t len = strlen(key->inputPassword);
            if (len < sizeof(key->inputPassword) - 1) {
                key->inputPassword[len] = keyInput;
                key->inputPassword[len + 1] = '\0';
                key->count++;
                if (key->count >= 5) {
                    // 입력 횟수 초과 시 에러 설정 및 초기화
                    key->error = true;
                    key->count = 0;
                    printf("Error: Too many inputs. Error flag set.\n");
                }
                // 5번 입력 시 비밀번호 확인
                if (key->count == 5) {
                    checkPassword(key);
                }
            }
        }
    }
}

int main() {
    // SecurityKey 구조체 초기화
    SecurityKey key = { "1234", "", 0, 0, 0, false };
    setup(&key); // 시스템 초기 설정

    while (1) {
        handleKeyPress(&key); // 키 입력 처리
        if (key.error) {
            // 에러가 발생하면 처리
            printf("Error occurred. Resetting...\n");
            key.error = false; // 에러 플래그 초기화
            sleep(1); // 간단한 지연
        }
    }
    return 0;
}
