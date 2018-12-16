## grpc帮助库
一些使用gRPC过程中可能会用到的库。

### zap
基于`log4cpp`的`json`日志库，应用于`EFK`等日志收集。

### protoc-gen-cpptags
`protoc`插件，与`github.com/grpc-ecosystem/go-grpc-middleware/tags`功能类似，用于从`proto`文件中定义哪些字段需要输出。
#### 使用方法
1. 在`.proto`文件中添加`import cpptags.proto`
1. 在需要输出的字段添加[(cpptags.log_field)="custom_name"]，其中`custom_name`会在最终的JSON中作为KEY存在
1. 调用`CppTagsMessageTypeMask<T>::MaskMessageToJson`可将各字段输出成JSON

### grpc-logging-plugin
`grpc`插件，在`ServerContext::client_metadata`中添加`grpc.service`及`grpc.method`。
#### 使用方法
1. 将文件拷贝至`grpc`目录。
1. 根据使用的生成工具，修改`CMakeLists.txt`或`Makefile`或`BUILD`，在`add_library(grpc++`、`LIBGRPC++_SRC`、`GRPCXX_SRCS`中添加`src/cpp/ext/filter/logging/logging.cc`
1. 在启动服务器之前，调用`grpc::logging::probe_logging_field_to_clientmeta()`（在头文件`grpcpp/ext/logging.h`中声明）