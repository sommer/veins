message LocPkt {
    fields:
	int type;
	int destAddr;
	int srcAddr;
	char data[];
}