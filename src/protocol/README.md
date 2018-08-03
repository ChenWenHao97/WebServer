## 定义了头的具体信息
### 使用
包含http_parser.c（实现）和http_parser.h（定义）</br>
解析好头之后，自己必须提供７个回调函数</br>

static int MessageBegincb(http_parser * parser)//消息开始
       
static int MessageCompletecb(http_parser * parser)//消息结束
       
static int HeadersCompletecb(http_parser * parser)//接受完头
        
static int HeadersKeycb(http_parser *parser,const//键</br>
static int HeaderValuecb(http_parser * parser,const char * p,size_t length)//值
        
static int Bodycb(http_parser *parser,const char *p,size_t length)//body的回调函数
        
static int Urlcb(http_parser * parser,const char *p,size_t length)//url
   
