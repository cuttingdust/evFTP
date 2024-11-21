#include "XMsgHead.pb.h"

#include <google/protobuf/compiler/importer.h>
#include <google/protobuf/dynamic_message.h>

#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char *argv[])
{
    ///////////////////////////////////////////////01 �������úͻ�ȡ����///////////////////////////////////////////////
    /// ����Protobuf
    XMsg::XMsgHead head;
    head.set_msgsize(1024);
    head.set_msgstr("===============test for protobuf ============");

    std::cout << "head msgsize: " << head.msgsize() << std::endl;
    std::cout << "head msgstr: " << head.msgstr() << std::endl;

    ///////////////////////////////////////////////02 �������л��ͷ����л�///////////////////////////////////////////////
    /// ���л���string
    std::string str;
    head.SerializeToString(&str);
    std::cout << "str: " << str << std::endl;

    /// ��string�����л�
    XMsg::XMsgHead head2;
    head2.ParseFromString(str);
    std::cout << "head2 msgsize: " << head2.msgsize() << std::endl;
    std::cout << "head2 msgstr: " << head2.msgstr() << std::endl;

    XMsg::XMsgHead head3;
    head3.ParseFromArray(str.data(), str.size());
    std::cout << "head3 msgsize: " << head3.msgsize() << std::endl;
    std::cout << "head3 msgstr: " << head3.msgstr() << std::endl;

    /// ���л����ļ�
    std::ofstream ofs;
    ofs.open("test.txt", std::ios::binary);
    head.SerializeToOstream(&ofs);
    ofs.close();

    /// ���ļ������л�
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

    ///////////////////////////////////////////////03 ���Է���///////////////////////////////////////////////
    std::cout << "==============================test protobuf reflect===============================" << std::endl;
    XMsg::XMsgHead msg;

    /// ��������
    auto descriptor = msg.GetDescriptor();

    /// �������
    auto reflecter = msg.GetReflection();

    /// ���Զ���
    std::string fname      = "msgSize";
    auto        size_field = descriptor->FindFieldByName(fname);
    if (!size_field)
    {
        std::cerr << "FindFieldByName " << fname << " failed!" << std::endl;
        return -1;
    }

    /// �������Ե�ֵ
    reflecter->SetInt32(&msg, size_field, 1024);


    /// ��ȡ���Ե�ֵ
    std::cout << fname << "= " << reflecter->GetInt32(msg, size_field) << std::endl;


    ///////////////////////////////////////////////04 ����ʱ����proto�ļ�///////////////////////////////////////////////
    std::cout << "====================test protobuf parse proto file at running time====================" << std::endl;
    /// ׼���ļ�ϵͳ
    google::protobuf::compiler::DiskSourceTree source_tree;

    ///  ����һ���ַ�����������·���滻
    source_tree.MapPath("protobuf_root", "./");

    /// ������̬������
    google::protobuf::compiler::Importer importer(&source_tree, NULL);
    std::string                          protoFilename = "XMsgHead.proto";
    /// ��̬����proto�ļ�������Ѿ��������ֱ�ӷ��ػ������
    std::string path = "protobuf_root/";
    path += protoFilename;
    /// ���ص��ļ��������󣬰����ļ�������ϵ
    auto file_desc = importer.Import(path);
    if (!file_desc)
    {
        std::cerr << "importer.Import " << path << " failed!" << std::endl;
        return -1;
    }
    /// ��ȡMessage����
    auto messageDesc = importer.pool()->FindMessageTypeByName("XMsg.XMsgHead");

    /// ����Ϣ������������Ϣ����
    google::protobuf::DynamicMessageFactory factory;
    ///����һ������ԭ��
    auto messageProto = factory.GetPrototype(messageDesc);
    auto msgTest      = messageProto->New();
    {
        /// ��������
        auto descriptor = msgTest->GetDescriptor();

        /// �������
        auto reflecter = msgTest->GetReflection();

        /// ���Զ���
        std::string fname     = "msgStr";
        auto        str_field = descriptor->FindFieldByName(fname);
        if (!str_field)
        {
            std::cerr << "FindFieldByName " << fname << " failed!" << std::endl;
            return -1;
        }

        /// �������Ե�ֵ
        reflecter->SetString(msgTest, str_field, "test dy proto str");

        /// ��ȡ���Ե�ֵ
        std::cout << "test dy proto " << fname << "= " << reflecter->GetString(*msgTest, str_field) << std::endl;
    }

    // std::cin.get();
    return 0;
}
