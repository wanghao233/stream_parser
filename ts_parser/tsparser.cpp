#include <tsparser.h>

#include <iostream>

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using std::cout;
using std::endl;

TS_Parser::TS_Parser(char * file)
{
    m_iomethod = new STREAMIO::IOMethod(file);
    cout<<__FUNCTION__<<"file:"<<file<<endl;	
}

TS_Parser::~TS_Parser()
{
    cout<<__FUNCTION__<<endl;
    delete m_iomethod;
}

void TS_Parser::parsePAT()
{
	getOffsetOfPid(0);
}

//get the offset of buffer.  from header of buffer.
uint64_t TS_Parser::getOffsetOfPid(uint16_t pid)
{
	for(uint16_t i = 0; i < 200; i ++){
		TS_Header_st header = {0};

		uint8_t * bytes8 = local_buffs + first_sync_offset + i * TS_PKG_LENGTH;

		header.SYNCBYTE = bytes8[0];
		header.transport_error_indicator    = bytes8[1] >> 7;
		header.payload_unit_start_indicator = bytes8[1] >> 6 & 0x01 ;
		header.PID                          = ( ( (uint16_t)bytes8[1] & 0x1f) << 8) + bytes8[2];
		fprintf(stderr,"sync:%#x,err-indicator:%d,start-indicator:%d,pid:%#x\n",header.SYNCBYTE,header.transport_error_indicator,
				header.payload_unit_start_indicator,header.PID);
		if(header.PID == pid){
			cout<<"found pid: i="<<i<<" !"<<endl;
			return (first_sync_offset + i * TS_PKG_LENGTH);
		}
	}

	return 0;
}

void TS_Parser::sync_offset()
{
	uint32_t i=0;
        if( m_iomethod->io_read( local_buffs , sizeof(local_buffs)) <= 0 ) return;

	do{
	     if(local_buffs[i] == 0x47 &&  local_buffs[i+TS_PKG_LENGTH]== 0x47 &&  local_buffs[i+2*TS_PKG_LENGTH] == 0x47){
		first_sync_offset = i;
		cout<<"ts sync-byte found: i = "<<i<<" !"<<endl;
		return;
	      }
	} while(i++ && i < TS_PKG_LENGTH);

	cout<<"ts sync-byte not found !"<<endl;
}
