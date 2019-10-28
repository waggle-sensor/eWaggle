#ifndef __H_WAGGLE_SENSORGRAM__
#define __H_WAGGLE_SENSORGRAM__

template <int N>
struct sensorgram {
  unsigned long timestamp;
  unsigned int id;
  unsigned int inst;
  unsigned int sub_id;
  unsigned int source_id;
  unsigned int source_inst;
  bytebuffer<N> body;
};

template <class writerT, class SG>
void pack_sensorgram(writerT &writer, SG &sg) {
  crc8writer<writerT> w(writer);

  // write sensorgram content
  pack_uint(w, sg.body.size(), 2);
  pack_uint(w, sg.timestamp, 4);
  pack_uint(w, sg.id, 2);
  pack_uint(w, sg.inst, 1);
  pack_uint(w, sg.sub_id, 1);
  pack_uint(w, sg.source_id, 2);
  pack_uint(w, sg.source_inst, 1);
  pack_bytes(w, sg.body.bytes(), sg.body.size());

  // write crc sum
  w.close();
}

template <class readerT, class SG>
bool unpack_sensorgram(readerT &reader, SG &sg) {
  crc8reader<readerT> r(reader);

  // read sensorgram content
  int len = unpack_uint(r, 2);
  sg.timestamp = unpack_uint(r, 4);
  sg.id = unpack_uint(r, 2);
  sg.inst = unpack_uint(r, 1);
  sg.sub_id = unpack_uint(r, 1);
  sg.source_id = unpack_uint(r, 2);
  sg.source_inst = unpack_uint(r, 1);
  sg.body.clear();
  copyn(r, sg.body, len);

  // throw away trailing crc byte and check sum
  char scratch[1];
  r.read(scratch, 1);
  return r.sum == 0;
}

#endif
