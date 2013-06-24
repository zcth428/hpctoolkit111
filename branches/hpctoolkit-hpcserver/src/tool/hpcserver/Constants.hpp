/*
 * Constants.h
 *
 *  Created on: Jul 7, 2012
 *      Author: pat2
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_
namespace TraceviewerServer
{

#define SIZEOF_LONG 8
#define SIZEOF_INT 4
#define SIZEOF_SHORT 2
#define SIZEOF_BYTE 1

#define SIZEOF_DELTASAMPLE (2*SIZEOF_INT) //The CPID is an int, and the delta timestamp is also an int
#define MULTI_PROCESSES  1
#define MULTI_THREADING  2
/**The size of one trace record in bytes (cpid (= 4 bytes) + timeStamp (= 8 bytes)).*/
#define SIZE_OF_TRACE_RECORD (SIZEOF_INT+SIZEOF_LONG)
#define SIZEOF_END_OF_FILE_MARKER 4

	#define DEFAULT_PORT 21590
	#define MAX_DB_PATH_LENGTH 1023

	//Message headers:
	#define DATA 0x44415441
	#define OPEN 0x4F50454E
	#define HERE 0x48455245
	#define DONE 0x444F4E45
	#define DBOK 0x44424F4B
	#define INFO 0x494E464F
	#define NODB 0x4E4F4442
	#define EXML 0x45584D4C
	#define FLTR 0x464C5452
	#define SLAVE_REPLY 0x534C5250
	#define SLAVE_DONE 0x534C444E

	//Error Codes
	//Yes, they are mostly arbitrary
	#define ERROR_STREAM_OPEN_FAILED -3
	#define ERROR_EXPECTED_OPEN_COMMAND -77
	#define ERROR_DB_OPEN_FAILED -4
	#define ERROR_UNKNOWN_COMMAND -7
	#define ERROR_PATH_TOO_LONG -9
	#define ERROR_INVALID_PARAMETERS -99
	#define ERROR_COMPRESSION_FAILED -33445
	#define ERROR_GET_RAM_SIZE_FAILED -4456
	#define ERROR_READ_TOO_LITTLE(z) (-(0x5200 | (z)))
	#define ERROR_STREAM_CLOSED -12
	#define ERROR_SOCKET_IN_USE -1111
}
#endif /* CONSTANTS_H_ */
