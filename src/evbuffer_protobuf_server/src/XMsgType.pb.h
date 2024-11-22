// Generated by the protocol buffer compiler.  DO NOT EDIT!
// NO CHECKED-IN PROTOBUF GENCODE
// source: XMsgType.proto
// Protobuf C++ Version: 5.28.3

#ifndef GOOGLE_PROTOBUF_INCLUDED_XMsgType_2eproto_2epb_2eh
#define GOOGLE_PROTOBUF_INCLUDED_XMsgType_2eproto_2epb_2eh

#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "google/protobuf/runtime_version.h"
#if PROTOBUF_VERSION != 5028003
#error "Protobuf C++ gencode is built with an incompatible version of"
#error "Protobuf C++ headers/runtime. See"
#error "https://protobuf.dev/support/cross-version-runtime-guarantee/#cpp"
#endif
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/arena.h"
#include "google/protobuf/arenastring.h"
#include "google/protobuf/generated_message_tctable_decl.h"
#include "google/protobuf/generated_message_util.h"
#include "google/protobuf/metadata_lite.h"
#include "google/protobuf/generated_message_reflection.h"
#include "google/protobuf/repeated_field.h"  // IWYU pragma: export
#include "google/protobuf/extension_set.h"  // IWYU pragma: export
#include "google/protobuf/generated_enum_reflection.h"
// @@protoc_insertion_point(includes)

// Must be included last.
#include "google/protobuf/port_def.inc"

#define PROTOBUF_INTERNAL_EXPORT_XMsgType_2eproto

namespace google {
namespace protobuf {
namespace internal {
class AnyMetadata;
}  // namespace internal
}  // namespace protobuf
}  // namespace google

// Internal implementation detail -- do not use these members.
struct TableStruct_XMsgType_2eproto {
  static const ::uint32_t offsets[];
};
extern const ::google::protobuf::internal::DescriptorTable
    descriptor_table_XMsgType_2eproto;
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

namespace XMsg {
enum MsgType : int {
  NONE_DO_NOT_USE = 0,
  MT_LOGIN_REQ = 1,
  MT_LOGIN_RES = 2,
  MT_MAX_TYPE = 65535,
  MsgType_INT_MIN_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::min(),
  MsgType_INT_MAX_SENTINEL_DO_NOT_USE_ =
      std::numeric_limits<::int32_t>::max(),
};

bool MsgType_IsValid(int value);
extern const uint32_t MsgType_internal_data_[];
constexpr MsgType MsgType_MIN = static_cast<MsgType>(0);
constexpr MsgType MsgType_MAX = static_cast<MsgType>(65535);
constexpr int MsgType_ARRAYSIZE = 65535 + 1;
const ::google::protobuf::EnumDescriptor*
MsgType_descriptor();
template <typename T>
const std::string& MsgType_Name(T value) {
  static_assert(std::is_same<T, MsgType>::value ||
                    std::is_integral<T>::value,
                "Incorrect type passed to MsgType_Name().");
  return ::google::protobuf::internal::NameOfEnum(MsgType_descriptor(), value);
}
inline bool MsgType_Parse(absl::string_view name, MsgType* value) {
  return ::google::protobuf::internal::ParseNamedEnum<MsgType>(
      MsgType_descriptor(), name, value);
}

// ===================================================================



// ===================================================================




// ===================================================================


#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif  // __GNUC__
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif  // __GNUC__

// @@protoc_insertion_point(namespace_scope)
}  // namespace XMsg


namespace google {
namespace protobuf {

template <>
struct is_proto_enum<::XMsg::MsgType> : std::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor<::XMsg::MsgType>() {
  return ::XMsg::MsgType_descriptor();
}

}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCLUDED_XMsgType_2eproto_2epb_2eh
