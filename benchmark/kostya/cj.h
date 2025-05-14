#pragma once

#ifdef SIMDJSON_COMPETITION_CJ

#include "kostya.h"

namespace kostya {

struct cj_base {
  static constexpr diff_flags DiffFlags = diff_flags::NONE;

  simdjson_inline double get_double(JSON_ELEMENT *object, const char *key) {
    JSON_ELEMENT *field = get_by_pointer(object, key);

    if (field == NULL) {
      throw "Missing double field";
    }
    if (field->type == JET_DOUBLE) {
      return field->data.double_value;
    } else if (field->type == JET_INT) {
      return field->data.int_value;
    } else {
      throw "Field is not double";
    }
  }

  bool run(JSON_ELEMENT *doc, std::vector<point> &result) {
    if (!doc) {
      return false;
    }
    JSON_ELEMENT *root = doc;

    if (root->type != JET_OBJ) {
      return false;
    }

    JSON_ELEMENT *coords = get_by_pointer(root, "/coordinates");
    if (!coords || coords->type != JET_ARRAY) {
      return false;
    }

    JSON_ELEMENT *sub = coords->data.sub_value->sub_begin;
    while (sub) {
      if (sub->type != JET_OBJ) {
        return false;
      }
      result.emplace_back(json_benchmark::point{
          get_double(sub, "/x"), get_double(sub, "/y"), get_double(sub, "/z")});
      sub = sub->next;
    }

    return true;
  }
};

struct cj : cj_base {
  bool run(simdjson::padded_string &json, std::vector<point> &result) {
    return cj_base::run(cj_parse((const uint8_t *)json.data(), json.size()),
                        result);
  }
};
BENCHMARK_TEMPLATE(kostya, cj)->UseManualTime();

} // namespace kostya

#endif // SIMDJSON_COMPETITION_CJ
