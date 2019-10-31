#ifndef __H_WAGGLE_DATAGRAM__
#define __H_WAGGLE_DATAGRAM__

// template <int N>
// struct datagram {
//   unsigned int protocol_version;
//   unsigned int timestamp;
//   unsigned int packet_seq;
//   unsigned int packet_type;
//   unsigned int plugin_id;
//   unsigned int plugin_major_version;
//   unsigned int plugin_minor_version;
//   unsigned int plugin_patch_version;
//   unsigned int plugin_instance;
//   unsigned int plugin_run_id;
//   bytebuffer<N> body;
// };

// template <class DG>
// void pack_datagram(writer &w, DG &dg) {
//   basic_encoder e(w);
//   e.encode_uint(dg.body.size(), 3);           // [Length (3B)]
//   e.encode_uint(dg.protocol_version, 1);      // [Protocol_version (1B)]
//   e.encode_uint(dg.timestamp, 4);             // [time (4B)]
//   e.encode_uint(dg.packet_seq, 2);            // [Packet_Seq (2B)]
//   e.encode_uint(dg.packet_type, 1);           // [Packet_type (1B)]
//   e.encode_uint(dg.plugin_id, 2);             // [Plugin ID (2B)]
//   e.encode_uint(dg.plugin_major_version, 1);  // [Plugin Maj Ver (1B)]
//   e.encode_uint(dg.plugin_minor_version, 1);  // [Plugin Min Ver (1B)]
//   e.encode_uint(dg.plugin_patch_version, 1);  // [Plugin Build Ver (1B)]
//   e.encode_uint(dg.plugin_instance, 1);       // [Plugin Instance (1B)]
//   e.encode_uint(dg.plugin_run_id, 2);         // [Plugin Run ID (2B)]
//   e.encode_bytes(dg.body.bytes(), dg.body.size());

//   // framing footer
//   e.encode_uint(calc_crc8(dg.body.bytes(), dg.body.size()),
//                 1);  // [CRC (1B) -> (2B)]
//   // TODO make crc16 instead of 8. and computed against header + body...
// }

// int find_byte(char x, const char *b, int n) {
//   for (int i = 0; i < n; i++) {
//     if (b[i] == x) {
//       return i;
//     }
//   }

//   return -1;
// }

// need to have a way of indicating whether a datagram was actually unpacked or
// not. yep...this is a little ugly here... reading / writing should be managed
// at a higher level
// template <class DG>
// bool unpack_datagram(reader &r, DG &dg) {
//   bytereader r(buf.bytes(), buf.size());

//   int len = unpack_uint(r, 3);                  // [Length (3B)]
//   dg.protocol_version = unpack_uint(r, 1);      // [Protocol_version (1B)]
//   dg.timestamp = unpack_uint(r, 4);             // [time (4B)]
//   dg.packet_seq = unpack_uint(r, 2);            // [Packet_Seq (2B)]
//   dg.packet_type = unpack_uint(r, 1);           // [Packet_type (1B)]
//   dg.plugin_id = unpack_uint(r, 2);             // [Plugin ID (2B)]
//   dg.plugin_major_version = unpack_uint(r, 1);  // [Plugin Maj Ver (1B)]
//   dg.plugin_minor_version = unpack_uint(r, 1);  // [Plugin Min Ver (1B)]
//   dg.plugin_patch_version = unpack_uint(r, 1);  // [Plugin Build Ver (1B)]
//   dg.plugin_instance = unpack_uint(r, 1);       // [Plugin Instance (1B)]
//   dg.plugin_run_id = unpack_uint(r, 2);         // [Plugin Run ID (2B)]

//   // Is this consistent with Pack??
//   dg.body.clear();
//   dg.body.readfrom(r, len);

//   if (r.error()) {
//     return false;
//   }

//   char recv_crc = unpack_uint(r, 1);
//   char calc_crc = calc_crc8(dg.body.bytes(), dg.body.size());

//   if (recv_crc != calc_crc) {
//     return false;
//   }

//   return !r.error();
// }

#endif
