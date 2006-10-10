message channelStateMsg
{
	fields:
		int msId;
		int msChannel;
		double xPos;
		double yPos;
		double distance;
		double speed;
		double channelState[];
		int errors;
};
