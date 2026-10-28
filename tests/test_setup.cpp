// One-time setup that the engine normally does in the Engine constructor.
// Tests use Board and attacks:: directly, so the magic tables have to be filled here.

#include "attacks.hpp"

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

namespace {

class AttackTablesInit : public Catch::EventListenerBase {
public:
    using Catch::EventListenerBase::EventListenerBase;

    void testRunStarting(const Catch::TestRunInfo&) override {
        azalea::attacks::init();
    }
};

} // namespace

CATCH_REGISTER_LISTENER(AttackTablesInit)
