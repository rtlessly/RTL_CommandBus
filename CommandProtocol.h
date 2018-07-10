/*******************************************************************************
 CommandProtocol.h

 Defines constants and messages for the robot command protocol
*******************************************************************************/
#ifndef _CommandProtocol_h_
#define _CommandProtocol_h_

#include <inttypes.h>


//****************************************************************************
// Common command codes
//****************************************************************************
const byte CMD_NONE              = 0x00;    // No command
const byte CMD_QUERY_ID          = 0x01;    // Queries the ID of a slave device
const byte CMD_QUERY_RESPONSE    = 0x02;    // Queries if a response is ready from a slave device
const byte CMD_RESET_DEVICE      = 0x03;    // Commands a slave device to reset - may be broadcast
const byte CMD_MASTER_ADDR       = 0x04;    // Informs a slave device of the master's I2C address - may be broadcast
const byte CMD_EXECUTE           = 0x05;    // Send command string to master to execute
const byte CMD_ECHO              = 0x06;    // Commands slave to echo the command data

//****************************************************************************
// Common Notification codes
//****************************************************************************
const byte NOTIFY_CMD_INVALID    = 0xFE;    // Invalid command sent


//****************************************************************************
// Common response codes
//****************************************************************************
const byte CMD_RESPONSE_OK       = 0x00;    // Indicates that a this response contains the requested data
const byte CMD_RESPONSE_DEFERRED = 0x01;    // Indicates that the response to the current command is deferred
const byte CMD_RESPONSE_NOTREADY = 0x02;    // Indicates that a response is not yet ready (in response to the CMD_QUERY_RESPONSE command)
const byte CMD_RESPONSE_BUSY     = 0x03;    // Indicates that the slave device could not process the current command because it is busy
const byte CMD_RESPONSE_ERROR    = 0x04;    // Indicates that there was some error in processing the current command
const byte CMD_RESPONSE_UNKNOWN  = 0x05;    // Indicates that the command was not recognized


//****************************************************************************
/// The base structure for a command message over the I2C bus. This can be used
/// for basic commands that do not require parameters, and as the base for derived 
/// commands that do require parameters.
///
/// NOTE: Command messages SHOULD NEVER have any virtual members! This is because
///       these structs are often overlayed on buffers and it is assumed that
///       that the field layout is EXACTLY as specified in the struct without
///       any hidden fields (i.e., virtual) interfering with that layout.
//****************************************************************************
struct CommandMessage
{
    byte Length;        // The length of the command, in bytes
    byte CommandCode;   // Command code

    CommandMessage(const byte commandCode=0) : CommandCode(commandCode) { Length = sizeof(*this); };
};


struct CommandExecute : public CommandMessage
{
    byte RequestorAddress;
    char CommandLine[27];        // The command line to execute

    CommandExecute(byte requestorAddr, const char* commandLine) : CommandMessage(CMD_EXECUTE), RequestorAddress(requestorAddr) 
    { 
        Length = sizeof(*this); 
        memset(CommandLine, 0, sizeof(CommandLine));
        strncpy(CommandLine, commandLine, sizeof(CommandLine)-1); 
    }
};


struct CommandEcho : public CommandMessage
{
    char EchoData[27];        // The data to echo

    CommandEcho(const char* echoData) : CommandMessage(CMD_ECHO) 
    { 
        Length = sizeof(*this); 
        memset(EchoData, 0, sizeof(EchoData));
        strncpy(EchoData, echoData, sizeof(EchoData)-1); 
    }
};


//****************************************************************************
/// The Query Response Ready command request
/// 
/// This command is sent after a CMD_RESPONSE_DEFERRED response was returned
/// to see if the data for that response is now ready. Each deferred response
/// has a unique response ID that is used to identify the correct response from
/// the slave device.
///
/// Note that it is the responsibility of the requester to retain the response
/// ID, command code, and the device address of the slave device that sent the 
/// CMD_RESPONSE_DEFERRED response so that it can re-query the device at a later 
/// time for the response.
//****************************************************************************
struct CommandQueryResponseReady : public CommandMessage
{
    byte ResponseID;        // The ID of the response this request is for
    byte OriginalCommand;   // The command code of the original request that was deferred

    CommandQueryResponseReady(const byte responseID=0) : CommandMessage(CMD_QUERY_RESPONSE), ResponseID(responseID) { Length = sizeof(*this); };
};


//****************************************************************************
/// The basic structure for a command response over the I2C bus. This is used
/// primarily as the base for derived responses that contain data specific to
/// to the command response, although there may occasionally be a response (such
/// as CMD_RESPONSE_NOTREADY) that does not require data and could thus use
/// this struct directly.
///
/// NOTE: Command responses SHOULD NEVER have any virtual members! This is because
///       these structs are often overlayed on buffers and it is assumed that
///       that the field layout is EXACTLY as specified in the struct without
///       any hidden fields (i.e., virtual) interfering with that layout.
//****************************************************************************
struct CommandResponse
{
    byte Length;        // The length of the response, in bytes
    byte ResponseCode;  // Command Response code (one of CMD_RESPONSE_* values)
    byte ResponseID;    // Responder generated response ID, Used for deferred responses

    CommandResponse(const byte responseCode=CMD_RESPONSE_OK, const byte responseID=0) : ResponseCode(responseCode), ResponseID(responseID) { Length = sizeof(*this); };
};


//****************************************************************************
/// The response to the CMD_QUERY_ID command
///
/// This response is sent for the CMD_QUERY_ID to reply with the ID of the slave
/// device.
//****************************************************************************
struct CommandResponseQueryID : public CommandResponse
{
    byte ID;
    
    CommandResponseQueryID(const byte id=0) : ID(id) { Length = sizeof(*this); };
};


//****************************************************************************
/// The response deferred command response.
///
/// This response is sent for commands that take a significant amount of time
/// to complete, so they cannot be responded to in a timely manner in accordance
/// with the I2C protocol.
///
/// Note that it is the responsibility of the requester to retain the command 
/// code and the device address of the slave device so that the master can re-query 
/// the device at a later time for the response (via the CMD_QUERY_RESPONSE
/// command.
//****************************************************************************
struct CommandResponseDeferred : public CommandResponse
{
    CommandResponseDeferred(const byte responseID=0) : CommandResponse(CMD_RESPONSE_DEFERRED, responseID) { Length = sizeof(*this); };
};

#endif

