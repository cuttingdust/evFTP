// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: XMsgType.proto

#include "XMsgType.pb.h"

#include <algorithm>
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/extension_set.h"
#include "google/protobuf/wire_format_lite.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/reflection_ops.h"
#include "google/protobuf/wire_format.h"
#include "google/protobuf/generated_message_tctable_impl.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"
PROTOBUF_PRAGMA_INIT_SEG
namespace _pb = ::google::protobuf;
namespace _pbi = ::google::protobuf::internal;
namespace _fl = ::google::protobuf::internal::field_layout;
namespace XMsg {
}  // namespace XMsg
static const ::_pb::EnumDescriptor* file_level_enum_descriptors_XMsgType_2eproto[1];
static constexpr const ::_pb::ServiceDescriptor**
    file_level_service_descriptors_XMsgType_2eproto = nullptr;
const ::uint32_t TableStruct_XMsgType_2eproto::offsets[1] = {};
static constexpr ::_pbi::MigrationSchema* schemas = nullptr;
static constexpr ::_pb::Message* const* file_default_instances = nullptr;
const char descriptor_table_protodef_XMsgType_2eproto[] PROTOBUF_SECTION_VARIABLE(protodesc_cold) = {
    "\n\016XMsgType.proto\022\004XMsg*U\n\007MsgType\022\023\n\017NON"
    "E_DO_NOT_USE\020\000\022\020\n\014MT_LOGIN_REQ\020\001\022\020\n\014MT_L"
    "OGIN_RES\020\002\022\021\n\013MT_MAX_TYPE\020\377\377\003b\006proto3"
};
static ::absl::once_flag descriptor_table_XMsgType_2eproto_once;
const ::_pbi::DescriptorTable descriptor_table_XMsgType_2eproto = {
    false,
    false,
    117,
    descriptor_table_protodef_XMsgType_2eproto,
    "XMsgType.proto",
    &descriptor_table_XMsgType_2eproto_once,
    nullptr,
    0,
    0,
    schemas,
    file_default_instances,
    TableStruct_XMsgType_2eproto::offsets,
    nullptr,
    file_level_enum_descriptors_XMsgType_2eproto,
    file_level_service_descriptors_XMsgType_2eproto,
};

// This function exists to be marked as weak.
// It can significantly speed up compilation by breaking up LLVM's SCC
// in the .pb.cc translation units. Large translation units see a
// reduction of more than 35% of walltime for optimized builds. Without
// the weak attribute all the messages in the file, including all the
// vtables and everything they use become part of the same SCC through
// a cycle like:
// GetMetadata -> descriptor table -> default instances ->
//   vtables -> GetMetadata
// By adding a weak function here we break the connection from the
// individual vtables back into the descriptor table.
PROTOBUF_ATTRIBUTE_WEAK const ::_pbi::DescriptorTable* descriptor_table_XMsgType_2eproto_getter() {
  return &descriptor_table_XMsgType_2eproto;
}
// Force running AddDescriptors() at dynamic initialization time.
PROTOBUF_ATTRIBUTE_INIT_PRIORITY2
static ::_pbi::AddDescriptorsRunner dynamic_init_dummy_XMsgType_2eproto(&descriptor_table_XMsgType_2eproto);
namespace XMsg {
const ::google::protobuf::EnumDescriptor* MsgType_descriptor() {
  ::google::protobuf::internal::AssignDescriptors(&descriptor_table_XMsgType_2eproto);
  return file_level_enum_descriptors_XMsgType_2eproto[0];
}
PROTOBUF_CONSTINIT const uint32_t MsgType_internal_data_[] = {
    196608u, 65536u, 65535u, };
bool MsgType_IsValid(int value) {
  return ::_pbi::ValidateEnum(value, MsgType_internal_data_);
}
// @@protoc_insertion_point(namespace_scope)
}  // namespace XMsg
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
// @@protoc_insertion_point(global_scope)
#include "google/protobuf/port_undef.inc"