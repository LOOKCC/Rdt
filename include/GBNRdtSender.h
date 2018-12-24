#ifndef GBN_RDT_SENDER_H
#define GBN_RDT_SENDER_H
#include "../include/RdtSender.h"
#include <vector>
#include <fstream>
class GBNRdtSender :public RdtSender
{
private:
	int expectSequenceNumberSend;	// 下一个发送序号 
	bool waitingState;				// 是否处于等待Ack的状态
	// Packet packetWaitingAck;		//已发送并等待Ack的数据包
    std::vector<Packet*> window;
    int window_size; 
    int max_seqnum;
	ofstream fout;

public:
	int get_count(int x);
	bool getWaitingState();
	bool send(Message &message);						//发送应用层下来的Message，由NetworkServiceSimulator调用,如果发送方成功地将Message发送到网络层，返回true;如果因为发送方处于等待正确确认状态而拒绝发送Message，则返回false
	void receive(Packet &ackPkt);						//接受确认Ack，将被NetworkServiceSimulator调用	
	void timeoutHandler(int seqNum);					    //Timeout handler，将被NetworkServiceSimulator调用

public:
	GBNRdtSender();
	virtual ~GBNRdtSender();
};

#endif