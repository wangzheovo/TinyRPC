
#include<stdio.h>
#include<assert.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string>
#include<memory>
#include<unistd.h>
#include<google/protobuf/service.h>

#include "RPC/net/tcp/tcp_client.h"
#include "RPC/common/log.h"
#include "RPC/common/config.h"
#include "RPC/net/tcp/net_addr.h"
#include "RPC/net/coder/string_coder.h"
#include "RPC/net/coder/abstract_protocol.h"
#include "RPC/net/coder/tinypb_coder.h"
#include "RPC/net/coder/tinypb_protocol.h"
#include "RPC/net/tcp/tcp_server.h"
#include "RPC/net/rpc/rpc_dispatcher.h"
#include "order.pb.h"

class OrderImpl : public Order{
    public:
        void makeOrder(google::protobuf::RpcController* controller,
                       const ::makeOrderRequest* request,
                       ::makeOrderResponse* response,
                       ::google::protobuf::Closure* done){
                        if(request->price() < 10){
                            response->set_ret_code(-1);
                            response->set_res_info("short balance");
                            return ;
                        }
                        response->set_order_id("20240106");
                       }
};

void test_tcp_server(){
	RPC::IPNetAddr::s_ptr addr = std::make_shared<RPC::IPNetAddr>("127.0.0.1",12345);
    DEBUGLOG("create addr %s", addr->toString().c_str());
	RPC::TcpServer tcp_server(addr);

	tcp_server.start();

}

int main(){
	RPC::Config::SetGlobalConfig("../conf/Tinyxml.xml");
	RPC::Logger::InitGlobalLogger();
	
    std::shared_ptr<OrderImpl> service = std::make_shared<OrderImpl>();
    RPC::RpcDispatcher::GetRpcDispatcher()->registerService(service);

    test_tcp_server();

	return 0;
}