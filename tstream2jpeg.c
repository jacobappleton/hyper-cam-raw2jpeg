// hyper-cam - Interfaces camera sensor with serial communications for
// autonomous image capture of scientific experiments on embedded Linux
//
// Image processing functions.
//
// Copyright (C) 2012 Jacob Appleton
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <www.gnu.org/licenses/>.
//
// This software was developed as part of the Scramspace I Flight Experiment,
// funded by the Australian Space Research Program -
// http://www.space.gov.au/AUSTRALIANSPACERESEARCHPROGRAM and involving:
// - The University of Queensland (UQ) - www.uq.edu.au
// - Australian Government Department of Defence - Defence Science and
//   Technology Organisation (DSTO) - www.dsto.defence.gov.au
// - German Aerospace Center (DLR) - www.dlr.de/en/
// - University of Southern Queensland (USQ) - www.usq.edu.au
// - BAE Systems - www.baesystems.com
// - Japan Aerospace Exploration Agency (JAXA) - www.jaxa.jp/index_e.html
// - University of Minnesota (UMN) - www.umn.edu
// - AIMTEK, Inc. - www.umn.edu
// - Australian Youth Aerospace Association (AYAA) - www.ayaa.com.au
// - Centro Italiano Ricerche Aerospaziali (CIRA) - www.cira.it/en
// - The University of Adelaide - www.adelaide.edu.au
// - Teakle Composites - www.cira.colostate.edu
// - The University of New South Wales (UNSW) - www.unsw.edu.au/
//
// Converts a stream of video data from the Scramspace Flight Experiment to a
// series of JPEG image files. Expects stream in following format:
// 1. header (1 byte) 0xAA
// 2. number of  bytes requested (2 bytes)
// 3. number of bytes contained (2 bytes)
// 4. data bytes (maximum 346 bytes but consistent with number of bytes
//    contained)
// 5. check sum (XOR starting with 0x00 of all byes including header) (1 byte)
//
// You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include "util.h"

typedef unsigned char byte;

int main(int argc, char *argv[]) {

	if(argc != 3)
	{
		exitWithError("Usage: tstream2jpeg data_file_path outputPath");
	}
	char* 	tStreamPath = argv[1];  		  // Set the file in path
	char 	jpegPath[255];   				  // Buffer for current working dir
	int 	i = 0, len, ctr = 0;
	char*   workingDir = argv[2];

	//getcwd(workingDir, 255); 				  // Get current working directory
	FILE* tstream = fopen(tStreamPath, "rb"); // Open the telemetry stream
	fseek(tstream, 0, SEEK_END);			  // Seek to EOF
	len = ftell(tstream);					  // Find file size
	fseek(tstream, 0, SEEK_SET);			  // Seek to beginning of file
	byte* buf = (byte*)malloc(len);			  // Allocate file in memory
	fread(buf, 1, len-1, tstream);			  // Read file into memory
	sprintf(jpegPath, "%s/image%d.jpeg", workingDir, i++);
	FILE* jpeg = fopen(jpegPath, "wb");
	while(ctr < len)						  // While not at end of buffer
	{
		ctr++;								  // Ignore first 0xAA
		ctr += 2;							  // Ignore bytes received
		byte conBytes[2];					  // Buffer for bytes contained
		conBytes[1] = *(buf+(ctr++));		  // Fill buffer
		conBytes[0] = *(buf+(ctr++));		  // Fill buffer
		uint16_t con = byteToInt(conBytes);	  // Get int of bytes contained
		printf("Creating image of size %d bytes\n", con);
		if(*(buf+ctr) == 0xFF && *(buf+ctr+1) == 0xD8)
		{
			fclose(jpeg);
			sprintf(jpegPath, "%s/image%d.jpeg", workingDir, i++);
			jpeg = fopen(jpegPath, "wb");
		}									  // Open JPEG for writing
		fwrite(buf+ctr, 1, con, jpeg);		  // Write to the JPEG
		ctr += con;							  // Move ptr past image data
		ctr++;								  // Ignore checksum
	}
	free(buf);
	fclose(tstream);
	return EXIT_SUCCESS;
}
