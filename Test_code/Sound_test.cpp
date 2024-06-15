#include <iostream>
#include <cstdlib>
#include <unistd.h> // sleep 함수를 사용하기 위해 추가

#define SOUND_FILE "/alarm_sound.wav" // 재생할 사운드 파일 경로

void playSound(const char* soundFile, int durationSeconds) {
    std::string command = "aplay ";
    command += soundFile;
    command += " &"; // 백그라운드에서 실행
    system(command.c_str());

    sleep(durationSeconds); // 지정된 시간 동안 재생

    // aplay 프로세스를 중지시킴
    system("pkill aplay");
}

int main() {
    std::cout << "Playing sound for 10 seconds..." << std::endl;
    playSound(SOUND_FILE, 10); // 재생 시간 10초
    std::cout << "Sound playback finished." << std::endl;
    return 0;
}
