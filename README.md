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
