#include "Timer.h"
#include "UdpSocket.h"

#define TIMEOUT_INTERVAL = 1500

int handleAck(UdpSocket &sock, Timer clock, int message[]);

/*
 * sends message[] and receives an acknowledgment from the server max (=20,000) times using the sock object.
 * If the client cannot receive an acknowledgment immediately, it should start a Timer.
 * If a timeout occurs (i.e., no response after 1500 usec), the client must resend the same message.
 * The function must count the number of messages retransmitted and return it to the main function as its return value.
 */
int clientStopWait( UdpSocket &sock, const int max, int message[]) {
    int retransmitCount=0;
    Timer clock= new Timer();
    for (int i=0; i<max; i++){
        message[0]=i;
        sock.sendTo((char *)message, sizeof(message));
        int response;
        if (pollRecvFrom()>0 && recvFrom((char *)response, sizeof(response))&&response==i)
            continue;
        clock.start();
        retransmitCount+=handleAck(sock, clock, message);
    }
    return retransmitCount;
}

int handleAck(UdpSocket &sock, Timer clock, int message[]){
    while(!(pollRecvFrom()>0 && recvFrom((char *)response, sizeof(response))&&response==i)){
        if (clock.lap()>=TIMEOUT_INTERVAL){
            sock.sendTo((char *)message, sizeof(message));
            retransmitCount++;
            return 1+=handleAck(sock, clock, message);
        }
    }
    return 0;
}

/*
 * repeats receiving message[] and sending an acknowledgment at a server side max (=20,000) times using the sock object.
 */
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
