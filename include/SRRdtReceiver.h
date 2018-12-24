#ifndef SR_RDT_RECEIVER_H
#define SR_RDT_RECEIVER_H
#include "../include/RdtReceiver.h"
#include <vector>
#include <fstream>
class SRRdtReceiver :public RdtReceiver
{
private:
	// int expectSequenceNumberRcvd;	// 期待收到的下一个报文序号
	Packet lastAckPkt;				//上次发送的确认报文
    int max_seqnum;
    int window_size;
    std::vector<Packet> window;
	std::vector<int> expectSequenceNumberRcvd;
	std::vector<int> is_recd;
	std::ofstream fout;

public:
	SRRdtReceiver();
	virtual ~SRRdtReceiver();

public:
	int get_index(int x);
	void receive(Packet &packet);	//接收报文，将被NetworkService调用
};

#endif