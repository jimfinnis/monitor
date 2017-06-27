/**
 * \file
 * UDP server implementation
 *
 * 
 * \author $Author$
 * \date $Date$
 */


#include <stdio.h>
#include <sys/socket.h>

#include "udp.h"
#include "doubletime.h"
#include "diamond.h"

UDPServer::UDPServer(int port){
    listener = NULL;
    
    // set up the receive socket
    
    sock = new QUdpSocket(this);
    sock->bind(port,QUdpSocket::ShareAddress);
    
    int v = 8192;
    if(::setsockopt(sock->socketDescriptor(),SOL_SOCKET,SO_RCVBUF,
                    (char *)&v,sizeof(v))){
        printf("cannot set socket buffer size\n");
        exit(0);
    }
    
    connect(sock,SIGNAL(readyRead()),this,SLOT(readPendingDatagrams()));
    
}

UDPServer::~UDPServer(){
    delete sock;
}


void UDPServer::readPendingDatagrams(){
    while(sock->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(sock->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        sock->readDatagram(datagram.data(),datagram.size(),
                           &sender,&senderPort);
        UDPClient::getInstance()->setAddress(sender);
        listener->processUDP(datagram.data(),datagram.size());
//        printf("Got packet %d bytes\n",datagram.size());
    }
}


/*
 * 
 * Client side 
 *
 */



OutValue::OutValue(const char *n,float init,bool alw){
    name = (const char *)strdup(n);
    always=alw;
#if DIAMOND
    isDiamond=false;
#endif
    val=init;
    listener=NULL;
    timeChanged = -1;
    timeSent = 0;
}

void OutValue::set(float v){
    val = v;
    timeChanged = gettime();
    printf("Set called\n");
}

UDPClient *UDPClient::instance= NULL;

// typically used for sending waypoint packets
void UDPClient::send(const char *s){
    if(!hasAddress){
        printf("can't send : no address yet\n");
        return; 
    }
    sock.writeDatagram(s,strlen(s)+1,addr,port);
}


void UDPClient::update(){
    bool send=false;
    int ct=0;
    
    foreach(OutValue * const &v, values){
        if(v->always || v->timeChanged>v->timeSent)ct++;
    }
    if(!ct)return;
    
    QString out;
    QTextStream str(&out);
    
    str.setRealNumberNotation(QTextStream::FixedNotation);
    //we don't actually need to timestamp outbound data and its eating bandwidth
    //str << "time=" << gettime(); 
    
    
    foreach(OutValue * const &v, values){
        if(v->always || v->timeChanged>v->timeSent){
#if DIAMOND
            if(v->isDiamond){
                diamondapparatus::Topic t;
                t.add(diamondapparatus::Datum(v->val));
                diamondapparatus::publish(v->topic,t);
            }
            else
#else
            {
                QString s;
                printf("sending %s %s (%f > %f)\n",v->name,
                       v->always?"alw":"not-alw",v->timeChanged,v->timeSent);
                QTextStream(&s) << " " << v->name << "=" << v->val;
    
                out.append(s);
            }
#endif
            v->timeSent =  gettime();
            
            if(v->listener)
                v->listener->onSend();
        }
    }
    
    qDebug() << out;
    out.append("\n");
    if(!hasAddress){
        printf("can't send : no address yet\n");
        return; 
    }
    
    sock.writeDatagram(out.toLatin1(),addr,port);
}




extern "C" {

void udpSendFunc(const char *s) {
    printf("Sending: %s\n",s);fflush(stdout);
    UDPClient::getInstance()->send(s);
}

}
