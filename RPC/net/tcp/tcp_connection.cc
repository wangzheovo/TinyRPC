#include <unistd.h>

#include "RPC/net/tcp/tcp_connection.h"
#include "RPC/net/fd_event_group.h"
#include "RPC/common/log.h"
#include "RPC/net/coder/string_coder.h"
#include "RPC/net/coder/tinypb_coder.h"
#include "RPC/net/coder/tinypb_protocol.h"


namespace RPC{
    
    TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, NetAddr::s_ptr local_addr, TcpConnectionType type)
    : m_event_loop(event_loop), m_peer_addr(peer_addr), m_local_addr(local_addr), m_state(NotConnected), m_fd(fd), m_connection_type(type){
        m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
        m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

        m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
        m_fd_event->setNonBlock();
        

        m_coder = new TinyPBCoder();

        if(m_connection_type == TcpConnectionByServer){
            listenRead();
            // m_dispatcher = std::make_shared<RpcDispatcher>();
        }

    }

    TcpConnection::~TcpConnection(){
        DEBUGLOG("~TcpConnection");

        if(m_coder){
            delete m_coder;
            m_coder = nullptr;
        }
    }

    void TcpConnection::onRead(){
        // 1. 从Socke缓冲区，调用系统的read函数读取字节到in_buffer里

        if(m_state != Connected){
            ERRORLOG("onRead error, client has already disconnected, addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
            return ;
        }
        bool is_read_all = false;
        bool is_close = false;
        while(!is_read_all){
            if(m_in_buffer->writeAble() == 0){
                m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
            }
            int read_count = m_in_buffer->writeAble();
            int write_index = m_in_buffer->writeIndex();

            int rt = read(m_fd, &(m_in_buffer->m_buffer[write_index]), read_count);
            DEBUGLOG("success read %d bytes from addr [%s], clientfd [%d]", rt, m_peer_addr->toString().c_str(), m_fd);
            if(rt > 0){
                m_in_buffer->moveWriteIndex(rt);
                if(rt == read_count){
                    // 可能还没读完
                    continue;
                }else if(rt < read_count){
                    is_read_all = true;
                    break;
                }
            }else if(rt == 0){
                is_close = true;
                break;
            }else if(rt == -1 && errno == EAGAIN){
                is_read_all = true;
                break;
            }
        }
        if(is_close){
            DEBUGLOG("peer closed, peer addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
            clear();
            return ;
        }
        if(!is_read_all){
            ERRORLOG("not read all data");
        }
        TcpConnection::excute();
    }

    void TcpConnection::excute(){

        if(m_connection_type == TcpConnectionByServer){
            // 将RPC请求执行业务逻辑，获取RPC响应，再把RPC响应发送回去

            std::vector<AbstractProtocol::s_ptr> result;
            std::vector<AbstractProtocol::s_ptr> replay_messages;
            m_coder->decode(result, m_in_buffer);
            for(size_t i = 0; i < result.size(); i++){
                // 1. 针对每一个请求，调用RPC发方法，获取响应message
                // 2. 将响应message 放入到发送缓冲区中，监听可写时间回包
                INFOLOG("success get request [%s] from client [%s]",result[i]->m_msg_id.c_str(), m_peer_addr->toString().c_str());
                std::shared_ptr<TinyPBProtocol> message = std::make_shared<TinyPBProtocol>();
                // message->m_pb_data = "Hello !!! This is RPC test data";
                message->m_msg_id = result[i]->m_msg_id;
                RpcDispatcher::GetRpcDispatcher()->dispatch(result[i], message, this);
                replay_messages.emplace_back(message);
            }
            m_coder->encode(replay_messages, m_out_buffer);
            listenWrite();
  
        }else if(m_connection_type == TcpConnectionByClient){
            std::vector<AbstractProtocol::s_ptr> result;
            m_coder->decode(result,m_in_buffer);
            DEBUGLOG("result's size is [%d]", result.size());
            for(size_t i=0;i<result.size();i++){
                std::string msg_id = result[i]->m_msg_id;
                auto it =m_read_dones.find(msg_id);
                if(it != m_read_dones.end()){
                    it->second(result[i]);
                }
            }
        }

        
        
    }

    void TcpConnection::onWrite(){
        // 将当前out_buffer 里的数据全部发送给client
        if(m_state != Connected){
            ERRORLOG("onWrite error, client has already disconnected, addr [%s], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
            return ;
        }

        if(m_connection_type == TcpConnectionByClient){
            // 如果是客户端的连接请求
            // 1. 将message encode 得到字节流
            // 2. 需要将字节流写入到buffer里，然后全部发送
            std::vector<AbstractProtocol::s_ptr> messages;
            for(size_t i=0; i<m_write_dones.size();i++){
                messages.emplace_back(m_write_dones[i].first);
            }
            m_coder->encode(messages, m_out_buffer);

        }

        bool is_write_all = false;

        while(true){
            int write_size = m_out_buffer->readAble();
            if(write_size == 0){
                DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }

            int read_index = m_out_buffer->readIndex();
            int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);
            if(rt >= 0){
                DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
                is_write_all = true;
                break;
            }else if(rt == -1 && errno == EAGAIN){
                // 发送缓冲区已满，不能继续发送
                // 这种情况就等下次fd可写时再次发送数据即可
                ERRORLOG("write data error, rt == -1 && errno == EAGAIN");
                break;
            }
        }
        if(is_write_all){
            m_fd_event->cancle(FdEvent::OUT_EVENT);
            m_event_loop->addEpollEvent(m_fd_event);
        }

        if(m_connection_type == TcpConnectionByClient){
            for(size_t i=0; i<m_write_dones.size(); i++){
                m_write_dones[i].second(m_write_dones[i].first);
            }
            m_write_dones.clear();
        }
        
    }

    void TcpConnection::setState(const TcpState state){
        m_state = state;
    }

    TcpState TcpConnection::getState(){
        return m_state;
    }

    void TcpConnection::clear(){
        // 服务器处理关闭连接后的清理工作
        if(m_state == Closed){
            return ;
        }
        m_fd_event->cancle(FdEvent::IN_EVENT);
        m_fd_event->cancle(FdEvent::OUT_EVENT);

        m_event_loop->deleteEpollEvent(m_fd_event);

        m_state = Closed;
    }

    void TcpConnection::shutdown(){
        if(m_state == Closed || m_state == NotConnected){
            return ;
        }
        // 处于半关闭
        m_state = HalfClosing;

        // 调用shutdown关闭读写，意味着服务器不在对这个fd进行读写操作
        // 发送FIN报文，触发四次挥手的第一个阶段
        // 当fd 发生可读事件，但是可读的数据为0,即对方也发送了一个FIN
        ::shutdown(m_fd,SHUT_RDWR);
    }

    void TcpConnection::setConnectionType(TcpConnectionType type){
        m_connection_type = type;
    }

    void TcpConnection::listenWrite(){
        m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite,this));
        m_event_loop->addEpollEvent(m_fd_event);
    }
    void TcpConnection::listenRead(){
        m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead,this));
        m_event_loop->addEpollEvent(m_fd_event);
    }

    void TcpConnection::pushSendMessage(AbstractProtocol::s_ptr message,std::function<void(AbstractProtocol::s_ptr)> done){
        m_write_dones.emplace_back(std::make_pair(message, done));
    }

    void TcpConnection::pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done){
        m_read_dones.insert(std::make_pair(req_id,done));
    }

    NetAddr::s_ptr TcpConnection::getLocalAddr(){
        return m_local_addr;
    }

    NetAddr::s_ptr TcpConnection::getPeerAddr(){
        return m_peer_addr;
    }
}
