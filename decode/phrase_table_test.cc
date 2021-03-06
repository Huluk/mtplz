#include "decode/phrase_table.hh"
#include "decode/scorer.hh"
#include "util/mutable_vocab.hh"
#include <iostream>
#include <fstream>

#define BOOST_TEST_MODULE VocabTest
#include <boost/test/unit_test.hpp>

namespace decode {
namespace {

  // Unfortunate code to determine the file locations for test inputs.
  // This is necessary because bjam is nondeterministic in how it
  // orders the command line arguments for this test. 
struct Locations {
  Locations() {
    int argc = boost::unit_test::framework::master_test_suite().argc;
    char **argv = boost::unit_test::framework::master_test_suite().argv;
    phrase_table = source_text = lm = NULL;
    for (int i = 1; i < argc; ++i) {
      if (strstr(argv[i], "phrase_table")) {
        phrase_table = argv[i];
      } else if (strstr(argv[i], "source_text")) {
        source_text = argv[i];
      } else if (strstr(argv[i], "arpa")) {
        lm = argv[i];
      }
    }
    BOOST_REQUIRE(phrase_table);
    BOOST_REQUIRE(source_text);
    BOOST_REQUIRE(lm);
  }

  const char *phrase_table;
  const char *source_text;
  const char *lm;
};
 
// TODO: We need a more thorough test
BOOST_AUTO_TEST_CASE(phrase_table) {
  // Locations locations;
  // util::MutableVocab vocab;
  // Scorer scorer(locations.lm, "1 2 3 4 5 6", vocab);

  // PhraseTable phrase_table(locations.phrase_table, vocab, scorer);

  // std::ifstream fs(locations.source_text);
  // BOOST_REQUIRE(fs);
  // util::Pool pool;
  // bool should_find_phrase;
  // std::string line;
  // while((fs >> should_find_phrase) && getline(fs, line)) {
  //   Phrase phrase(pool, vocab, line);
  //   std::cerr << line << std::endl;
  //   bool does_find_phrase = (phrase_table.Phrases(phrase.begin(), phrase.end()) != NULL);
  //   std::cerr << "does_find_phrase: " << does_find_phrase << "  should_find_phrase: " << should_find_phrase << std::endl;
  //   BOOST_CHECK_EQUAL(does_find_phrase, should_find_phrase);
  // }
}

} // namespace
} // namespace decode
