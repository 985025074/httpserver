# http 服务器搭建日志
21点34分 2024年9月21日
http 无视大小写规定
报文以\r\n\r\n分隔头部和主体
\r\n 结束头的一行
# 处理分隔符的 用这个：
    
        std::istringstream iss(_head.substr(0,_head.find("\r\n")));//先处理首行
        iss >> _headers["method"] >> _headers["url"] >> _headers["version"];
巨爽无比
# 逆天vscode
为什么lambda return 和 {不隔开就不设别（变色）?
# curl 命令 
curl -X 方式 -v 详细信息 -d “键值对” 发送请求
