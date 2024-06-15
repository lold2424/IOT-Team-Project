#include <iostream>
#include <string>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstdio>
#include <vector>
#include <algorithm>

#define ALARM_SOUND_FILE "/alarm_sound.wav" // 실패 시 재생할 사운드 파일 경로
#define OPEN_SOUND_FILE "/Open_sound.wav" // 성공 시 재생할 사운드 파일 경로
#define TACT "/dev/tactsw" // 텍트 스위치 장치 파일 경로
#define CLCD "/dev/clcd" // CLCD 장치 파일 경로
#define TIME_QUANTUM 1667
#define WAITING_SCREEN_INTERVAL 5 // 대기 화면 표시 간격 (초)
#define DISPLAY_DURATION 5 // 메시지 표시 시간 (초)

using namespace std;

// 사운드 재생
void playSound(const char* soundFile) {
    std::string command = "aplay ";
    command += soundFile;
    command += " &"; // 백그라운드에서 실행
    system(command.c_str());
}

void writeCLCD(const string& message) {
    int clcd_fd = open(CLCD, O_WRONLY);
    if (clcd_fd < 0) {
        perror("clcd device error");
        return;
    }
    write(clcd_fd, message.c_str(), message.size());
    close(clcd_fd);
}

class SecurityKey {
private:
    char storedPassword[5];  // 저장된 비밀번호 (최대 4자리 + null 종단 문자)
    vector<char> inputPassword; // 입력된 비밀번호
    int attemptCount;        // 비밀번호 시도 횟수
    bool error;              // 에러 플래그
    time_t lastInputTime;    // 마지막 입력 시간

public:
    SecurityKey() {
        strcpy(storedPassword, "1234");
        attemptCount = 0;
        error = false;
        lastInputTime = time(NULL);
        std::cout << "SecurityKey initialized." << std::endl;
    }

    bool getError() const {
        return error;
    }

    void setError(bool value) {
        error = value;
    }

    void resetInput() {
        inputPassword.clear();
    }

    void displayMessage(const string& message, int duration) {
        writeCLCD(message);
        printf("CLCD Output: %s\n", message.c_str());
        sleep(duration);
        writeCLCD("Standby screen");
        printf("CLCD Output: Standby screen\n");
    }

    bool isPasswordCorrect() {
        if (inputPassword.size() < 4) return false;
        string enteredPassword(inputPassword.end() - 4, inputPassword.end());
        return enteredPassword == storedPassword;
    }

    void checkPassword(char modeChar) {
        std::cout << "Checking password..." << std::endl;
        if (isPasswordCorrect()) {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            char timeStr[20];
            sprintf(timeStr, "%02d:%02d:%02d", ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

            std::string modeStr = (modeChar == '*') ? "go to work" : "leave work";
            std::string fullMessage = std::string(timeStr) + "\n" + modeStr;

            writeCLCD(fullMessage);
            printf("CLCD Output: %s\n", fullMessage.c_str());
            playSound(OPEN_SOUND_FILE); // 비밀번호가 맞으면 성공 사운드 재생
            sleep(DISPLAY_DURATION); // 메시지 5초간 표시
            writeCLCD("Standby screen");
            printf("CLCD Output: Standby screen\n");
        } else {
            attemptCount++;
            if (attemptCount >= 5) {
                std::string message = "Incorrect password. Alarm triggered!";
                std::cout << message << std::endl;
                displayMessage(message, DISPLAY_DURATION);
                playSound(ALARM_SOUND_FILE); // 비밀번호 5번 실패 시 실패 사운드 재생
            } else {
                std::string message = "Incorrect password.";
                std::cout << message << std::endl;
                displayMessage(message, DISPLAY_DURATION);
            }
        }
        resetInput(); // 입력 초기화
    }

    char getKeypadInput() {
        unsigned char b;
        int tactswFd = open(TACT, O_RDONLY);
        if (tactswFd < 0) {
            std::perror("tact device error");
            return '\0';
        }

        if (read(tactswFd, &b, sizeof(b)) < 0) {
            std::perror("read error");
            close(tactswFd);
            return '\0';
        }

        close(tactswFd);

        switch (b) {
        case 1: return '1';
        case 2: return '2';
        case 3: return '3';
        case 4: return '4';
        case 5: return '5';
        case 6: return '6';
        case 7: return '7';
        case 8: return '8';
        case 9: return '9';
        case 10: return '*';
        case 11: return '0';
        case 12: return '#';
        default: return '\0';
        }
    }

    void handleKeyPress() {
        char keyInput = getKeypadInput();
        if (keyInput) {
            lastInputTime = time(NULL); // 입력 시간 업데이트
            std::cout << "Handling key press..." << std::endl;
            printf("Key pressed: %c\n", keyInput);
            if (keyInput == '*' || keyInput == '#') {
                attemptCount++; // 실패 카운트 증가
                if (inputPassword.size() >= 4) {
                    checkPassword(keyInput);
                } else {
                    std::string message = "Please enter a password";
                    std::cout << message << std::endl;
                    displayMessage(message, DISPLAY_DURATION);
                }
                resetInput(); // 입력 초기화
            } else {
                inputPassword.push_back(keyInput);
                printf("Current input: ");
                for (size_t i = 0; i < inputPassword.size(); ++i) {
                    printf("%c", inputPassword[i]);
                }
                printf("\n");
            }
        } else {
            // 입력이 없으면 대기화면 출력
            time_t currentTime = time(NULL);
            if (difftime(currentTime, lastInputTime) >= WAITING_SCREEN_INTERVAL) {
                std::string message = "Standby screen";
                writeCLCD(message);
                printf("CLCD Output: %s\n", message.c_str());
                lastInputTime = currentTime; // 대기 화면 출력 시간 업데이트
            }
        }
    }
};

int main() {
    std::cout << "Security Key Program Started" << std::endl;
    SecurityKey key;

    writeCLCD("Standby screen");
    printf("CLCD Output: Standby screen\n");

    while (true) {
        key.handleKeyPress();
        if (key.getError()) {
            std::cout << "Error occurred. Resetting..." << std::endl;
            key.setError(false);
            sleep(1);  // 간단한 지연
        } else {
            usleep(100000);  // 반복 주기 조정
        }
    }

    return 0;
}
