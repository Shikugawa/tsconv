#pragma once

#include <string>
#include <vector>

#include <mecab.h>

#include "utils.hpp"

#define IS_BEGIN_POS(i) 0 == i
#define IS_LAST_POS(i, features) i == features.size() - 1
#define IS_LAST_POS_BEFORE_ONE(i, features) i == features.size() - 2
#define IS_CONJUNCTIVE_FORM(feature) feature.conjugated_2 == "連用形"
#define IS_CONJUNCTIVE_TA_FORM(feature) feature.conjugated_2 == "連用タ接続"
#define IS_IMPERFECTIVE_FORM(feature) feature.conjugated_2 == "未然形"
#define IS_IMPERFECTIVE_FORM_U(feature) feature.conjugated_2 == "未然ウ接続"
#define IS_BASIC_FORM(feature) feature.conjugated_2 == "基本形"
#define IS_AUXILIARY_VERB(feature) feature.pos == "助動詞"
#define IS_ADJECTIVE(feature) feature.pos == "形容詞"
#define BASE_MAYBE(feature, str) feature.base == str

#define APPLY1(feature, f) f(feature)
#define APPLY2(feature, f, g) f(feature) && g(feature)
#define APPLY3(feature, f, g, h) f(feature) && g(feature) && h(feature)
#define APPLY(feature, num, ...) APPLY##num(feature, __VA_ARGS__)
#define APPLY_ALL_AND(feature, num, ...) APPLY(feature, num, __VA_ARGS__)

namespace TSConv {
struct MeCabfeatures {
  std::string normal;
  std::string pos;
  std::string detail_1;
  std::string detail_2;
  std::string detail_3;
  std::string conjugated_1;
  std::string conjugated_2;
  std::string base;
  std::string call;
  std::string voice;
};

// not thread-safe class
class Converter {
  static std::string Convert(std::string &&text) {
    return convertInternal(text);
  }

  static std::string Convert(std::string &text) {
    return convertInternal(text);
  }

private:
  static std::vector<MeCabfeatures> Getfeatures(std::string &&features) {
    std::vector<MeCabfeatures> results;

    MeCabfeatures features_struct;
    std::string tmp;
    size_t comma_count = 0;

    for (size_t i = 0; i < features.size(); ++i) {
      if (features[i] == ',') {
        ++comma_count;
        switch (comma_count) {
        case 1:
          features_struct.pos = tmp;
          tmp.clear();
          break;
        case 2:
          features_struct.detail_1 = tmp;
          tmp.clear();
          break;
        case 3:
          features_struct.detail_2 = tmp;
          tmp.clear();
          break;
        case 4:
          features_struct.detail_3 = tmp;
          tmp.clear();
          break;
        case 5:
          features_struct.conjugated_1 = tmp;
          tmp.clear();
          break;
        case 6:
          features_struct.conjugated_2 = tmp;
          tmp.clear();
          break;
        case 7:
          features_struct.base = tmp;
          tmp.clear();
          break;
        case 8:
          features_struct.call = tmp;
          tmp.clear();
          break;
        case 9:
          features_struct.voice = tmp;
          tmp.clear();
          break;
        default:
          break;
        }
      } else if (features[i] == '\t') {
        features_struct.normal = tmp;
        tmp.clear();
      } else if (features[i] == '\n') {
        comma_count = 0;
        results.emplace_back(features_struct);
        tmp.clear();
      } else {
        tmp += features[i];
      }
    }
    results.resize(results.size() - 1);
    return results;
  }

  static std::string convertInternal(std::string &text) {
    const auto tagger = MeCab::createTagger("");
    auto mecab_parse_result = tagger->parse(text.c_str());
    std::vector<std::string> text_piece;
    size_t actual_size = 0;
    bool termination_symbol_required = false;
    auto features = Getfeatures(mecab_parse_result);

    if (features[features.size() - 1].pos != "記号") {
      actual_size = features.size();
    } else {
      termination_symbol_required = true;
      actual_size = features.size() - 1;
    }

    for (size_t i = 0; i < actual_size; ++i) {
      auto last_pos_flag = termination_symbol_required
                               ? IS_LAST_POS_BEFORE_ONE(i, features)
                               : IS_LAST_POS(i, features);
      if (IS_BEGIN_POS(i) && features[i].pos == "接続詞" &&
          (features[i].base == "だが" || features[i].base == "が")) {
        text_piece.emplace_back("ですが");
        continue;
      }

      if (last_pos_flag && BASE_MAYBE(features[i - 2], "だ") &&
          APPLY_ALL_AND(features[i - 2], 2, IS_AUXILIARY_VERB,
                        IS_CONJUNCTIVE_FORM) && \ 
          BASE_MAYBE(features[i - 1], "ある") &&
          APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                        IS_CONJUNCTIVE_TA_FORM) &&
          BASE_MAYBE(features[i], "た") &&
          APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB, IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.pop_back();
        text_piece.emplace_back("でした");
      } else if (last_pos_flag &&
                 APPLY_ALL_AND(features[i - 2], 1, IS_IMPERFECTIVE_FORM) &&
                 BASE_MAYBE(features[i - 1], "ない") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                               IS_CONJUNCTIVE_TA_FORM) &&
                 BASE_MAYBE(features[i], "た") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.emplace_back("ませんでした");
      } else if (last_pos_flag && BASE_MAYBE(features[i - 2], "だ") &&
                 APPLY_ALL_AND(features[i - 2], 2, IS_AUXILIARY_VERB,
                               IS_CONJUNCTIVE_FORM) && \ 
          BASE_MAYBE(features[i - 1], "ある") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                               IS_CONJUNCTIVE_TA_FORM) &&
                 BASE_MAYBE(features[i], "た") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.emplace_back("でした");
      } else if (last_pos_flag && BASE_MAYBE(features[i - 2], "だ") &&
                 APPLY_ALL_AND(features[i - 2], 2, IS_AUXILIARY_VERB,
                               IS_CONJUNCTIVE_FORM) &&
                 BASE_MAYBE(features[i - 1], "ある") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                               IS_IMPERFECTIVE_FORM_U) &&
                 BASE_MAYBE(features[i], "う") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.pop_back();
        text_piece.emplace_back("でしょう");
      } else if (last_pos_flag &&
                 (APPLY_ALL_AND(features[i - 1], 1, IS_CONJUNCTIVE_FORM) ||
                  APPLY_ALL_AND(features[i - 1], 1, IS_CONJUNCTIVE_TA_FORM)) &&
                 BASE_MAYBE(features[i], "た") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        if (features[i - 1].normal.find("っ") != std::string::npos) {
          auto res = FindReplace(features[i - 1].normal, std::string("っ"),
                                 std::string("り"));
          text_piece.pop_back();
          text_piece.emplace_back(res);
        }
        text_piece.emplace_back("ました");
      } else if (last_pos_flag &&
                 APPLY_ALL_AND(features[i - 1], 1, IS_IMPERFECTIVE_FORM) &&
                 BASE_MAYBE(features[i], "ない") &&
                 APPLY_ALL_AND(features[i], 1, IS_BASIC_FORM)) {
        text_piece.emplace_back("ません");
      } else if (last_pos_flag && BASE_MAYBE(features[i - 1], "だ") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                               IS_CONJUNCTIVE_TA_FORM) && \ 
           BASE_MAYBE(features[i], "た") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.emplace_back("でした");
      } else if (last_pos_flag && BASE_MAYBE(features[i - 1], "だ") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                               IS_CONJUNCTIVE_FORM) &&
                 BASE_MAYBE(features[i], "ある") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.emplace_back("です");
      } else if (last_pos_flag && BASE_MAYBE(features[i - 1], "だ") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_AUXILIARY_VERB,
                               IS_IMPERFECTIVE_FORM) &&
                 BASE_MAYBE(features[i], "う") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.pop_back();
        text_piece.emplace_back("でしょう");
      } else if (last_pos_flag && BASE_MAYBE(features[i - 1], "ない") &&
                 APPLY_ALL_AND(features[i - 1], 2, IS_ADJECTIVE,
                               IS_CONJUNCTIVE_TA_FORM) &&
                 BASE_MAYBE(features[i], "た") &&
                 APPLY_ALL_AND(features[i], 2, IS_BASIC_FORM,
                               IS_AUXILIARY_VERB)) {
        text_piece.pop_back();
        text_piece.emplace_back("ありませんでした");
      } else if (last_pos_flag && BASE_MAYBE(features[i], "だ") &&
                 APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                               IS_BASIC_FORM)) {
        text_piece.emplace_back("です");
      }

      else if (last_pos_flag && BASE_MAYBE(features[i], "ない") &&
               APPLY_ALL_AND(features[i], 2, IS_AUXILIARY_VERB,
                             IS_BASIC_FORM)) {
        text_piece.emplace_back("ありません");
      }

      else if (last_pos_flag && BASE_MAYBE(features[i], "ない") &&
               APPLY_ALL_AND(features[i], 2, IS_BASIC_FORM, IS_ADJECTIVE)) {
        text_piece.emplace_back("ありません");
      } else if (last_pos_flag && IS_BASIC_FORM(features[i])) {
        if (features[i].normal.find("る") != std::string::npos) {
          auto res = FindReplace(features[i].normal, std::string("る"),
                                 std::string("ます"));
          text_piece.emplace_back(res);
        } else {
          text_piece.emplace_back("ます");
        }
      } else {
        text_piece.emplace_back(features[i].normal);
      }
    }

    if (termination_symbol_required) {
      text_piece.emplace_back("。");
    }

    return Join(text_piece);
  }
};
} // namespace TSConv
