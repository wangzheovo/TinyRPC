
#include<tinyxml/tinyxml.h>
#include "config.h"


#define READ_XML_NODE(name,parent)											\
TiXmlElement* name##_node = parent->FirstChildElement(#name);				\
if(!name##_node){															\
	printf("start RPC server error,faild to read node [%s]\n",#name);       \
	exit(0);																\
}																			\


#define READ_STRING_FROM_XML_NODE(name,parent)								\
TiXmlElement* name##_node = parent->FirstChildElement(#name);				\
if(!name##_node || !name##_node->GetText()){								\
	printf("start RPC server error,faild to read config file %s",#name);	\
    exit(0);																\
}																			\
std::string name##_str = std::string(name##_node->GetText());				\


namespace RPC{
	
	static Config* g_config = NULL;
	Config* Config::GetGlobalConfig(){
		return g_config;
	}
	void Config::SetGlobalConfig(const char* xmlfile){
		if(g_config == NULL){
			g_config = new Config(xmlfile);
		}
	}
	
	RPC::Config::Config(const char* xmlfile){
		TiXmlDocument* xml_document = new TiXmlDocument();
		bool rt = xml_document->LoadFile(xmlfile);
		if(!rt){
			printf("start RPC server error,faild to read config file %s",xmlfile);
			exit(0);
		}
		READ_XML_NODE(root,xml_document);
		READ_XML_NODE(log,root_node);

		READ_STRING_FROM_XML_NODE(log_level,log_node);
		
		m_log_level = log_level_str;

	}

}


