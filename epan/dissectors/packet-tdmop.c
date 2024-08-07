/* packet-tdmop.c
 * Routines for TDM over Packet network disassembly
 * Copyright 2015, Andrew Chernyh <andew.chernyh@gmail.com>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
/*
 * TDMoP protocol is proprietary protocol developed by "NSC Communication Siberia Ltd."
 */
#include "config.h"
#include <epan/packet.h>
#include <epan/conversation.h>
#include <epan/prefs.h>

/*Using of ethertype 0x0808(assigned to Frame Relay ARP) was implemented in hardware, when ethertype was not assigned*/
#define ETHERTYPE_TDMOP       0

#define MAX_DCHANNEL_LEN      128

void proto_register_tdmop(void);
void proto_reg_handoff_tdmop(void);

static dissector_handle_t tdmop_handle;

static int proto_tdmop;
static int ett_tdmop;
static int ett_tdmop_channel;

static int hf_tdmop_TransferID;
static int hf_tdmop_DstCh;
static int hf_tdmop_SrcCh;
static int hf_tdmop_Flags;
static int hf_tdmop_Flags_no_data;
static int hf_tdmop_Flags_lost_request;
static int hf_tdmop_Flags_remote_no_data;
static int hf_tdmop_Flags_compressed;
static int hf_tdmop_SrcDst;
static int hf_tdmop_SeqNum;
static int hf_tdmop_LastRecv;
static int hf_tdmop_Delay;
static int hf_tdmop_Reserved;
static int hf_tdmop_payload;
static int hf_tdmop_Compression_mask;

static dissector_handle_t lapd_handle;

static int pref_tdmop_d_channel      = 16;
static uint32_t pref_tdmop_mask        = 0xFFFFFFFFUL;
static uint32_t pref_tdmop_ethertype   = ETHERTYPE_TDMOP;

#define TDMOP_FLAG_NO_DATA (1<<3)
#define TDMOP_FLAG_REMOTE_NO_DATA (1<<2)
#define TDMOP_FLAG_COMPRESSED (1<<4)
#define TDMOP_FLAG_LOST_REQUEST ((1<<3)|(0x02))

static uint8_t reverse_map[256]=
{
0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};

static int dissect_tdmop(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree, void* data _U_)
{
    uint8_t   dchannel_data[MAX_DCHANNEL_LEN];
    unsigned dchannel_len;
    uint8_t   flags;
    int offset;
    proto_item    *ti;
    proto_tree    *tdmop_tree;
    uint32_t dstch, srcch;
    flags = tvb_get_uint8(tvb, 4);
    offset = 0;
    col_set_str(pinfo->cinfo, COL_PROTOCOL, "TDMoP");
    col_clear(pinfo->cinfo, COL_INFO);
    if (flags & TDMOP_FLAG_LOST_REQUEST)
    {
        col_set_str(pinfo->cinfo, COL_INFO, "Lost Request");
    }

    ti = proto_tree_add_item(tree, proto_tdmop, tvb, 0, -1, ENC_NA);
    tdmop_tree = proto_item_add_subtree(ti, ett_tdmop);
    /*path info*/
    proto_tree_add_item(tdmop_tree, hf_tdmop_TransferID, tvb, offset, 4, ENC_LITTLE_ENDIAN);
    offset += 2;
    proto_tree_add_item_ret_uint(tdmop_tree, hf_tdmop_DstCh, tvb, offset, 1, ENC_LITTLE_ENDIAN, &dstch);
    offset += 1;
    proto_tree_add_item_ret_uint(tdmop_tree, hf_tdmop_SrcCh, tvb, offset, 1, ENC_LITTLE_ENDIAN, &srcch);
    offset += 1;

    /*conversation*/
    conversation_set_conv_addr_port_endpoints(pinfo, &pinfo->src, &pinfo->dst, CONVERSATION_TDMOP, srcch, dstch);

    /*flags*/
    proto_tree_add_item(tdmop_tree, hf_tdmop_Flags, tvb, offset, 1, ENC_NA);
    proto_tree_add_item(tdmop_tree, hf_tdmop_Flags_no_data, tvb, offset, 1, ENC_NA);
    proto_tree_add_item(tdmop_tree, hf_tdmop_Flags_lost_request, tvb, offset, 1, ENC_NA);
    proto_tree_add_item(tdmop_tree, hf_tdmop_Flags_remote_no_data, tvb, offset, 1, ENC_NA);
    proto_tree_add_item(tdmop_tree, hf_tdmop_Flags_compressed, tvb, offset, 1, ENC_NA);
    offset += 1;
    /*sequence and delay info*/
    proto_tree_add_item(tdmop_tree, hf_tdmop_SrcDst, tvb, offset, 1, ENC_LITTLE_ENDIAN);
    offset += 1;
    proto_tree_add_item(tdmop_tree, hf_tdmop_SeqNum, tvb, offset, 2, ENC_LITTLE_ENDIAN);
    offset += 2;
    proto_tree_add_item(tdmop_tree, hf_tdmop_LastRecv, tvb, offset, 2, ENC_LITTLE_ENDIAN);
    offset += 2;
    proto_tree_add_item(tdmop_tree, hf_tdmop_Delay, tvb, offset, 2, ENC_LITTLE_ENDIAN);
    offset += 2;
    proto_tree_add_item(tdmop_tree, hf_tdmop_Reserved, tvb, offset, 2, ENC_LITTLE_ENDIAN);
    offset += 2;
    if ((flags & TDMOP_FLAG_NO_DATA)==0)
    {
        int len;
        int blockid;
        dchannel_len=0;
        len=tvb_captured_length_remaining(tvb, 0);
        proto_tree_add_item(tdmop_tree, hf_tdmop_payload, tvb, offset, -1, ENC_NA);
        blockid=0;
        /*demux TDM stream*/
        while (offset<len)
        {
            proto_tree *currentblock = proto_tree_add_subtree_format(tdmop_tree, tvb, 0, 0, ett_tdmop_channel, 0, "Block %d", blockid);
            uint32_t mask;
            int i;
            int j;
            blockid++;
            mask = pref_tdmop_mask; /*default mask is for timeslots 1-32*/
            if (flags&TDMOP_FLAG_COMPRESSED)
            {
                mask = tvb_get_letohl(tvb,offset);
                mask = ((mask >> 16) & 0xFFFF)|((mask & 0xFFFF) << 16);
                proto_tree_add_uint(currentblock, hf_tdmop_Compression_mask, tvb, offset, 4, mask);
                offset+=4;
            }
            for (i=0; i<32; i++)
            {
                if (mask & (1UL<<i))
                {
                    proto_tree *subtree;
                    tvbuff_t *cdata;
                    subtree = proto_tree_add_subtree_format(currentblock, tvb, 0, 0, ett_tdmop_channel, 0, "Channel %d", i);
                    cdata = tvb_new_subset_length(tvb, offset, 4);
                    if (i==pref_tdmop_d_channel)
                    {
                        if (dchannel_len + 4 < MAX_DCHANNEL_LEN)
                        {
                            for (j = 0; j < 4; j++)
                            {
                                dchannel_data[dchannel_len+j]=reverse_map[tvb_get_uint8(cdata, j)];
                            }
                            dchannel_len += 4;
                        }
                    } else
                    {
                        call_data_dissector(cdata, pinfo, subtree);
                    }
                    offset += 4;
                }
            }
        }
        if (dchannel_len>0)
        {
            uint8_t *buff = (uint8_t *)wmem_memdup(pinfo->pool, dchannel_data, dchannel_len);
            tvbuff_t *new_tvb;
            new_tvb = tvb_new_child_real_data(tvb, buff, dchannel_len, dchannel_len);
            call_dissector(lapd_handle, new_tvb, pinfo, tree);
        }
    }
    return tvb_captured_length(tvb);
}

void proto_register_tdmop(void)
{
    module_t *tdmop_module;
    static hf_register_info hf[] =
    {
        {
            &hf_tdmop_TransferID,
            {    "TDMoP Transfer ID", "tdmop.transferid",
                FT_UINT32, BASE_HEX,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_DstCh,
            {    "TDMoP Dst Ch", "tdmop.dstch",
                FT_UINT8, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_SrcCh,
            {    "TDMoP Src Ch", "tdmop.srcch",
                FT_UINT8, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Flags,
            {    "TDMoP Flags", "tdmop.flags",
                FT_UINT8, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Flags_lost_request,
            {    "TDMoP Lost Request Flag", "tdmop.flags.lostrequest",
                FT_BOOLEAN, 8,
                NULL, TDMOP_FLAG_LOST_REQUEST,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Flags_no_data,
            {    "TDMoP No data flag", "tdmop.flags.nodata",
                FT_BOOLEAN, 8,
                NULL, TDMOP_FLAG_NO_DATA,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Flags_remote_no_data,
            {    "TDMoP No data received from remote side flag", "tdmop.flags.remotenodata",
                FT_BOOLEAN, 8,
                NULL, TDMOP_FLAG_REMOTE_NO_DATA,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Flags_compressed,
            {    "TDMoP compressed framed", "tdmop.flags.compressed",
                FT_BOOLEAN, 8,
                NULL, TDMOP_FLAG_COMPRESSED,
                NULL, HFILL}
        },
        {
            &hf_tdmop_SrcDst,
            {    "TDMoP Short SrcDst", "tdmop.srcdst",
                FT_UINT8, BASE_HEX,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_SeqNum,
            {    "TDMoP Sequence number", "tdmop.seqnum",
                FT_UINT16, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_LastRecv,
            {    "TDMoP Last Received number", "tdmop.recvnumber",
                FT_UINT16, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Delay,
            {    "TDMoP Delay", "tdmop.delay",
                FT_UINT16, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Reserved,
            {    "TDMoP Reserved", "tdmop.reserved",
                FT_UINT16, BASE_DEC,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_payload,
            {    "TDMoP Payload", "tdmop.payload",
                FT_BYTES, BASE_NONE,
                NULL, 0x0,
                NULL, HFILL}
        },
        {
            &hf_tdmop_Compression_mask,
            {    "TDMoP Compression mask", "tdmop.cmask",
                FT_UINT32, BASE_HEX,
                NULL, 0x0,
                NULL, HFILL}
        }
    };
    static int *ett[] = {
        &ett_tdmop,
        &ett_tdmop_channel
    };
    proto_tdmop = proto_register_protocol ("TDMoP protocol", "TDMoP", "tdmop");
    proto_register_field_array(proto_tdmop, hf, array_length(hf));
    proto_register_subtree_array(ett, array_length(ett));
    tdmop_handle = register_dissector("tdmop", dissect_tdmop, proto_tdmop);
    tdmop_module = prefs_register_protocol(proto_tdmop, proto_reg_handoff_tdmop);
    prefs_register_uint_preference(tdmop_module, "d_channel",
                    "TDMoP D-Channel",
                    "The TDMoD channel that contains the D-Channel.",
                    10, &pref_tdmop_d_channel);
    prefs_register_uint_preference(tdmop_module, "ts_mask",
                    "TDMoP default timeslot mask",
                    "The bitmask of channels in uncompressed TDMoP frame",
                    16, &pref_tdmop_mask);
    prefs_register_uint_preference(tdmop_module, "ethertype",
                    "Ethertype for TDMoP stream(Usually 0808)",
                    "The ethertype assigned to TDMoP (without IP/UDP) stream",
                    16, &pref_tdmop_ethertype);
}

void proto_reg_handoff_tdmop(void)
{
    static bool init = false;
    static uint32_t current_tdmop_ethertype;
    if (!init)
    {
        dissector_add_for_decode_as_with_preference("udp.port", tdmop_handle);

        if (pref_tdmop_ethertype) {
            dissector_add_uint("ethertype", pref_tdmop_ethertype, tdmop_handle);
        }
        lapd_handle = find_dissector_add_dependency("lapd-bitstream", proto_tdmop);
        current_tdmop_ethertype = pref_tdmop_ethertype;
        init = true;
    }
    if (current_tdmop_ethertype != pref_tdmop_ethertype)
    {
        dissector_delete_uint("ethertype", current_tdmop_ethertype, tdmop_handle);
        if (pref_tdmop_ethertype) {
            dissector_add_uint("ethertype", pref_tdmop_ethertype, tdmop_handle);
        }
        current_tdmop_ethertype = pref_tdmop_ethertype;
    }
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 4
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=4 tabstop=8 expandtab:
 * :indentSize=4:tabSize=8:noTabs=true:
 */
