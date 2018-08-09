#include <iostream>
#include "http_parser.h"
#include <algorithm>
using namespace std;
class HttpRequestParser
{
  private:
    http_parser parser;
    HttpRequest result;
    http_parser_settings settings;
    byte *buf;

    static int MessageBegincb(http_parser *parser) //消息开始
    {
        return 0;
    }
    static int MessageCompletecb(http_parser *parser) //消息结束
    {
        return 0;
    }
    static int HeadersCompletecb(http_parser *parser) //接受完头
    {
        HttpRequest *req = reinterpret_cast<HttpRequest *>(parser->data);
        //强制类型转换，没有位数损失
        req->SetMethod(HttpMethod(parser->method)); //传一个HttpMethod类型
        if (parser->http_major == 1)                //设置版本
        {
            if (parser->http_minor == 1)
            {
                req->SetVersion(HttpVersion::HTTP_1_1);
            }
            else if (parser->http_minor == 0)
            {
                req->SetVersion(HttpVersion::HTTP_1_0);
            }
        }
        else
        {
            req->SetStatus(HttpStatus::NOT_SUPPORT);
        }
        auto str = req->GetValue("Connection");
        if (str != "")
        {
            if (req->GetVersion() == HttpVersion::HTTP_1_1)
                req->SetStatus(HttpStatus::KEEP_ALIVE);
        }
        return 0;
    }
    static int HeadersKeycb(http_parser *parser, const char *p, size_t length)
    {
        HttpRequest *req = reinterpret_cast<HttpRequest *>(parser->data);
        req->SetKeyLength({p, length}); //因为用的map需要将键值对同时放入，所以需要先记录一次键
        //c语言用二维数组还是很方便的
        //加length是为了防止读取字符串读取一整行
        return 0;
    }
    static int HeaderValuecb(http_parser *parser, const char *p, size_t length)
    {
        HttpRequest *req = reinterpret_cast<HttpRequest *>(parser->data);
        auto key = req->GetKeyLength();
        req->Setkeyvalue(string(key.data(), key.length()), string(p, length));
        return 0;
    }
    static int Bodycb(http_parser *parser, const char *p, size_t length)
    {
        HttpRequest *req = reinterpret_cast<HttpRequest *>(parser->data);
        byte *buf = new byte[length];
        copy(p, p + length, (char *)buf);
        req->SetBody(buf, length);
        return 0;
    }
    static int Urlcb(http_parser *parser, const char *p, size_t length)
    {
        HttpRequest *req = reinterpret_cast<HttpRequest *>(parser->data);
        string str = {p, length};
        req->SetUrl(str);
        return 0;
    }

  public:
    HttpRequestParser(byte *b) : buf(b)
    {
        http_parser_init(&this->parser, ::HTTP_REQUEST);
        this->parser.data = &this->result;
        this->settings.on_message_begin = MessageBegincb;
        this->settings.on_message_complete = MessageCompletecb;
        this->settings.on_headers_complete = HeadersCompletecb;
        this->settings.on_header_field = HeadersKeycb;
        this->settings.on_header_value = HeaderValuecb;
        this->settings.on_body = Bodycb;
        this->settings.on_url = Urlcb;
    }
    HttpRequest operator()()
    {
        http_parser_execute(&this->parser, &this->settings, (char *)buf, strlen((char *)buf));
        if (this->parser.http_errno != ::HPE_OK)
        {
            this->result.SetStatus(HttpStatus::BAD_REQUEST);
        }
        return move(this->result);
    }
};