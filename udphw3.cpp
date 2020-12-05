#include "../432hw3/Timer.h"
#include "../432hw3/UdpSocket.h"

#define TIMEOUT_INTERVAL = 1500

/**/
int clientStopWait( UdpSocket &sock, const int max, int message[]) {
    int numResent = 0;
    return numResent;
}

/**/
void serverReliable( UdpSocket &sock, const int max, int message[]) {
    for (int i = 0; i < max; i++) {
        while (1) {
            if (sock.pollRecvFrom() > 0) {
                sock.recvFrom((char *) message, MSGSIZE);
                if (message[0] == i) {
                    sock.ackTo((char *) &i, sizeof(int));
                    break; //the loop when the server receives the entire message
                    //ack should only be sent once the message gets received
                }
            }
        }
    }
}

/**/
int clientSlidingWindow( UdpSocket &sock, const int max, int message[], int windowSize) {
    int lastAck = 0;
    int numUnacked = 0;
    int numResent = 0;
    bool timeoutFlag = 0;
    for (int i = 0; i < max; i++) {

        if (numUnacked < windowSize) {
            message[0] = i;
            sock.sendTo((char*)message, MSGSIZE);
            numUnacked++;
        }

        if (numUnacked == windowSize) {
            Timer time;
            time.start();
            while (1) {

                if (sock.pollRecvFrom() > 0) {
                    sock.recvFrom((char*)message, MSGSIZE);
                    if (message[0] == lastAck) {
                        lastAck++;
                        numUnacked--;
                        break;
                    }
                }
                if (time.lap() > TIMEOUT_INTERVAL && numUnacked == windowSize) {
                    numResent = numResent + (i + windowSize - lastAck);
                    i = lastAck;
                    numUnacked = 0;
                    break;
                }
            }
        }
    }
    return numResent;
}

/**/
void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], int windowSize) {
    for (int i = 0; i < max; i++) {
        while (1) {
            if (sock.pollRecvFrom() > 0) {
                sock.recvFrom((char *) message, MSGSIZE);
                sock.ackTo((char *) &i, sizeof(int));
                if (message[0] == i) {

                    break; //the loop when the server receives the entire message

                }
            }
        }
    }
}