#include "dreal/contractor/contractor_fixpoint.h"

#include <utility>

#include "dreal/util/assert.h"
#include "dreal/util/exception.h"
#include "dreal/util/interrupt.h"
#include "dreal/util/logging.h"

using std::ostream;
using std::vector;

namespace dreal {

ContractorFixpoint::ContractorFixpoint(TerminationCondition term_cond,
                                       vector<Contractor> contractors,
                                       const Config& config)
    : ContractorCell{Contractor::Kind::FIXPOINT,
                     ibex::BitSet::empty(ComputeInputSize(contractors)),
                     config},
      term_cond_{std::move(term_cond)},
      contractors_{std::move(contractors)},
      old_iv_{1 /* will be updated anyway */} {
  DREAL_ASSERT(!contractors_.empty());
  ibex::BitSet& input{mutable_input()};
  for (const Contractor& c : contractors_) {
    input |= c.input();
  }
}

void ContractorFixpoint::Prune(ContractorStatus* cs) const {
  do {
    // Note that 'DREAL_CHECK_INTERRUPT' is only defined in setup.py,
    // when we build dReal python package.
#ifdef DREAL_CHECK_INTERRUPT
    if (g_interrupted) {
      DREAL_LOG_DEBUG("KeyboardInterrupt(SIGINT) Detected.");
      throw std::runtime_error("KeyboardInterrupt(SIGINT) Detected.");
    }
#endif
    old_iv_ = cs->box().interval_vector();
    for (const Contractor& ctc : contractors_) {
      ctc.Prune(cs);
      if (cs->box().empty()) {
        return;
      }
    }
  } while (!term_cond_(old_iv_, cs->box().interval_vector()));
}

ostream& ContractorFixpoint::display(ostream& os) const {
  os << "Fixpoint(";
  for (const Contractor& c : contractors_) {
    os << c << ", ";
  }
  return os << ")";
}

}  // namespace dreal
