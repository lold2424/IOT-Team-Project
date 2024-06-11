#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <nfc/nfc.h>

#define LED_DEVICE "/dev/led" // LED ��ġ ���
#define DIP_DEVICE "/dev/dipsw" // DIP ����ġ ��ġ ���
#define ULTRASONIC_DEVICE "/dev/ultrasonic" // ������ ���� ��ġ ��� (����)

// ��ġ ���� ����
int open_device(const char* device_path) {
    int device = open(device_path, O_RDWR);
    if (device < 0) {
        printf("Can't open %s.\n", device_path);
        exit(0);
    }
    return device;
}

// LED ���� ����
void set_led_state(int dev, unsigned char data) {
    write(dev, &data, sizeof(unsigned char));
}

// DIP ����ġ ���� �б�
unsigned char read_dip_switch(int dip_d) {
    unsigned char c;
    read(dip_d, &c, sizeof(c));
    return c;
}

// ������ ��ȣ ó�� �� LED ����
void handle_ultrasonic_signal(int ultrasonic_d, int dip_d, int led_d) {
    unsigned char c, data = 0xff;
    unsigned char ultrasonic_signal = 0;

    // ������ ��ȣ�� �о���� ���� (����)
    // read(ultrasonic_d, &ultrasonic_signal, sizeof(ultrasonic_signal));

    // ������ ��ȣ�� �ִ� ���
    if (ultrasonic_signal) {
        data = 0x00; // ��� LED �ѱ�
        c = read_dip_switch(dip_d);

        // DIP ����ġ�� ON�� ��� �ش� LED ����
        if (c & 0x01) data |= 0x01;
        if (c & 0x02) data |= 0x02;
        if (c & 0x04) data |= 0x04;
        if (c & 0x08) data |= 0x08;
        if (c & 0x10) data |= 0x10;
        if (c & 0x20) data |= 0x20;
        if (c & 0x40) data |= 0x40;
        if (c & 0x80) data |= 0x80;

        set_led_state(led_d, data);
    }
}

// ��� �� ��� ó��
void handle_attendance(int dip_d) {
    unsigned char c = read_dip_switch(dip_d);

    if (c & 0x01) {
        printf("��� ó��\n");
    }
    if (c & 0x02) {
        printf("��� ó��\n");
    }
}

// NFC �±� ���� ó��
void handle_nfc(nfc_device* pnd) {
    nfc_target nt;
    const nfc_modulation nmModulations[] = {
        {.nmt = NMT_ISO14443A, .nbr = NBR_106 }
    };
    const size_t szModulations = 1;
    const uint8_t uiPollNr = 1;
    const uint8_t uiPeriod = 2;

    if (nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt) > 0) {
        // NFC �±װ� �����Ǿ��� �� �޽��� ���
        printf("NFC tag detected!\n");
    }
    else {
        // NFC �±װ� �������� �ʾ��� �� �޽��� ���
        printf("No NFC tag detected.\n");
    }
}

int main() {
    int led_d, dip_d, ultrasonic_d;
    nfc_device* pnd;
    nfc_context* context;

    // ��ġ ���� ����
    led_d = open_device(LED_DEVICE); // LED ��ġ ���� ����
    dip_d = open_device(DIP_DEVICE); // DIP ����ġ ��ġ ���� ����
    ultrasonic_d = open_device(ULTRASONIC_DEVICE); // ������ ���� ��ġ ���� ����

    // libnfc �ʱ�ȭ
    nfc_init(&context);
    if (context == NULL) {
        fprintf(stderr, "Unable to init libnfc\n");
        return EXIT_FAILURE;
    }

    // NFC ����̽� ����
    pnd = nfc_open(context, NULL);
    if (pnd == NULL) {
        fprintf(stderr, "Unable to open NFC device\n");
        nfc_exit(context);
        return EXIT_FAILURE;
    }

    // NFC ����̽� ����
    if (nfc_initiator_init(pnd) < 0) {
        nfc_perror(pnd, "nfc_initiator_init");
        nfc_close(pnd);
        nfc_exit(context);
        return EXIT_FAILURE;
    }

    printf("NFC reader: %s opened\n", nfc_device_get_name(pnd));

    while (1) {
        handle_ultrasonic_signal(ultrasonic_d, dip_d, led_d);
        handle_attendance(dip_d);
        handle_nfc(pnd);

        usleep(200000); // 200�и��� ���
    }

    // ���ҽ� ����
    close(dip_d);
    close(led_d);
    close(ultrasonic_d);
    nfc_close(pnd);
    nfc_exit(context);

    return 0;
}
