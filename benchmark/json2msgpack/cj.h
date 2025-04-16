#pragma once
#ifdef SIMDJSON_COMPETITION_CJ

#include "json2msgpack.h"

namespace json2msgpack {

struct cj2msgpack {
  inline std::string_view to_msgpack(JSON_ELEMENT *doc, uint8_t *buf);

private:
  inline void write_double(const double d) noexcept;
  inline void write_byte(const uint8_t b) noexcept;
  inline void write_uint32(const uint32_t w) noexcept;
  inline void write_string(const char *s, size_t length) noexcept;
  inline void recursive_processor(JSON_ELEMENT *obj);

  uint8_t *buff{};
};

std::string_view cj2msgpack::to_msgpack(JSON_ELEMENT *doc, uint8_t *buf) {
  buff = buf;
  JSON_ELEMENT *root = doc;
  recursive_processor(root);
  return std::string_view(reinterpret_cast<char *>(buf), size_t(buff - buf));
}

void cj2msgpack::write_string(const char *c, size_t len) noexcept {
  write_byte(0xdb);
  write_uint32(uint32_t(len));
  ::memcpy(buff, c, len);
  buff += len;
}

void cj2msgpack::write_double(const double d) noexcept {
  *buff++ = 0xcb;
  ::memcpy(buff, &d, sizeof(d));
  buff += sizeof(d);
}

void cj2msgpack::write_byte(const uint8_t b) noexcept {
  *buff = b;
  buff++;
}

void cj2msgpack::write_uint32(const uint32_t w) noexcept {
  ::memcpy(buff, &w, sizeof(w));
  buff += sizeof(w);
}

void cj2msgpack::recursive_processor(JSON_ELEMENT *obj) {
  JSON_ELEMENT *curr = obj;
  while (curr) {
    switch (curr->type) {
    case JET_STRING: {
      const J_STRING *str_p = (const J_STRING *)curr->data;
      const char *str = (char *)str_p->value;
      write_string(str, str_p->size);
    } break;
    case JET_ARRAY: {
      write_byte(0xdf);
      write_uint32(uint32_t(curr->sub_size));
      JSON_ELEMENT *sub_ele = curr->sub_begin;
      while (sub_ele) {
        if (sub_ele)
          recursive_processor(sub_ele);
        sub_ele = sub_ele->next;
      }
    } break;
    case JET_OBJ: {
      write_byte(0xdd);
      write_uint32(uint32_t(curr->sub_size));

      JSON_ELEMENT *sub_ele = curr->sub_begin;
      while (sub_ele) {
        const char *sub_key = (const char *)sub_ele->key;
        if (sub_key)
          write_string(sub_key, sub_ele->key_size);

        recursive_processor(sub_ele);

        sub_ele = sub_ele->next;
      }
    } break;
    case JET_BOOL: {
      const J_BOOL *bool_p = (const J_BOOL *)curr->data;
      write_byte(0xc2 + bool_p->value);
    } break;
    case JET_NULL:
      write_byte(0xc0);
      break;
    case JET_INT: {
      const J_INT *int_p = (const J_INT *)curr->data;
      double iv = double(int_p->value);
      write_double(iv);
    } break;
    case JET_DOUBLE: {
      const J_DOUBLE *int_p = (const J_DOUBLE *)curr->data;
      write_double(int_p->value);
    } break;
    default:
      SIMDJSON_UNREACHABLE();
    }

    curr = curr->next;
  }
}

struct cj : cj2msgpack {
  bool run(simdjson::padded_string &json, char *buffer,
           std::string_view &result) {
    JSON_ELEMENT *doc = cj_parse((const uint8_t *)json.data(), json.size());
    result = to_msgpack(doc, reinterpret_cast<uint8_t *>(buffer));
    return true;
  }
};

BENCHMARK_TEMPLATE(json2msgpack, cj)->UseManualTime();

} // namespace json2msgpack

#endif // SIMDJSON_COMPETITION_YYJSON