#include "search/edge_generator.hh"

#include "lm/left.hh"
#include "lm/partial.hh"
#include "search/context.hh"
#include "search/vertex.hh"
#include "search/vertex_generator.hh"

#include <numeric>

namespace search {

bool EdgeGenerator::Init(Edge &edge) {
  from_ = &edge;
  PartialEdge root;
  root.score = GetRule().Bound();
  for (unsigned int i = 0; i < GetRule().Arity(); ++i) {
    root.nt[i] = edge.GetVertex(i).RootPartial();
    if (root.nt[i].Empty()) return false;
    root.score += root.nt[i].Bound();
  }
  for (unsigned int i = GetRule().Arity(); i < 2; ++i) {
    root.nt[i] = kBlankPartialVertex;
  }
  for (unsigned int i = 0; i < GetRule().Arity() + 1; ++i) {
    root.between[i] = GetRule().Lexical(i);
  }
  // wtf no clear method?
  generate_ = Generate();
  generate_.push(root);
  top_ = root.score;
  return true;
}

namespace {

template <class Model> float FastScore(const Context<Model> &context, unsigned char victim, unsigned char arity, const PartialEdge &previous, PartialEdge &update) {
  memcpy(update.between, previous.between, sizeof(lm::ngram::ChartState) * (arity + 1));

  float ret = 0.0;
  lm::ngram::ChartState *before, *after;
  if (victim == 0) {
    before = &update.between[0];
    after = &update.between[(arity == 2 && previous.nt[1].Complete()) ? 2 : 1];
  } else {
    assert(victim == 1);
    assert(arity == 2);
    before = &update.between[previous.nt[0].Complete() ? 0 : 1];
    after = &update.between[2];
  }
  const lm::ngram::ChartState &previous_reveal = previous.nt[victim].State();
  const PartialVertex &update_nt = update.nt[victim];
  const lm::ngram::ChartState &update_reveal = update_nt.State();
  float just_after = 0.0;
  if ((update_reveal.left.length > previous_reveal.left.length) || (update_reveal.left.full && !previous_reveal.left.full)) {
    just_after += lm::ngram::RevealAfter(context.LanguageModel(), before->left, before->right, update_reveal.left, previous_reveal.left.length);
  }
  if ((update_reveal.right.length > previous_reveal.right.length) || (update_nt.RightFull() && !previous.nt[victim].RightFull())) {
    ret += lm::ngram::RevealBefore(context.LanguageModel(), update_reveal.right, previous_reveal.right.length, update_nt.RightFull(), after->left, after->right);
  }
  if (update_nt.Complete()) {
    if (update_reveal.left.full) {
      before->left.full = true;
    } else {
      assert(update_reveal.left.length == update_reveal.right.length);
      ret += lm::ngram::Subsume(context.LanguageModel(), before->left, before->right, after->left, after->right, update_reveal.left.length);
    }
    if (victim == 0) {
      update.between[0].right = after->right;
    } else {
      update.between[2].left = before->left;
    }
  }
  return previous.score + (ret + just_after) * context.GetWeights().LM();
}

} // namespace

template <class Model> bool EdgeGenerator::Pop(Context<Model> &context, VertexGenerator &parent) {
  assert(!generate_.empty());
  const PartialEdge &top = generate_.top();
  unsigned int victim = 0;
  unsigned char lowest_length = 255;
  for (unsigned int i = 0; i != GetRule().Arity(); ++i) {
    if (!top.nt[i].Complete() && top.nt[i].Length() < lowest_length) {
      lowest_length = top.nt[i].Length();
      victim = i;
    }
  }
  if (lowest_length == 255) {
    // All states report complete.  
    lm::ngram::ChartState state;
    state.left = top.between[0].left;
    state.right = top.between[GetRule().Arity()].right;
    parent.NewHypothesis(state, *from_, top);
    generate_.pop();
    top_ = generate_.empty() ? -kScoreInf : generate_.top().score;
    return !generate_.empty();
  }

  unsigned int stay = !victim;
  PartialEdge continuation, alternate;
  // The alternate's score will change because alternate.nt[victim] changes.  
  bool split = top.nt[victim].Split(continuation.nt[victim], alternate.nt[victim]);

  continuation.nt[stay] = top.nt[stay];
  continuation.score = FastScore(context, victim, GetRule().Arity(), top, continuation);

  if (split) {
    // We have an alternate.  
    alternate.score = top.score - top.nt[victim].Bound() + alternate.nt[victim].Bound();
    memcpy(alternate.between, top.between, sizeof(lm::ngram::ChartState) * (GetRule().Arity() + 1));
    alternate.nt[stay] = top.nt[stay];

    generate_.pop();
    // top is now a dangling reference.  
 
    // TODO: dedupe?  
    generate_.push(alternate);
  } else {
    generate_.pop();
    // top is now a dangling reference.  
  }

  // TODO: dedupe?  
  generate_.push(continuation);

  top_ = generate_.top().score;
  return true;
}

template bool EdgeGenerator::Pop(Context<lm::ngram::RestProbingModel> &context, VertexGenerator &parent);
template bool EdgeGenerator::Pop(Context<lm::ngram::ProbingModel> &context, VertexGenerator &parent);

} // namespace search
