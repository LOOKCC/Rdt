#include "../include/Base.h"
#include "../include/Global.h"
#include "../include/TCPSender.h"


TCPSender::TCPSender():expectSequenceNumberSend(0),waitingState(false),
    window_size(Configuration::WINDOW_SIZE),max_seqnum(Configuration::MAX_SEQNUM),ack_times(0)
{
	this->fout.open("TCPOut.txt", ios::app);
}


TCPSender::~TCPSender()
{
	this->fout.close();
}



bool TCPSender::getWaitingState() {
	return waitingState;
}




bool TCPSender::send(Message &message) {
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

	fout<<"after sending: ";
	for(int i=0; i<this->window.size(); i++){
		fout<<this->window[i]->seqnum<<" ";
	}
	fout<<endl;

	if(this->window.size() == this->window_size){
        this->waitingState = true;
    }
	return true;
}

int TCPSender::get_count(int x){
	for(int i=0; i<this->window.size(); i++){
		if(this->window[i]->seqnum == x)
			return i;
	}
	return -1;
}


void TCPSender::receive(Packet &ackPkt) {
	//检查校验和是否正确
	int checkSum = pUtils->calculateCheckSum(ackPkt);
	//如果校验和正确，并且确认序号=发送方已发送并等待确认的数据包序号
	if (checkSum == ackPkt.checksum) {
		int count = get_count(ackPkt.acknum);
        if(count != -1){
            pUtils->printPacket("发送方正确收到确认", ackPkt);
            for(int i = 0; i < count+1; i++){
                pns->stopTimer(SENDER, this->window.front()->seqnum);		//关闭定时器
                this->window.erase(this->window.begin());
            }
            this->waitingState = false;
			this->ack_times = 0;
        }else{
            this->ack_times++;
            if(this->ack_times == 3){
                // send packets
				for(int i=0; i<window.size(); i++){
					pns->stopTimer(SENDER, this->window[i]->seqnum);										//首先关闭定时器
					pns->startTimer(SENDER, Configuration::TIME_OUT, this->window[i]->seqnum);			//重新启动发送方定时器
					pns->sendToNetworkLayer(RECEIVER, *this->window[i]);			//重新发送数据包
				}
                this->ack_times = 0;
				fout<<"3 times ACK: ";
				for(int i=0; i<this->window.size(); i++){
					fout<<this->window[i]->seqnum<<" ";
				}
				fout<<endl;
            }
        }
		fout<<"after receiving: ";
		for(int i=0; i<this->window.size(); i++){
			fout<<this->window[i]->seqnum<<" ";
		}
		fout<<endl;
	}	
}

void TCPSender::timeoutHandler(int seqNum) {
	int count = get_count(seqNum);
	if(count == -1)
		return ;
    //send pkg
    for(int i=0; i<this->window.size(); i++){
		 pUtils->printPacket("发送方定时器时间到，重发上次发送的报文", *this->window[i]);
		pns->stopTimer(SENDER, this->window[i]->seqnum);										//首先关闭定时器
		pns->startTimer(SENDER, Configuration::TIME_OUT, this->window[i]->seqnum);			//重新启动发送方定时器
		pns->sendToNetworkLayer(RECEIVER, *this->window[i]);			//重新发送数据包
	}
}

