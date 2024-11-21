#include "XMsgHead.pb.h"

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[])
{
    ///////////////////////////////////////////////01 测试设置和获取属性///////////////////////////////////////////////
    /// 设置Protobuf
    XMsg::XMsgHead head;
    head.set_msgsize(1024);
    head.set_msgstr("===============test for protobuf ============");

    std::cout << "head msgsize: " << head.msgsize() << std::endl;
    std::cout << "head msgstr: " << head.msgstr() << std::endl;

    ///////////////////////////////////////////////02 测试序列化和反序列化///////////////////////////////////////////////
    /// 序列化到string
    std::string str;
    head.SerializeToString(&str);
    std::cout << "str: " << str << std::endl;

    /// 从string反序列化
    XMsg::XMsgHead head2;
    head2.ParseFromString(str);
    std::cout << "head2 msgsize: " << head2.msgsize() << std::endl;
    std::cout << "head2 msgstr: " << head2.msgstr() << std::endl;

    XMsg::XMsgHead head3;
    head3.ParseFromArray(str.data(), str.size());
    std::cout << "head3 msgsize: " << head3.msgsize() << std::endl;
    std::cout << "head3 msgstr: " << head3.msgstr() << std::endl;

    /// 序列化到文件
    std::ofstream ofs;
    ofs.open("test.txt", std::ios::binary);
    head.SerializeToOstream(&ofs);
    ofs.close();

    /// 从文件反序列化
    XMsg::XMsgHead head4;
    head4.ParseFromFileDescriptor(fileno(fopen("test.txt", "r")));
    std::cout << "head4 msgsize: " << head4.msgsize() << std::endl;
    std::cout << "head4 msgstr: " << head4.msgstr() << std::endl;

    XMsg::XMsgHead head5;
    std::ifstream  ifs;
    ifs.open("test.txt", std::ios::binary);
    head5.ParseFromIstream(&ifs);
    std::cout << "head5 msgsize: " << head5.msgsize() << std::endl;
    std::cout << "head5 msgstr: " << head5.msgstr() << std::endl;

    ///////////////////////////////////////////////03 测试反射///////////////////////////////////////////////
    std::cout << "==============================test protobuf reflect===============================" << std::endl;
    XMsg::XMsgHead msg;

    /// 描述对象
    auto descriptor = msg.GetDescriptor();

    /// 反射对象
    auto reflecter = msg.GetReflection();

    /// 属性对象
    std::string fname      = "msgSize";
    auto        size_field = descriptor->FindFieldByName(fname);
    if (!size_field)
    {
        std::cerr << "FindFieldByName " << fname << " failed!" << std::endl;
        return -1;
    }

    /// 设置属性的值
    reflecter->SetInt32(&msg, size_field, 1024);


    /// 获取属性的值
    std::cout << fname << "= " << reflecter->GetInt32(msg, size_field) << std::endl;


    ///////////////////////////////////////////////04 运行时解析proto文件///////////////////////////////////////////////
    std::cout << "====================test protobuf parse proto file at running time====================" << std::endl;
    /// 准备文件系统
    google::protobuf::compiler::DiskSourceTree source_tree;

    ///  设置一个字符串，用来做路径替换
    source_tree.MapPath("protobuf_root", "./");

    /// 创建动态编译器
    google::protobuf::compiler::Importer importer(&source_tree, NULL);
    std::string                          protoFilename = "XMsgHead.proto";
    /// 动态编译proto文件，如果已经编译过，直接返回缓冲对象
    std::string path = "protobuf_root/";
    path += protoFilename;
    /// 返回的文件描述对象，包含文件依赖关系
    auto file_desc = importer.Import(path);
    if (!file_desc)
    {
        std::cerr << "importer.Import " << path << " failed!" << std::endl;
        return -1;
    }
    /// 获取Message类型
    auto messageDesc = importer.pool()->FindMessageTypeByName("XMsg.XMsgHead");

    /// 用消息工厂，创建消息对象
    google::protobuf::DynamicMessageFactory factory;
    ///创建一个类型原型
    auto messageProto = factory.GetPrototype(messageDesc);
    auto msgTest      = messageProto->New();
    {
        /// 描述对象
        auto descriptor = msgTest->GetDescriptor();

        /// 反射对象
        auto reflecter = msgTest->GetReflection();

        /// 属性对象
        std::string fname     = "msgStr";
        auto        str_field = descriptor->FindFieldByName(fname);
        if (!str_field)
        {
            std::cerr << "FindFieldByName " << fname << " failed!" << std::endl;
            return -1;
        }

        /// 设置属性的值
        reflecter->SetString(msgTest, str_field, "test dy proto str");

        /// 获取属性的值
        std::cout << "test dy proto " << fname << "= " << reflecter->GetString(*msgTest, str_field) << std::endl;
    }

    // std::cin.get();
    return 0;
}
