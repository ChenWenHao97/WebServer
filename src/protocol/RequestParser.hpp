#include<iostream>
#include"../utils/http_parser.c"
#include"../utils/hhtp_parser.h"

class HttpRequestParser{
    private:
        http_parser parser;
        HttpRequest result;

    public:
        int MeesageBegincb(http_parser * parser)//消息开始
        {
            return 0;
        }
        int MessageCompletecb(http_parser * parser)//消息结束
        {
            return 0;
        }
        int HeadersCompletecb(http_parser * parser)//接受完头
        {
            HttpRequest * req = reinterpret_cast<HttpRequest *>(parser->data);
            //强制类型转换，没有位数损失
            req->SetMethod(HttpMethod(parser->method));//传一个HttpMethod类型
            if(parser->http_major == 1)//设置版本
            {
                if(parser->http_minor == 1)
                {
                    req->SetVersion(HttpVersion::HTTP_1_1);
                }
                else if(parser->htto_minor == 0)
                {
                    req->SetVersion(HttpVersion::HTTP_1_0);
                }
            }
            else  
            {
                req->SetStatus(HttpStatus::NOT_SUPPORT);
            }
            auto str = req->GetValue("Connection");
            if(str!="")
            {
                if(req->GetVersion()==HttpVersion::HTTP_1_1)
                    req->SetStatus(HttpStatus::KEEP_ALIVE);
            }
            return 0;
        }
        int HeadersKeycb(http_parser *parser,const char * p,size_t length)
        {
            HttpRequest *req = reinterpret_cast<HttpRequest*>(parser->data);
            req->SetKeyLength({p,length});//因为用的map需要将键值对同时放入，所以需要先记录一次键
            //c语言用二维数组还是很方便的
            //加length是为了防止读取字符串读取一整行
            return 0;
        }
        int HeaderValuecb(htto_parser * parser,const char * p,size_t length)
        {
            HttpRequest * req = reinterpret_cast<HttpRequest*>(parser->data);
            auto key = req->GetKeyLength();
            req->Setkeyvalue(string(key.data(),key.length()),string(p,length));
            return 0;
        }
        


};