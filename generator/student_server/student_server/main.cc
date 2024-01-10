/***************************************************************
* Wang ！
* ${FILENAME}
* Generated by RPC framework rpc_generator.py
***************
* Do not edit *
***************
***************************************************************/
#include<google/protobuf/service.h>

#include "RPC/common/log.h"
#include "RPC/common/config.h"
#include "RPC/net/tcp/net_addr.h"
#include "RPC/net/tcp/tcp_server.h"
#include "RPC/net/rpc/rpc_dispatcher.h"

int main(int argc, char* argv[]){
    if(argc != 2){
        printf("Start student_server error, the number of parameter is wrong");
        printf("Please enter like this: \n");
        printf("./student_server ../conf/Tinyxml.xml\n");
        return 0;
    }
	RPC::Config::SetGlobalConfig("argv[1]");
	RPC::Logger::InitGlobalLogger();

    std::shared_ptr<${SERVICE_NAMME}> service = std::make_shared<${SERVICE_NAMME}>();
    RPC::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    RPC::IPNetAddr::s_ptr addr = std::make_shared<RPC::IPNetAddr>("127.0.0.1",RPC::Config::GetGlobalConfig()->m_port);

    DEBUGLOG("create addr %s", addr->toString().c_str());
	RPC::TcpServer tcp_server(addr);

	tcp_server.start();
	return 0;
}