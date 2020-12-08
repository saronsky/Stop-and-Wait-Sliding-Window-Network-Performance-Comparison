#include "Timer.h"
#include "UdpSocket.h"

#define TIMEOUT_INTERVAL 1500 //timeout of 1500usec

/**/
int clientStopWait( UdpSocket &sock, const int max, int message[]) {
    int numResent = 0;
    for (int i = 0; i < max; i++) {
		//message[0] is packet #; see hw3.cpp's unreliable test
        message[0] = i;
        sock.sendTo((char*)message, MSGSIZE);
        bool timeoutFlag = false;
        Timer time;
        time.start();
        while (1) {
			int numBytesToRecv = sock.pollRecvFrom();
            if (numBytesToRecv > 0) {
                break; //because there is data to receive
            }
            if (time.lap() > TIMEOUT_INTERVAL && !timeoutFlag) {
                timeoutFlag = true;
                break; //because we timed out
            }
        }
        if (timeoutFlag) {
            //we timed out; resend
            i--;
            numResent++;
            continue;
        }
        sock.recvFrom((char*)message, MSGSIZE);
        if (message[0] != i) {
            //we didn't get the correct ACK; resend
            i--;
            numResent++;
            continue;
        }
    }
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
		
		//if we have empty slots, send a packet until we have all slots filled
        if (numUnacked < windowSize) {
            message[0] = i;
            sock.sendTo((char*)message, MSGSIZE);
            numUnacked++;
        }

		//if we have a full queue, start processing data
        if (numUnacked == windowSize) {
            Timer time;
            time.start();
            while (1) {
				int numBytesToRecv = sock.pollRecvFrom();
                if (numBytesToRecv > 0) {
                    sock.recvFrom((char*)message, MSGSIZE);
                    if (message[0] == lastAck) {
                        lastAck++;
                        numUnacked--;
                        break;
                    }
                }
				//
                if (time.lap() > TIMEOUT_INTERVAL && numUnacked == windowSize) {
                    numResent = numResent + (i + windowSize - lastAck);
                    i = lastAck;
                    numUnacked = 0;
                    continue;
                }
            }
        }
    }
    return numResent;
}

/**/
void serverEarlyRetrans( UdpSocket &sock, const int max, int message[], int windowSize, int dropRate) {
    for (int i = 0; i < max; i++) {
        while (1) {
            if (sock.pollRecvFrom() > 0) {
                int randomNum = rand() % 101;
                if (randomNum < dropRate) {
                    continue;
                }
                sock.recvFrom((char *) message, MSGSIZE);
                sock.ackTo((char *) &i, sizeof(int));
                if (message[0] == i) {

                    break; //the loop when the server receives the entire message

                }
            }
        }
    }
}
