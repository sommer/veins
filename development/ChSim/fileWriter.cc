#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <omnetpp.h>
#include "channelStateMsg_m.h"

using namespace std;

// module class declaration:
class fileWriter:public cSimpleModule {
      public:
		Module_Class_Members(fileWriter, cSimpleModule, 0)
		virtual void initialize();
		virtual void handleMessage(cMessage * msg);
		virtual void finish();

      private:
      	void initChannelStateBuf();
		void deleteChannelStateBuf();
		void printChannelStateFiles();
		
		// file writing buffer		
		double ****channelState_buf;		// matrix for buffering the arriving values
		double MEM_BUF_SIZE;		// memory size for filewriting buffer in Megabyte
		int MAX_VALUES_IN_BUF;	// # values to fit in the buffer
		int buf_counter;		// counts the current position in buffer
		bool filesInitialized;

		int J; // # of ms in cell
		int C; // # of Channels per ms
		int S; // # of Subcarriers in cell	

		// values to show up in filename
	    double MAX_TIME;		// time length (in seconds) for the simulation
		int SAMPLE_PER_SEC;		// how often channelState samples should be generated per second, e.g. 500 means every 2ms

		int WRITEFILES;
};

// module type registration:
Define_Module(fileWriter);

void fileWriter::initialize()
{
	ev << "fileWriter->initialize()" << endl;
	
	J = par("qty_ms");
	C = par("qty_channels");
	S = par("subbands");
	
	MAX_TIME = par("max_time");
	SAMPLE_PER_SEC = par("sample_per_sec");
	
	WRITEFILES = par("writeFiles");
	
	MEM_BUF_SIZE = par("mem_buf_size");
	MAX_VALUES_IN_BUF = ((int)(MEM_BUF_SIZE * 1024 * 1024)) / (J * C * S) / 8;	// J*C*S double values, double = 8byte
	ev << "buffer size: " << MEM_BUF_SIZE << "Mb -> value pairs: " << MAX_VALUES_IN_BUF << endl;

	initChannelStateBuf();
	buf_counter = 0;

	filesInitialized = false;
}

// implementation of the module class:
void fileWriter::handleMessage(cMessage * msg)
{
		channelStateMsg *channelStatemsg = (channelStateMsg *) msg;
/*
		ev << "got channelState's from ms(" << channelStatemsg->getMsId() << ").(" << channelStatemsg->getMsChannel() << "): ";
		for (int s = 0; s < S; ++s) {
			ev << channelStatemsg->getchannelState(s) << " ";
		}
		ev << endl;
*/
		for (int s = 0; s < S; ++s) {									// values for all subbands
			channelState_buf[channelStatemsg->getMsId()][channelStatemsg->getMsChannel()][buf_counter][s] = channelStatemsg->getChannelState(s);
		}
		if(channelStatemsg->getMsId() == J - 1 and channelStatemsg->getMsChannel() == C - 1)
			buf_counter++;
			if(buf_counter == MAX_VALUES_IN_BUF) {
			printChannelStateFiles();
			buf_counter = 0;
		}
		delete msg;
}

void fileWriter::finish()
{
	ev << "fileWriter->finish()" << endl;
	printChannelStateFiles();
	deleteChannelStateBuf();
}

// allocates the necessary memory for the channelState buffer
void fileWriter::initChannelStateBuf()
{
	channelState_buf = new double ***[J]; 					// # ms
	for (int j = 0; j < J; j++) {
		channelState_buf[j] = new double **[C];				// # Channels
		for (int c = 0; c < C; c++) {
			channelState_buf[j][c] = new double *[MAX_VALUES_IN_BUF]; // # values in buf
			for (int i = 0; i < MAX_VALUES_IN_BUF; i++) {
				channelState_buf[j][c][i] = new double[S];	// # Subbands
			}
		}
	}
}

void fileWriter::deleteChannelStateBuf()
{
	for (int j = 0; j < J; j++) {
		for (int c = 0; c < C; c++) {
			for (int i = 0; i < MAX_VALUES_IN_BUF; i++) {
				delete []channelState_buf[j][c][i];
			}
			delete []channelState_buf[j][c];
		}
		delete []channelState_buf[j];
	}
	delete []channelState_buf;
}

// This prints the channelState matrix, including the fading, shadowing and path loss
void fileWriter::printChannelStateFiles()
{
	string filename;
	const char *directory_name;
	const char *filename_prefix;

	directory_name = par("directory_name");
	filename_prefix = par("filename_prefix");

	if (WRITEFILES) {
		for (int j = 0; j < J; j++)	// count ms
		{
			for (int c = 0; c < C; ++c) {	// count Channels
				ostringstream oss;
				oss << directory_name << "/" << filename_prefix << "-" << MAX_TIME << "s" << SAMPLE_PER_SEC << "sps" << S << "sb" << "-ms" << j << "ch" << c << ".trace";
				filename = oss.str();
				if (!filesInitialized)
				{
					ev << "init file: " << filename << endl;
					ofstream output(filename.c_str(), ios::trunc);
				}
				ofstream output(filename.c_str(), ios::app);
				if(!output) {
					ev << "ERROR: " << filename << " could not be opened!\n";
					endSimulation();
				}
				output.setf(ios::fixed);
				output.precision(par("precision"));
				for (int i = 0; i < buf_counter; ++i) {
					output << channelState_buf[j][c][i][0];		//first values without delimiter
					for (int s = 1; s < S; s++) {
						output << " ";					//delimiter
						output << channelState_buf[j][c][i][s];	//values
					}
					output << endl;
				}
			}
		}
	}
	filesInitialized = true;
}
