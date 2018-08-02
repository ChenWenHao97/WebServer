#include<iostream>
#include<unordered_map>
#include"../FastCGI/fcgi.h"
#include"../FastCGI/fcgi.c"
#include"../FastCGI/fastcgi.h"
using namespace std;
using byte = unsigned char;

enum class HttpMethod:int{
    DELETE  = 0,
    GET     = 1,
    HEAD    = 2,
    POST    = 3,
    PUT     = 4,
    CONNECT = 5,
    OPTIONS = 6,
    TRACE   = 7
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

unordered_map<HttpVersion,string> version_map {
    {HttpVersion::HTTP_1_0, "HTTP/1.0"},
    {HttpVersion::HTTP_1_1, "HTTP/1.1"}
};

unordered_map<HttpMethod,string> method_map {
    {HttpMethod::GET,"GET"},
    {HttpMethod::POST,"POST"} ,
    {HttpMethod::HEAD,"HEAD"} ,
    {HttpMethod::DELETE,"DELETE"},
    {HttpMethod::PUT,"PUT"},
    {HttpMethod::OPTIONS,"OPTIONS"},
    {HttpMethod::TRACE,"TRACE"},
    {HttpMethod::CONNECT,"CONNECT"}
};

class HttpRequest{
    private:
        HttpMethod method;
        HttpVersion version;
        string url;
        HttpStatus status;
        unordered_map<string,string> key_value;
        shared_ptr<byte> body;//body有不需要的内容，只需要转发给CGI
        int body_length;
        string KeyLength;

    public:
        void SetKeyLength(string kl)
        {
            this->KeyLength=kl;
        }
        string GetKeyLength()
        {
            return this->KeyLength;
        }
        void SetMethod(HttpMethod m)
        {
            this->method = m;
        }
        void SetVersion(HttpVersion v)
        {
            this->version = v;
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
            this->body.reset(b, default_delete<byte []>());
            this->body_length = size;
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
            if(this->key_value.find(key)==this->key_value.end())
            //map没有.end（）方法!!!!!!!!!!!!!!!!!!!!!
                return "";//空
            return key_value[key];
        }
        byte * GetBody()
        {
            return body.get();//如何返回
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
        unordered_map<string,string> key_value;
        shared_ptr<byte> body;//body有不需要的内容，只需要转发给CGI
        int body_length;
        // body_stream;
        // body_fd;
    public:
        HttpRespnse(HttpRequest &req) 
        {
            this->version = req.GetVersion();
            this->status = req.GetStatus(); 
        }

        // void cgi(int connfd)
        // {

        //     skt.send("HTTP/1.1 ");
        //     skt.send(status TOSTRING..);
        //     skt.send(" DESC\r\n");
        //     for (auto &i: key_value) {
        //         skt.send(i.first);
        //         skt.send(": ");
        //         skt.send(i.second "\r\n");
        //     }
        //     skt.send('\r\n');
        //     sendfile();
        //     skt.send(body);
        // }

        void Setkeyvalue(string key,string value)
        {
            this->key_value.emplace(key,value);
        }
        void SetBody(byte * b,int size)
        {
            this->body.reset(b, default_delete<byte []>());
            this->body_length = size;
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
            if(this->key_value.find(key)==this->key_value.end())
                return "";//空
            return key_value[key];
        }
        byte * GetBody()
        {
            return body.get();
        }
        int GetBodySize()
        {
            return body_length;
        }
};