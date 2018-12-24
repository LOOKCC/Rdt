#include "../include/Base.h"
#include "../include/Global.h"
#include "../include/SRRdtReceiver.h"


SRRdtReceiver::SRRdtReceiver():expectSequenceNumberRcvd(0),window_size(Configuration::WINDOW_SIZE),max_seqnum(Configuration::MAX_SEQNUM)
{
	lastAckPkt.acknum = -1; //初始状态下，上次发送的确认包的确认序号为-1，使得当第一个接受的数据包出错时该确认报文的确认号为-1
	lastAckPkt.checksum = 0;
	lastAckPkt.seqnum = -1;	//忽略该字段
	for(int i = 0; i < Configuration::PAYLOAD_SIZE;i++){
		lastAckPkt.payload[i] = '.';
	}
	lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
	for(int i=0; i<this->window_size; i++){
		this->expectSequenceNumberRcvd.push_back(i);
		Packet packet;
		this->window.push_back(packet);
		this->is_recd.push_back(0);
	}
	this->fout.open("../SRRecOut.txt", ios::app);
}


SRRdtReceiver::~SRRdtReceiver()
{
	this->fout.close();
}

int SRRdtReceiver::get_index(int x){
	for(int i=0; i<this->expectSequenceNumberRcvd.size(); i++){
		if(this->expectSequenceNumberRcvd[i] == x)
			return i;
	}
	return -1;
}


void SRRdtReceiver::receive(Packet &packet) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(packet);
	int index = get_index(packet.seqnum);
	//如果校验和正确，同时收到报文的序号等于接收方期待收到的报文序号一致
	if (checkSum == packet.checksum && index != -1 ) {
		pUtils->printPacket("接收方正确收到发送方的报文", packet);


		this->window[index] = packet;
		this->is_recd[index] = 1;
		lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
		lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
		pUtils->printPacket("接收方发送确认报文", lastAckPkt);
		pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方

		int count = window_size;
		for(int i=0; i<is_recd.size(); i++){
			if(is_recd[i] == 0){
				count = i;
				break;
			}
		}
		for(int i=0; i<count; i++){
			Message msg;
			memcpy(msg.data, this->window[0].payload, sizeof(this->window[0].payload));
			pns->delivertoAppLayer(RECEIVER, msg);
			int x = this->expectSequenceNumberRcvd[this->window_size-1] + 1;
			if(x == this->max_seqnum)
				x = 0;
			Packet temp;

			this->window.erase(this->window.begin());
			this->expectSequenceNumberRcvd.erase(this->expectSequenceNumberRcvd.begin());
			this->is_recd.erase(this->is_recd.begin());

			this->window.push_back(temp);
			this->expectSequenceNumberRcvd.push_back(x);
			this->is_recd.push_back(0);

		}
		fout<<"after receiving, expected seqnum: ";
		for(int i=0; i<this->expectSequenceNumberRcvd.size(); i++){
			fout<<this->expectSequenceNumberRcvd[i]<<" ";
		}
		fout<<endl;
		fout<<"after receiving, whether received packet ";
		for(int i=0; i<this->is_recd.size(); i++){
			fout<<this->is_recd[i]<<" ";
		}
		fout<<endl;
	}
	else {
		if (checkSum != packet.checksum) {
			pUtils->printPacket("接收方没有正确收到发送方的报文,数据校验错误", packet);
		}
		else {
            // cout<<packet.seqnum<<endl;
            // cout<<this->expectSequenceNumberRcvd<<endl;
			pUtils->printPacket("接收方没有正确收到发送方的报文,报文序号不对", packet);

			lastAckPkt.acknum = packet.seqnum; //确认序号等于收到的报文序号
			lastAckPkt.checksum = pUtils->calculateCheckSum(lastAckPkt);
			pUtils->printPacket("接收方发送确认报文", lastAckPkt);
			pns->sendToNetworkLayer(SENDER, lastAckPkt);	//调用模拟网络环境的sendToNetworkLayer，通过网络层发送确认报文到对方
		}
		
	}
}