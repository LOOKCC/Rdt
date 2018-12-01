#include "../include/Base.h"
#include "../include/Global.h"
#include "../include/SRRdtSender.h"


SRRdtSender::SRRdtSender():expectSequenceNumberSend(0),waitingState(false),window_size(Configuration::WINDOW_SIZE),max_seqnum(Configuration::MAX_SEQNUM)
{
}


SRRdtSender::~SRRdtSender()
{
}



bool SRRdtSender::getWaitingState() {
	return waitingState;
}




bool SRRdtSender::send(Message &message) {
	if (this->waitingState) { //发送方处于等待确认状态
		return false;
	}
    Packet* pkg = new Packet;
	pkg->seqnum =this->expectSequenceNumberSend;
	pkg->acknum = -1; //忽略该字段
    this->expectSequenceNumberSend++;
    if(this->expectSequenceNumberSend == this->max_seqnum)
        this->expectSequenceNumberSend = 0;
	pkg->checksum = 0;
	memcpy(pkg->payload, message.data, sizeof(message.data));
	pkg->checksum = pUtils->calculateCheckSum(*pkg);
	pUtils->printPacket("发送方发送报文", *pkg);
	pns->startTimer(SENDER, Configuration::TIME_OUT,pkg->seqnum);			//启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, *pkg);								//调用模拟网络环境的sendToNetworkLayer，通过网络层发送到对方
    this->window.push_back(pkg);
	this->window_recd.push_back(0);

	if(this->window.size() == this->window_size){
        this->waitingState = true;
    }
	return true;
}

int SRRdtSender::get_count(int x){
	for(int i=0; i<this->window.size(); i++){
		if(this->window[i]->seqnum == x)
			return i;
	}
	return -1;
}


void SRRdtSender::receive(Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum) {
		for(int i=0; i<this->window.size(); i++){
			if(this->window[i]->seqnum == ackPkt.acknum){
				this->window_recd[i] = 1;
			}
		}

		pUtils->printPacket("发送方正确收到确认", ackPkt);
		int count = this->window.size();
		for(int i=0; i<this->window_recd.size(); i++){
			if(this->window_recd[i] == 0){
				count = i;
				break;
			}
		}
		for(int i = 0; i < count; i++){
			pns->stopTimer(SENDER, this->window.front()->seqnum);		//关闭定时器
			this->window.erase(this->window.begin());
			this->window_recd.erase(this->window_recd.begin());

		}
		if(this->window.size() < this->window_size)
			this->waitingState = false;
	}
	else {
		// pUtils->printPacket("发送方没有正确收到确认，重发上次发送的报文", this->packetWaitingAck);
		// pns->stopTimer(SENDER, this->packetWaitingAck.seqnum);									//首先关闭定时器
		// pns->startTimer(SENDER, Configuration::TIME_OUT, this->packetWaitingAck.seqnum);			//重新启动发送方定时器
		// pns->sendToNetworkLayer(RECEIVER, this->packetWaitingAck);								//重新发送数据包
	}	
}

void SRRdtSender::timeoutHandler(int seqNum) {
	int count = get_count(seqNum);
	if(count == -1)
		return ;
	//唯一一个定时器,无需考虑seqNum
	pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", *this->window[count]);
	pns->stopTimer(SENDER, this->window[count]->seqnum);										//首先关闭定时器
	pns->startTimer(SENDER, Configuration::TIME_OUT, this->window[count]->seqnum);			//重新启动发送方定时器
	pns->sendToNetworkLayer(RECEIVER, *this->window[count]);			//重新发送数据包
}

