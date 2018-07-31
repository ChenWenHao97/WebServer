#include<iostream>
#include<unordered_map>
#include<string_view>
using namespace std;
using byte = unsigned char;
enum class HttpMethod:int{
    GET = 0,
    POST ,
    HEAD ,//只有头，没内容
    DELETE,
    PUT,
    OPTIONS,//获取URL的方法
    TRACE,
    CONNECT,//透明tcp/ip通道
};
enum class HttpVersion:int{
    HTTP_1_0 = 0,
    HTTP_1_1
};
enum class HttpStatus:int{
    OK = 0,
    BAD_REQUEST = 400,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    INTERNAL_ERROR = 500,
    NOT_SUPPORT = 501,
    KEEP_ALIVE = 502
};
class HttpRequest{
    private:
        HttpMethod method;
        HttpVersion version;
        string url;
        HttpStatus status;
        unorder_map<string,string> key_value;
        shared_ptr<byte> body;//body有不需要的内容，只需要转发给CGI
        int body_length;
        string_view  KeyLength;

    public:
        void SetKeyLength(KeyLength kl)
        {
            this->KeyLength=kl;
        }
        string_view GetKeyLength()
        {
            return this->KeyLength;
        }
        void SetMethod(Method m)
        {
            this->method = m;
        }
        void SetVersion(HttpVersion v)
        {
            this-<version = v;
        }
        void SetUrl(string url)
        {
            this->url = url;
        }
        void Setkeyvalue(string key,string value)
        {
            this->key_value.emplace(key,value);
        }
        void SetBody(byte * b,int size)
        {
            this->body = b;
            this->body_size = size;
        }
         void SetStatus(HttpStatus s)
        {
            this->status = s;
        }
        HttpStatus GetStatus()
        {
            return this->status;
        }
        HttpMethod GetMethod()
        {
            return this->method;
        }
        HttpVersion GetVersion()
        {
            return this->version;
        }
        string Geturl()
        {
            return this->url;
        }
        string GetValue(string key)
        {
            if(this->key_value.find(key)==key.end())
                return "";//空
            return key_value[key];
        }
        byte * GetBody()
        {
            return body;
        }
        int GetBodySize()
        {
            return body_length;
        }
};

class HttpRespnse{
    private:
        HttpVersion version;
        HttpStatus status;
        unorder_map<string,string> key_value;
        shared_ptr<byte> body;//body有不需要的内容，只需要转发给CGI
        int body_length;
        // body_stream;
        // body_fd;
    public:
        HttpRespnse(HttpRequest &req) 
        {
            this->version = req.GetVersion();
            // openfd...
            // set status code
            // fd
            // key_Value

        }

        void sendTo(socket skt)
        {
            skt.send("HTTP/1.1 ");
            skt.send(status TOSTRING..);
            skt.send(" DESC\r\n");
            for (auto &i: key_value) {
                skt.send(i.first);
                skt.send(": ");
                skt.send(i.second "\r\n");
            }
            skt.send('\r\n');
            sendfile();
            skt.send(body);
        }

        void Setkeyvalue(string key,string value)
        {
            this->key_value.emplace(key,value);
        }
        void SetBody(byte * b,int size)
        {
            this->body = b;
            this->body_size = size;
        }
        void SetVersion(HttpVersion v)
        {
            this->version = v;
        }
        void SetStatus(HttpStatus s)
        {
            this->status = s;
        }
        HttpVersion GetVersion()
        {
            return this->version;
        }
        HttpStatus GetStatus()
        {
            return this->status;
        }
         string GetValue(string key)
        {
            if(this->key_value.find(key)==key.end())
                return "";//空
            return key_value[key];
        }
        byte * GetBody()
        {
            return body;
        }
        int GetBodySize()
        {
            return body_length;
        }
};