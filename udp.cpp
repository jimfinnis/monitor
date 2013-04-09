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

UDPServer::UDPServer(int port){
    listener = NULL;
    
    sock = new QUdpSocket(this);
    sock->bind(QHostAddress::LocalHost,port);
    
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
        listener->processUDP(datagram.data(),datagram.size());
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
    val=init;
    listener=NULL;
}

void OutValue::set(float v){
    val = v;
    timeChanged = gettime();
}

UDPClient *UDPClient::instance= NULL;


void UDPClient::update(){
    QString out;
    QTextStream str(&out);
    
    str.setRealNumberNotation(QTextStream::FixedNotation);
    str << "time=" << gettime();
    
    QString s;
    
    if(values.size()==0)return;
    
    foreach(OutValue * const &v, values){
        if(v->always || v->timeChanged>v->timeSent){
            QTextStream(&s) << " " << v->name << "=" << v->val;
            out.append(s);
            v->timeSent =  gettime();
            
            if(v->listener)
                v->listener->onSend();
        }
    }
    sock.writeDatagram(out.toLatin1(),addr,port);
//    qDebug() << out;
}


