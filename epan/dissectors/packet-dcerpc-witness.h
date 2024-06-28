/* DO NOT EDIT
	This file was automatically generated by Pidl
	from witness.idl and witness.cnf.

	Pidl is a perl based IDL compiler for DCE/RPC idl files.
	It is maintained by the Samba team, not the Wireshark team.
	Instructions on how to download and install Pidl can be
	found at https://wiki.wireshark.org/Pidl
*/

#include "packet-dcerpc-misc.h"

#ifndef __PACKET_DCERPC_WITNESS_H
#define __PACKET_DCERPC_WITNESS_H

#define WITNESS_V1 (0x00010001)
#define WITNESS_V2 (0x00020000)
#define WITNESS_UNSPECIFIED_VERSION (0xFFFFFFFF)
extern const value_string witness_witness_version_vals[];
int witness_dissect_enum_version(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, guint32 *param _U_);
#define WITNESS_STATE_UNKNOWN (0x00)
#define WITNESS_STATE_AVAILABLE (0x01)
#define WITNESS_STATE_UNAVAILABLE (0xff)
extern const value_string witness_witness_interfaceInfo_state_vals[];
int witness_dissect_enum_interfaceInfo_state(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, guint16 *param _U_);
int witness_dissect_bitmap_interfaceInfo_flags(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_struct_interfaceInfo(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *parent_tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_struct_interfaceList(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *parent_tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
#define WITNESS_NOTIFY_RESOURCE_CHANGE (1)
#define WITNESS_NOTIFY_CLIENT_MOVE (2)
#define WITNESS_NOTIFY_SHARE_MOVE (3)
#define WITNESS_NOTIFY_IP_CHANGE (4)
extern const value_string witness_witness_notifyResponse_type_vals[];
int witness_dissect_enum_notifyResponse_type(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, guint32 *param _U_);
#define WITNESS_RESOURCE_STATE_UNKNOWN (0x00)
#define WITNESS_RESOURCE_STATE_AVAILABLE (0x01)
#define WITNESS_RESOURCE_STATE_UNAVAILABLE (0xff)
extern const value_string witness_witness_ResourceChange_type_vals[];
int witness_dissect_enum_ResourceChange_type(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, guint32 *param _U_);
int witness_dissect_struct_ResourceChange(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *parent_tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_bitmap_IPaddrInfo_flags(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_struct_IPaddrInfo(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *parent_tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_struct_IPaddrInfoList(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *parent_tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_struct_notifyResponse(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *parent_tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
int witness_dissect_bitmap_RegisterEx_flags(tvbuff_t *tvb _U_, int offset _U_, packet_info *pinfo _U_, proto_tree *tree _U_, dcerpc_info* di _U_, uint8_t *drep _U_, int hf_index _U_, uint32_t param _U_);
#endif /* __PACKET_DCERPC_WITNESS_H */
