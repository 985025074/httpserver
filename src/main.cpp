#include "std.h"
auto check_error(const char*msg,ssize_t status){
    if(status == -1){
        cout << msg << ":" << strerror(errno) << endl;
        throw;
    }
    else return status;
}
#define CHECK_CALL(funcname,...) check_error(#funcname,funcname(__VA_ARGS__))

struct socketaddr_pointer{
    struct sockaddr *_addr;
    socklen_t _len;
};
struct socketaddr_container{
    union{
    sockaddr _addr;
    sockaddr_storage _storage;
    };
    socklen_t _len = sizeof(sockaddr_storage);
};
class address_pointer{
addrinfo *addr;
public:
    address_pointer(addrinfo *addr):addr(addr){
    }
    int get_socket()const{
        int sockfd = socket(addr->ai_family,addr->ai_socktype,addr->ai_protocol);
        check_error("socket",sockfd);
        return sockfd;
    }
    socketaddr_pointer get_socketaddr(){
        return {addr->ai_addr,addr->ai_addrlen};
    }
    [[nodiscard]] bool next_addr(){
        if(addr->ai_next == NULL){
            return false;
        }
        addr = addr->ai_next;
        return true;
    }
    

};
class address_resolver{
    addrinfo *addr = NULL;
    public:
    address_resolver(const char *host, const char *service){
        auto addrinfo = getaddrinfo(host,service,NULL,&addr);
        if(addrinfo != 0){
            cout << "addrinfo error: " << gai_strerror(addrinfo) <<endl;
        }
    }
    address_pointer get_addr()const{
        return address_pointer(addr);
    }
    address_resolver(address_resolver&&other):addr(other.addr){
        other.addr = NULL;
    }
    ~address_resolver(){
        if(addr){
            freeaddrinfo(addr);
        }
    }
};

class http_request_parser{
    std::string _head;
    std::string _body;
    bool head_ok = false;
    bool body_ok = false;
    public:
    http_request_parser() =default;
    [[nodiscard]] bool need_more_data()const{
        return !(head_ok == true && body_ok == true);
    }
    std::map<std::string,std::string> _headers;
    void parse(const char *data, size_t len){
        std::string_view chunk(data,len);
        if(!head_ok){
            auto pos = chunk.find("\r\n\r\n");
            if(pos == std::string_view::npos){
                _head.append(chunk);
            }
            else{
                _head.append(chunk.substr(0,pos));
                head_ok = true;
                // parse_head();
                _body.append(chunk.substr(pos+4));
                //假设全部完毕
                body_ok = true;
                cout << "head:"<<_head<<endl;
                cout << "body:"<<_body<<endl;
                //这里需要修改
            }
        }
        else{
            _body.append(chunk);
        }
    }
    void process_char(std::string& c){
        std::transform(c.begin(),c.end(),c.begin(),[](char d)->char {
            if(d>='A'&&d<='Z'){
                return d-'A'+'a';
            }
            return d;
        }
      );
    }
    void parse_head(){
        std::istringstream iss(_head.substr(0,_head.find("\r\n")));//先处理首行
        iss >> _headers["method"] >> _headers["url"] >> _headers["version"];

        cout << "method:"<<_headers["method"]<<endl;
        cout << "url:"<<_headers["url"]<<endl;
        cout << "version:"<<_headers["version"]<<endl;
        size_t start_pos = _head.find("\r\n");
        size_t end_pos = _head.find("\r\n",pos+2);
        size_t finish_pos = _head.find("\r\n\r\n");
        while(end_pos != std::string::npos){
            std::istringstream line_stream(line.substr(start_pos+2,end_pos-start_pos-2));
            std::string key,value,empty;
            line_stream >> key >>empty>>value;
            process_char(key);
            process_char(value);
            _headers[key] = value;
        }
    }
    void print_headers(){
        cout << "headers:"<<endl;
        for(auto &p:_headers){
            cout << p.first << ":" << p.second << endl;
        }
    }
    
};
std::vector <std::thread>pool;
int func(int argc, char const *argv[]){
   
    // 设置本地化环境
    setlocale(LC_ALL,"zh_CN.UTF-8");
    cout <<"port"<<std::string_view(argv[1],strlen(argv[1]))<<endl;
    address_resolver resolver("localhost",argv[1]);
    auto server_addr = resolver.get_addr();
    int sockfd = server_addr.get_socket();
    auto bind_addr = server_addr.get_socketaddr();
    CHECK_CALL(bind,sockfd,bind_addr._addr,bind_addr._len);
    // 最大连接数 SOMAXCONN
    CHECK_CALL(listen,sockfd,SOMAXCONN);
    while(1){
        socketaddr_container client_addr;
        int connfd = CHECK_CALL(accept,sockfd,&client_addr._addr,&client_addr._len);
        pool.emplace_back(std::thread([connfd](){
            http_request_parser parser;
            cout << "new connection! ID:"<<connfd << endl;
            char buffer [1024];
            do{
                ssize_t bytes = CHECK_CALL(read,connfd,buffer,sizeof(buffer));
                if(bytes == 0){
                    close(connfd);
                    cout << "connection closed! ID:"<<connfd << endl;
                    return;
                }
                parser.parse(buffer,bytes);
            }while(parser.need_more_data());
            //重复读取直到数据收集完毕
            parser.parse_head();
            cout <<"recv:"<<std::string_view(buffer,sizeof(buffer));
            memset(buffer,0,sizeof(buffer));
        
            strcpy(buffer,"HTTP/1.1  200 OK\r\nserver:http\r\nconnection: close\r\ncontent-type: text/html\r\n content-length: 13\r\n\r\nHello, world!");
            //conten length 为了解决粘包问题
            CHECK_CALL(write,connfd,buffer,sizeof(buffer));
            close(connfd);
            return;
        }
        ));
    }
    for(auto &t:pool){
        t.join();
    }
    return 0;
}
int main(int argc, char const *argv[]){
    // std:: string str ="GET / HTTP/1.1";
    
    // cout <<str.substr(0,str.find(' '));
    func(argc,argv);
}