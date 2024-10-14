#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>

int main() {
    // Serial port connection (see Arduino IDE for references)
    int serial_port = open("/dev/ttyACM0", O_RDWR);


    // Create new termios struct, we call it 'tty' for convention
    struct termios tty;

    // Read in existing settings, and handle any error
    if(tcgetattr(serial_port, &tty) != 0) {
        printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
    tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size
    tty.c_cflag |= CS8; // 8 bits per byte (most common)
    tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
        return 1;
    }

    /*
     * Here begins the sending data example
     */

    // 1 = Left, 2 = Right, 0 = None
    int trafficLights[] = {0, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2, 2, 0};

    // How many ships on left line?
    int leftLines[] = {0, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9};

    // How many ships on right line?
    int rightLines[] = {0, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2};

    // If there's a ship in the channel, what's its progress percentage? Else, 0
    int channels[] = {0, 10, 35, 56, 69, 97, 0, 3, 22, 41, 79, 90, 0};

    int i;
    int trafficLight;
    int leftLine;
    int rightLine;
    int channel;

    for (i = 0; i < 13; i++) {
        trafficLight = trafficLights[i];
        leftLine = leftLines[i];
        rightLine = rightLines[i];
        channel = channels[i];

        // Define data to be sent
        unsigned int msg[] = { trafficLight, leftLine, rightLine, channel };
        write(serial_port, msg, sizeof(msg)); // Send data to serial port

        // Sent data control
        printf("Traffic Light: %d\n", trafficLight);
        printf("Left Line: %d\n", leftLine);
        printf("Right Line: %d\n", rightLine);
        printf("Channel: %d\n", channel);
        printf("\n\n\n");
        sleep(3); // Needed to visualize the progress
    }

}
