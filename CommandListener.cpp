/*******************************************************************************
 CommandListener.cpp

 Implementation file for the I2C command listener base class.
*******************************************************************************/
#define DEBUG 0

#include <Arduino.h>
#include <RTL_Debug.h>
#include "CommandListener.h"


DEFINE_CLASSNAME(CommandListener);

void CommandListener::Begin()
{
    CommandPending  = false;
    ResponsePending = false;
    OnBegin();
}


//****************************************************************************
// EventSource Poll method override to handle pending commands
//****************************************************************************
void CommandListener::Poll()
{
    TRACE(Logger(_classname_, __func__, this) << endl);

    if (CommandPending)
    {
        OnCommand((const CommandMessage*)CommandBuffer);
        CommandPending = false;
    }
    
    EventSource::Poll();
}


//****************************************************************************
// IEventListener interface
//****************************************************************************
void CommandListener::OnEvent(const Event* pEvent)
{
    switch(pEvent->EventID) 
    {
        // case ResponseEvent:
            // PostDeferredResponse((CommandResponse*)pEvent->Data.Pointer);
        // break;

        default:
        break;
    }
}


void CommandListener::OnCommand(const CommandMessage* pCommand)
{
    DefaultCommandHandler(pCommand);
}


void CommandListener::DefaultCommandHandler(const CommandMessage* pCommand)
{
    switch(pCommand->CommandCode)
    {
        case CMD_QUERY_ID:
        {
            auto response = CommandResponseQueryID(_myDeviceID); 
            
            PostResponse(&response);
        }
        break;

        // case CMD_QUERY_RESPONSE:
        // {
            // HandleQueryResponseReady((CommandQueryResponseReady&)*pCommand);
        // }
        // break;
    }
}


// //****************************************************************************
// // Called when a command request is received on the I2C interface
// // NOTE: This method is called from an interrupt handler, so it should do
// //       as little as possible and get out as quickly as possible.
// //****************************************************************************
// void CommandListener::OnCommandReceived(const CommandMessage* pCommand)
// {
    // // If there is already a command pending then send the BUSY response for 
    // // this command (if the command expects a response).
    // // TODO: consider changing to queue the incoming commands
    // if (CommandPending)
    // {
        // auto responseBusy = CommandResponse(CMD_RESPONSE_BUSY);

        // if (IsResponseExpected(pCommand)) PostResponse(&responseBusy);
        // return;
    // }
    
    // CommandPending = true;
    // memcpy(CommandBuffer, (byte*)pCommand, pCommand->Length);
// }


// //****************************************************************************
// // Called when a command request is received on the I2C interface
// // NOTE: This method is called from an interrupt handler, so it should do
// //       as little as possible and get out as quickly as possible.
// //****************************************************************************
// void CommandListener::OnCommandReceived(TwoWire& twi, int messageLength)
// {
    // // If there is already a command pending then flush the incoming message 
    // // TODO: consider changing to queue the incoming commands
    // if (CommandPending)
    // {
        // // flush I2C receive buffer
        // while (twi.available()) twi.read();

        // return;
    // }

    // CommandPending = true;

    // if (I2C_Read(twi, (byte*)CommandBuffer, messageLength) == 0) CommandPending = false;
// }


//****************************************************************************
// Send an I2C response
// NIOTE: This method is called from an interrupt handler so keep it short and simple!
//****************************************************************************
void CommandListener::SendResponse(TwoWire& twi)
{
    volatile auto response = CommandResponse(CMD_RESPONSE_UNKNOWN);

    twi.write((byte*)&response, sizeof(response));
}


//****************************************************************************
// Called when a response is requested on the I2C interface
// NOTE: This method is called from an interrupt handler, so it should do
//       as little as possible and get out as quickly as possible.
//****************************************************************************
const CommandResponse* CommandListener::GetResponse()
{
    static auto responseNotReady = CommandResponse(CMD_RESPONSE_NOTREADY);
    auto pResponse = (CommandResponse*)&responseNotReady;   

    if (ResponsePending) 
    { 
        ResponsePending = false;
        pResponse = (CommandResponse*)ResponseBuffer;
    }
    
    return pResponse;
}


// void CommandListener::HandleCommand(const CommandMessage* pCommand)
// {
    // TRACE(Logger(_classname_, __func__, this) << ToHex((byte*)pCommand, pCommand->Length) << endl);

    // switch(pCommand->CommandCode)
    // {
        // case CMD_QUERY_ID:
        // {
            // auto response = CommandResponseQueryID(_myDeviceID); 
            
            // PostResponse(&response);
        // }
        // break;

        // case CMD_QUERY_RESPONSE:
        // {
            // HandleQueryResponseReady((CommandQueryResponseReady&)*pCommand);
        // }
        // break;
    
        // default:
            // OnCommand(pCommand);
        // break;
    // }
// }


// void CommandListener::HandleQueryResponseReady(const CommandQueryResponseReady& command)
// {
    // auto responseID = command.ResponseID;

    // for (auto i = 0; i < DEFERRED_RESPONSE_LIST_SIZE; i++)
    // {
        // auto responseItem = _deferredResponseList[i];
        
        // if (responseItem.InUse)
        // {
            // auto pResponse = responseItem.pResponse;
        
            // if (pResponse->ResponseID == responseID)
            // {
                // PostResponse(pResponse);
                // responseItem.InUse = false;
                // break;
            // }
        // }
    // }
// }


void CommandListener::PostResponse(CommandResponse* pResponse)
{
    if (pResponse != NULL)
    {
        noInterrupts();
        memcpy(ResponseBuffer, (byte*)pResponse, pResponse->Length);
        ResponsePending = true;
        interrupts();
        
        // Serial << "PostResponse:";
        // for (auto i = 0; i < pResponse->Length; i++)
        //     Serial <<  " " << _HEX(ResponseBuffer[i]);
        // Serial << endl;
    }
}


// void CommandListener::PostDeferredResponse(CommandResponse* pResponse)
// {
    // if (pResponse == NULL) return;

    // for (auto i = 0; i < DEFERRED_RESPONSE_LIST_SIZE; i++)
    // {
        // auto responseItem = _deferredResponseList[i];
        
        // if (!responseItem.InUse)
        // {
            // noInterrupts();
            // responseItem.InUse = true;
            // responseItem.pResponse = pResponse;
            // interrupts();
            // break;
        // }
    // }
// }


