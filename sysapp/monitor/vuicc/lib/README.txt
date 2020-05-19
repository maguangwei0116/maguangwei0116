《COS Lib》

时间：
2020-03-13 21:23:09


头文件：
cos_api.h


静态库：
libxUICC.a
libskb.so
libasn.1.so
(SKB 由于是测试版本，到 4 月 13 日就过期了，到时候需要重新申请)


编译：
- Cmake
1. include_directories("cos_api.h 所在的目录")  
2. link_directories("存放静态库的目录")  
3. target_link_libraries(main 自己的模块 xUICC skb asn.1)


简要使用说明：
1. cos_init(NULL); 
	进行初始化。默认适配 9x07，其他平台如果需要适配，可以传 cos_type_operation_t 指针，并自己实现具体内容
2. cos_client_connect(NULL); 
	建立对 vUICC 的消息通道，并获取 io_id
3. cos_client_reset(io_id_t io_id, uint8_t *atr, uint16_t *atr_len); 
	重置 vUICC 内部状态，并获取 ATR，主要配合 qmi 的 reset 动作使用
4. cos_client_transport(io_id_t io_id, uint8_t io_type, uint8_t *req, uint16_t req_len, uint8_t *resp, uint16_t *resp_len);
	发送消息指令。type 类型在 cos_api.h 中有定义，req 和 resp 长度不会超过 260
	type 为 IO_PACKET_TYPE_DATA 的时候，发送 APDU，考虑到各平台差异性，61XX 和 6CXX 需要上层自己处理。
	type 为 IO_PACKET_TYPE_CTRL 的时候，发送控制指令，可用于获取版本信息等，暂时可能用不上，cos_api.h 中有提供基础接口
5. cos_client_close(io_id); 
	断开消息通道

其他 API 暂时还用不到
	
	
