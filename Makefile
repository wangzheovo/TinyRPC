##################################
# makefile
##################################

PATH_BIN = bin
PATH_LIB = lib
PATH_OBJ = obj


PATH_TinyRPC = RPC
PATH_COMM = $(PATH_TinyRPC)/common
PATH_NET = $(PATH_TinyRPC)/net
PATH_TCP = $(PATH_TinyRPC)/net/tcp
PATH_CODER = $(PATH_TinyRPC)/net/coder
PATH_RPC = $(PATH_TinyRPC)/net/rpc
PATH_HASH = $(PATH_TinyRPC)/consistent_hash




PATH_TESTCASES = testcases

# will install lib to /usr/lib/libTinyRPC.a
PATH_INSTALL_LIB_ROOT = /usr/lib

# will install all header file to /usr/include/RPC
PATH_INSTALL_INC_ROOT = /usr/include


PATH_INSTALL_INC_COMM = $(PATH_INSTALL_INC_ROOT)/$(PATH_COMM)
PATH_INSTALL_INC_NET = $(PATH_INSTALL_INC_ROOT)/$(PATH_NET)
PATH_INSTALL_INC_TCP = $(PATH_INSTALL_INC_ROOT)/$(PATH_TCP)
PATH_INSTALL_INC_CODER = $(PATH_INSTALL_INC_ROOT)/$(PATH_CODER)
PATH_INSTALL_INC_RPC = $(PATH_INSTALL_INC_ROOT)/$(PATH_RPC)
PATH_INSTALL_INC_HASH = $(PATH_INSTALL_INC_ROOT)/$(PATH_HASH)



PATH_PROTOBUF = /usr/include/google
PATH_TINYXML = /usr/include/tinyxml

# etcd path
# PATH_ETCD = /usr/local/include/etcd
# PATH_PPLX = /usr/local/include/pplx

CXX := g++

CXXFLAGS += -g -O0 -std=c++11 -Wall -Wno-deprecated -Wno-unused-but-set-variable


CXXFLAGS += -I ./ -I$(PATH_TinyRPC)	-I$(PATH_COMM) -I$(PATH_NET) -I$(PATH_TCP) -I$(PATH_CODER) -I$(PATH_RPC) -I$(PATH_HASH)


# CXXFLAGS += -letcd-cpp-api -lprotobuf  -lgrpc++ -lgrpc -lz -lcpprest -lssl -lcrypto -lboost_system


LIBS += /usr/lib/libprotobuf.a	/usr/lib/libtinyxml.a


COMM_OBJ := $(patsubst $(PATH_COMM)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_COMM)/*.cc))
NET_OBJ := $(patsubst $(PATH_NET)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_NET)/*.cc))
TCP_OBJ := $(patsubst $(PATH_TCP)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_TCP)/*.cc))
CODER_OBJ := $(patsubst $(PATH_CODER)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_CODER)/*.cc))
RPC_OBJ := $(patsubst $(PATH_RPC)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_RPC)/*.cc))
HASH_OBJ := $(patsubst $(PATH_HASH)/%.cc, $(PATH_OBJ)/%.o, $(wildcard $(PATH_HASH)/*.cc))

ALL_TESTS : $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop $(PATH_BIN)/test_tcp $(PATH_BIN)/test_client $(PATH_BIN)/test_rpc_client $(PATH_BIN)/test_rpc_server $(PATH_BIN)/test_etcd

TEST_CASE_OUT := $(PATH_BIN)/test_log $(PATH_BIN)/test_eventloop $(PATH_BIN)/test_tcp $(PATH_BIN)/test_client $(PATH_BIN)/test_rpc_client $(PATH_BIN)/test_rpc_server $(PATH_BIN)/test_etcd

LIB_OUT := $(PATH_LIB)/libTinyRPC.a

$(PATH_BIN)/test_log: $(LIB_OUT)
	$(CXX)  $(CXXFLAGS) $(PATH_TESTCASES)/test_log.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_eventloop: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_eventloop.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_tcp: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_tcp.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_client: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_client.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

$(PATH_BIN)/test_rpc_client: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_client.cc $(PATH_TESTCASES)/order.pb.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread -letcd-cpp-api -L/usr/lib -lprotobuf  -lgrpc++ -lgrpc -lz -lcpprest -lssl -lcrypto -lboost_system

$(PATH_BIN)/test_rpc_server: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_server.cc $(PATH_TESTCASES)/order.pb.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread -letcd-cpp-api -L/usr/lib -lprotobuf  -lgrpc++ -lgrpc -lz -lcpprest -lssl -lcrypto -lboost_system

$(PATH_BIN)/test_etcd: $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_etcd.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread -letcd-cpp-api -L/usr/lib -lprotobuf  -lgrpc++ -lgrpc -lz -lcpprest -lssl -lcrypto -lboost_system


# $(PATH_BIN)/test_rpc_client: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_client.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread

# $(PATH_BIN)/test_rpc_server: $(LIB_OUT)
# 	$(CXX) $(CXXFLAGS) $(PATH_TESTCASES)/test_rpc_server.cc -o $@ $(LIB_OUT) $(LIBS) -ldl -pthread



$(LIB_OUT): $(COMM_OBJ) $(NET_OBJ) $(TCP_OBJ) $(CODER_OBJ) $(RPC_OBJ) $(HASH_OBJ)
	cd $(PATH_OBJ) && ar rcv libTinyRPC.a *.o && cp libTinyRPC.a ../lib/


$(PATH_OBJ)/%.o : $(PATH_COMM)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_NET)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_TCP)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_CODER)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_RPC)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(PATH_OBJ)/%.o : $(PATH_HASH)/%.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

# print something test
# like this: make PRINT-PATH_BIN, and then will print variable PATH_BIN
PRINT-% : ; @echo $* = $($*)


# to clean 
clean :
	rm -f $(COMM_OBJ) $(NET_OBJ) $(TESTCASES) $(TEST_CASE_OUT) $(PATH_LIB)/libTinyRPC.a $(PATH_OBJ)/libTinyRPC.a $(PATH_OBJ)/*.o

# install
install:
	mkdir -p $(PATH_INSTALL_INC_COMM) $(PATH_INSTALL_INC_NET) $(PATH_INSTALL_INC_TCP) $(PATH_INSTALL_INC_CODER) $(PATH_INSTALL_INC_RPC) $(PATH_INSTALL_INC_HASH)\
		&& cp $(PATH_COMM)/*.h $(PATH_INSTALL_INC_COMM) \
		&& cp $(PATH_NET)/*.h $(PATH_INSTALL_INC_NET) \
		&& cp $(PATH_TCP)/*.h $(PATH_INSTALL_INC_TCP) \
		&& cp $(PATH_CODER)/*.h $(PATH_INSTALL_INC_CODER) \
		&& cp $(PATH_RPC)/*.h $(PATH_INSTALL_INC_RPC) \
		&& cp $(PATH_RPC)/*.h $(PATH_INSTALL_INC_HASH) \
		&& cp $(LIB_OUT) $(PATH_INSTALL_LIB_ROOT)	/
		


# uninstall
uninstall:
	rm -rf $(PATH_INSTALL_INC_ROOT)/RPC && rm -f $(PATH_INSTALL_LIB_ROOT)/libTinyRPC.a