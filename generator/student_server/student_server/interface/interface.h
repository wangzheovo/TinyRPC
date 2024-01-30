/****************************************************
 *
 * ****     ***     ****    *   *    *****    *****
 * *  *    *   *   *        ****     ***        *
 * *   *    ***     ****    *   *    *****      *
 *
 * interface.h
 * 2024-01-10 22:42:27
 * Generated by rpc framework rpc_generator.py
 * File will not generate while exist
 * Allow editing
****************************************************/

#ifndef STUDENT_SERVER_INTERFACE_H
#define STUDENT_SERVER_INTERFACE_H 

#include <rpc/net/rpc/rpc_closure.h>
#include <rpc/net/rpc/rpc_controller.h>
#include <rpc/net/rpc/rpc_interface.h>
#include <google/protobuf/message.h>

namespace student_server {

/*
 * Rpc Interface Base Class
 * All interface should extend this abstract class
*/

class Interface : public rpc::RpcInterface {
 public:

  Interface(const google::protobuf::Message* req, google::protobuf::Message* rsp, rpc::RpcClosure* done, rpc::RpcController* controller);

  virtual ~Interface() = 0;

};


}


#endif