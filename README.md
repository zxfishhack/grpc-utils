## grpc帮助库
一些使用gRPC过程中可能会用到的库。

### zap
基于`log4cpp`的`json`日志库，应用于`EFK`等日志收集。

### protoc-gen-cpptags
`protoc`插件，与`github.com/grpc-ecosystem/go-grpc-middleware/tags`功能类似，用于从`proto`文件中定义哪些字段需要输出。基于`FieldMask`实现，配合`json_util.h`可实现字段输出。
