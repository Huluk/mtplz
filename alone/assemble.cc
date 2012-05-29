#include "alone/assemble.hh"

#include "search/final.hh"
#include "search/rule.hh"

#include <iostream>

namespace alone {

std::ostream &operator<<(std::ostream &o, const search::Final &final) {
  const search::Rule::ItemsRet &words = final.From().Items();
  if (words.empty()) return o;
  const search::Final *const *child = final.Children().data();
  search::Rule::ItemsRet::const_iterator i(words.begin());
  for (; i != words.end() - 1; ++i) {
    if (i->Terminal()) {
      o << i->String() << ' ';
    } else {
      o << **child << ' ';
      ++child;
    }
  }

  if (i->Terminal()) {
    if (i->String() != "</s>") {
      o << i->String();
    }
  } else {
    o << **child;
  }

  return o;
}

namespace {

void MakeIndent(std::ostream &o, const char *indent_str, unsigned int level) {
  for (unsigned int i = 0; i < level; ++i)
    o << indent_str;
}

void DetailedFinalInternal(std::ostream &o, const search::Final &final, const char *indent_str, unsigned int indent) {
  o << "(\n";
  MakeIndent(o, indent_str, indent);
  const search::Rule::ItemsRet &words = final.From().Items();
  const search::Final *const *child = final.Children().data();
  for (search::Rule::ItemsRet::const_iterator i(words.begin()); i != words.end(); ++i) {
    if (i->Terminal()) {
      o << i->String();
      if (i == words.end() - 1) {
        o << '\n';
        MakeIndent(o, indent_str, indent);
      } else {
        o << ' ';
      }
    } else {
      // One extra indent from the line we're currently on.  
      o << indent_str;
      DetailedFinalInternal(o, **child, indent_str, indent + 1);
      for (unsigned int i = 0; i < indent; ++i) o << indent_str;
      ++child;
    }
  }
  o << ")=" << final.Bound() << '\n';
}
} // namespace

void DetailedFinal(std::ostream &o, const search::Final &final, const char *indent_str) {
  DetailedFinalInternal(o, final, indent_str, 0);
}

void PrintFinal(const search::Final &final) {
  std::cout << final << std::endl;
}

} // namespace alone
