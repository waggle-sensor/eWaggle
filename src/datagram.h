#ifndef __H_WAGGLE_DATAGRAM__
#define __H_WAGGLE_DATAGRAM__

template <int N>
struct datagram {
  unsigned int protocol_version;
  unsigned int timestamp;
  unsigned int packet_seq;
  unsigned int packet_type;
  unsigned int plugin_id;
  unsigned int plugin_major_version;
  unsigned int plugin_minor_version;
  unsigned int plugin_patch_version;
  unsigned int plugin_instance;
  unsigned int plugin_run_id;
  bytebuffer<N> body;
};

// Now, we can use different functions for actually sending / recving
// datagrams.

template <class writerT, class DG>
void pack_datagram(writerT &w, DG &dg) {
  pack_uint(w, dg.body.size(), 3);         // [Length (3B)]
  pack_uint(w, dg.ProtocolVersion, 1);     // [Protocol_version (1B)]
  pack_uint(w, dg.Timestamp, 4);           // [time (4B)]
  pack_uint(w, dg.PacketSeq, 2);           // [Packet_Seq (2B)]
  pack_uint(w, dg.PacketType, 1);          // [Packet_type (1B)]
  pack_uint(w, dg.PluginID, 2);            // [Plugin ID (2B)]
  pack_uint(w, dg.PluginMajorVersion, 1);  // [Plugin Maj Ver (1B)]
  pack_uint(w, dg.PluginMinorVersion, 1);  // [Plugin Min Ver (1B)]
  pack_uint(w, dg.PluginPatchVersion, 1);  // [Plugin Build Ver (1B)]
  pack_uint(w, dg.PluginInstance, 1);      // [Plugin Instance (1B)]
  pack_uint(w, dg.PluginRunID, 2);         // [Plugin Run ID (2B)]
  pack_bytes(w, dg.body.bytes(), dg.body.size());

  // framing footer
  pack_uint(w, calc_crc8(dg.body.bytes(), dg.body.size()), 1);  // [CRC (1B)]
  // TODO make crc16 instead of 8. and computed against header + body...
  // length should be length all the way until the end...
}

int find_byte(char x, const char *b, int n) {
  for (int i = 0; i < n; i++) {
    if (b[i] == x) {
      return i;
    }
  }

  return -1;
}

// need to have a way of indicating whether a datagram was actually unpacked or
// not.
template <class B, class DG>
bool unpack_datagram(B &buf, DG &dg) {
  bytereader r(buf.bytes(), buf.size());

  int len = unpack_uint(r, 3);                  // [Length (3B)]
  dg.protocol_version = unpack_uint(r, 1);      // [Protocol_version (1B)]
  dg.timestamp = unpack_uint(r, 4);             // [time (4B)]
  dg.packet_seq = unpack_uint(r, 2);            // [Packet_Seq (2B)]
  dg.packet_type = unpack_uint(r, 1);           // [Packet_type (1B)]
  dg.plugin_id = unpack_uint(r, 2);             // [Plugin ID (2B)]
  dg.plugin_major_version = unpack_uint(r, 1);  // [Plugin Maj Ver (1B)]
  dg.plugin_minor_version = unpack_uint(r, 1);  // [Plugin Min Ver (1B)]
  dg.plugin_patch_version = unpack_uint(r, 1);  // [Plugin Build Ver (1B)]
  dg.plugin_instance = unpack_uint(r, 1);       // [Plugin Instance (1B)]
  dg.plugin_run_id = unpack_uint(r, 2);         // [Plugin Run ID (2B)]

  // Is this consistent with Pack??
  dg.body.clear();
  copyn(r, dg.body, len);

  if (r.error()) {
    return false;
  }

  char recv_crc = unpack_uint(r, 1);
  char calc_crc = calc_crc8(dg.body.bytes(), dg.body.size());

  if (recv_crc != calc_crc) {
    return false;
  }

  return !r.error();
}

#endif
