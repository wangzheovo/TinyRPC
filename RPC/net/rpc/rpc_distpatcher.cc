#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/message.h>

#include  "RPC/net/rpc/rpc_dispatcher.h"
#include "RPC/net/rpc/rpc_controller.h"
#include "RPC/net/coder/tinypb_protocol.h"
#include "RPC/net/tcp/net_addr.h"
#include "RPC/net/tcp/tcp_connection.h"
#include "RPC/common/log.h"
#include "RPC/common/error_code.h"


namespace RPC{

    static RpcDispatcher* g_rpc_dispatcher = NULL;

    RpcDispatcher* RpcDispatcher::GetRpcDispatcher(){
        if(g_rpc_dispatcher == NULL){
            g_rpc_dispatcher = new RpcDispatcher();
        }
        return g_rpc_dispatcher;
    }

    void RpcDispatcher::dispatch(AbstractProtocol::s_ptr request, AbstractProtocol::s_ptr response, TcpConnection* connection){
        
        std::shared_ptr<TinyPBProtocol> req_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(request);
        std::shared_ptr<TinyPBProtocol> rsp_protocol = std::dynamic_pointer_cast<TinyPBProtocol>(response);
        
        std::string method_full_name = req_protocol->m_method_name;
        std::string service_name = "";
        std::string method_name = "";
        if(!parseServiceFullName(method_full_name, service_name, method_name)){
            setTinyPBError(rsp_protocol, ERROR_PARSE_SERVICE_NAME, "parse service name error");
            return ;
        }
        auto it = m_service_map.find(service_name);
        if(it == m_service_map.end()){
            ERRORLOG("%s | service name [%s] not found", req_protocol->m_msg_id.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_SERVICE_NOT_FOUND, "service not found");
            return ;
        }
        service_s_ptr service = (*it).second;
        const google::protobuf::MethodDescriptor* method =  service->GetDescriptor()->FindMethodByName(method_name);
        if(method == NULL){
            ERRORLOG("%s | method name [%s] not found in service [%s]", req_protocol->m_msg_id.c_str(), method_name.c_str(), service_name.c_str());
            setTinyPBError(rsp_protocol, ERROR_METHOD_NOT_FOUND, "method not found");
            return ;
        }
        google::protobuf::Message* req_msg = service->GetRequestPrototype(method).New();
        
        // 反序列化，将pb_data 反序列化为req_msg
        if(!req_msg->ParseFromString(req_protocol->m_pb_data)){
            ERRORLOG("%s | deserilize error", req_protocol->m_msg_id.c_str());
            setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");

            if(req_msg != NULL){
                delete req_msg;
                req_msg = NULL;
            }

            return ;
        }

        INFOLOG("[%s] | get rpc request [%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str());

        google::protobuf::Message* rsp_msg = service->GetResponsePrototype(method).New();

        RpcController rpcController;
        IPNetAddr::s_ptr local_addr = std::make_shared<IPNetAddr>("127.0.0.1",1234);
        rpcController.SetLocalAddr(connection->getLocalAddr());
        rpcController.SetPeerAddr(connection->getPeerAddr());
        rpcController.SetMsgId(req_protocol->m_msg_id);

        service->CallMethod(method, &rpcController, req_msg,  rsp_msg, NULL);

        if(!rsp_msg->SerializeToString(&rsp_protocol->m_pb_data)){
            ERRORLOG("%s | serilize error, origin message [%s]", req_protocol->m_msg_id.c_str(), rsp_msg->ShortDebugString().c_str());
            setTinyPBError(rsp_protocol, ERROR_FAILED_DESERIALIZE, "deserilize error");

            if(req_msg != NULL){
                delete req_msg;
                req_msg = NULL;
            }
            if(rsp_msg != NULL){
                delete rsp_msg;
                rsp_msg = NULL;
            }
            return ;
        }
        rsp_protocol->m_msg_id = req_protocol->m_msg_id;
        rsp_protocol->m_method_name = req_protocol->m_method_name;
        rsp_protocol->m_err_code = 0;

        INFOLOG("%s | dispatch success, request [%s], responsse[%s]", req_protocol->m_msg_id.c_str(), req_msg->ShortDebugString().c_str(), rsp_msg->ShortDebugString().c_str());
        delete req_msg;
        req_msg = NULL;
        delete rsp_msg;
        rsp_msg = NULL;
    }

    void RpcDispatcher::registerService(service_s_ptr service){
        std::string service_name = service->GetDescriptor()->full_name();
        m_service_map[service_name] = service;
    }

    bool RpcDispatcher::parseServiceFullName(const std::string& full_name, std::string &service_name, std::string& method_name){
        if(full_name.empty()){
            ERRORLOG("the full name is empty");
            return false;
        }
        size_t idx = full_name.find_first_of(".");
        if(idx == full_name.npos){
            ERRORLOG("not find \".\" in full name [%s]", full_name.c_str());
            return false;
        }
        service_name = full_name.substr(0, idx);
        method_name = full_name.substr(idx + 1, full_name.length() - idx - 1);

        INFOLOG("parse serviice_name [%s] and method_name [%s] from full_name [%s]", service_name.c_str(), method_name.c_str(), full_name.c_str());
        return true;
    }

    void RpcDispatcher::setTinyPBError(std::shared_ptr<TinyPBProtocol> msg, int32_t err_code, const std::string err_info){
        msg->m_err_code = err_code;
        msg->m_err_info = err_info;
        msg->m_err_info_len = err_info.length();
    }
}