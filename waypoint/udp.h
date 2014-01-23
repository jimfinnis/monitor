/**
 * \file
 * test udp code
 * 
 * \author $Author$
 * \date $Date$
 */


#ifndef __UDP_H
#define __UDP_H

void udpSend(int port,const char *s);
void udpInitRecv(int port);
const char *udpPoll(long waitms);
void udpClose();


#endif /* __UDP_H */
